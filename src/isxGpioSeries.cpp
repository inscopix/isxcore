#include "isxGpioSeries.h"
#include "isxAsync.h"
#include "isxAsyncTaskHandle.h"
#include "isxDispatchQueue.h"
#include "isxSeries.h"
#include "isxException.h"
#include "isxSeriesUtils.h"

#include <algorithm>
#include <string.h>

namespace isx
{

GpioSeries::GpioSeries()
{
}

GpioSeries::GpioSeries(const std::vector<std::string> & inFileNames)
{
    ISX_ASSERT(inFileNames.size() > 0);
    if (inFileNames.size() == 0)
    {
        return;
    }

    for (const auto & fn : inFileNames)
    {
        m_gpios.emplace_back(readGpio(fn));
    }

    std::string errorMessage;
    for (isize_t i = 1; i < m_gpios.size(); ++i)
    {
        if (!checkNewMemberOfSeries({m_gpios[i - 1]}, m_gpios[i], errorMessage))
        {
            ISX_THROW(ExceptionSeries, errorMessage);
        }
    }

    // Use the first channel as reference for sorting by time
    auto channelList = m_gpios.front()->getChannelList();
    std::string firstChannel = channelList.front();
    std::sort(m_gpios.begin(), m_gpios.end(), [&firstChannel](SpGpio_t a, SpGpio_t b)
    {
        return a->getTimingInfo(firstChannel).getStart() < b->getTimingInfo(firstChannel).getStart();
    });


    for (auto & n : channelList)
    {
        m_gaplessTimingInfo[n] = makeGaplessTimingInfo(getTimingInfosForSeries(n));        
    }    
    
    m_generalGaplessTimingInfo = makeGaplessTimingInfo(getTimingInfosForSeries());
    m_valid = true;
}

GpioSeries::~GpioSeries()
{
}

bool
GpioSeries::isValid() const
{
    return m_valid;
}

bool
GpioSeries::isAnalog(const std::string & inChannelName) const
{
    for (const auto & g : m_gpios)
    {
        if (!g->isAnalog(inChannelName))
        {
            return false;
        }
    }
    return m_gpios.size() > 0;
}

std::string
GpioSeries::getFileName() const
{
    std::vector<std::string> filePaths;
    for (const auto & g : m_gpios)
    {
        filePaths.push_back(g->getFileName());
    }
    return makeSeriesFilePathString("GpioSeries", filePaths);
}

isize_t
GpioSeries::numberOfChannels()
{
    return m_gpios[0]->numberOfChannels();
}

const std::vector<std::string>
GpioSeries::getChannelList() const
{
    return m_gpios[0]->getChannelList();
}

void 
GpioSeries::getAllTraces(std::vector<SpFTrace_t> & outContinuousTraces, std::vector<SpLogicalTrace_t> & outLogicalTraces)
{
    auto channels = getChannelList();

    outContinuousTraces.resize(channels.size());
    outLogicalTraces.resize(channels.size());
    std::vector<float *> globalContVals(channels.size(), nullptr);

    for (size_t i(0); i < channels.size(); ++i)
    {
        outContinuousTraces[i] = nullptr;
        outLogicalTraces[i] = nullptr;
        auto & n = channels[i];
        
        outLogicalTraces[i] = std::make_shared<LogicalTrace>(m_gaplessTimingInfo[n], channels[i]);
        
        if (isAnalog(n))
        {
            outContinuousTraces[i] = std::make_shared<FTrace_t>(m_gaplessTimingInfo[n], channels[i]);
            globalContVals[i] = outContinuousTraces[i]->getValues();
        }
    }     

    for (const auto & g : m_gpios)
    {
        std::vector<SpFTrace_t> partialContinuousTraces;
        std::vector<SpLogicalTrace_t> partialLogicalTraces;
        g->getAllTraces(partialContinuousTraces, partialLogicalTraces);

        // Copy continuous data
        isize_t partialIdx = 0;
        for (auto & v : globalContVals)
        {
            auto & p = partialContinuousTraces[partialIdx];
            if (v != nullptr && p != nullptr)
            {
                float * partialContVals = p->getValues();
                const isize_t numSamples = p->getTimingInfo().getNumTimes();
                memcpy((char *)v, (char *)partialContVals, sizeof(float) * numSamples);
                v += numSamples;
            }
            ++partialIdx;
        }

        // Copy logical data
        for (isize_t i(0); i < partialLogicalTraces.size(); ++i)
        {
            if (outLogicalTraces[i] && partialLogicalTraces[i])
            {
                auto partialVals = partialLogicalTraces[i]->getValues();
                for ( auto p : partialVals)
                {
                    outLogicalTraces[i]->addValue(p.first, p.second);
                }
            }
        }
    }
}

SpFTrace_t
GpioSeries::getAnalogData(const std::string & inChannelName)
{
    SpFTrace_t trace = std::make_shared<FTrace_t>(m_gaplessTimingInfo[inChannelName]);
    float * v = trace->getValues();
    for (const auto & g : m_gpios)
    {
        SpFTrace_t partialTrace = g->getAnalogData(inChannelName);
        float * vPartial = partialTrace->getValues();
        const isize_t numSamples = partialTrace->getTimingInfo().getNumTimes();
        memcpy((char *)v, (char *)vPartial, sizeof(float) * numSamples);
        v += numSamples;
    }
    return trace;
}

void
GpioSeries::getAnalogDataAsync(const std::string & inChannelName, GpioGetAnalogDataCB_t inCallback)
{
    std::weak_ptr<Gpio> weakThis = shared_from_this();

    AsyncTaskResult<SpFTrace_t> asyncTaskResult;
    asyncTaskResult.setValue(std::make_shared<FTrace_t>(m_gaplessTimingInfo[inChannelName]));

    isize_t counter = 0;
    bool isLast = false;
    isize_t offset = 0;

    for (const auto & g : m_gpios)
    {
        isLast = (counter == (m_gpios.size() - 1));

        GpioGetAnalogDataCB_t finishedCB =
            [weakThis, &asyncTaskResult, offset, isLast, inCallback, inChannelName] (AsyncTaskResult<SpFTrace_t> inAsyncTaskResult)
            {
                auto sharedThis = weakThis.lock();
                if (!sharedThis)
                {
                    return;
                }

                if (inAsyncTaskResult.getException())
                {
                    asyncTaskResult.setException(inAsyncTaskResult.getException());
                }
                else
                {
                    // only continue copying if previous segments didn't throw
                    if (!asyncTaskResult.getException())
                    {
                        auto traceSegment = inAsyncTaskResult.get();
                        auto traceSeries = asyncTaskResult.get();
                        const isize_t numSamples = traceSegment->getTimingInfo().getNumTimes();
                        float * vals = traceSeries->getValues();
                        vals += offset;
                        memcpy((char *)vals, (char *)traceSegment->getValues(), sizeof(float) * numSamples);
                    }
                }

                if (isLast)
                {
                    inCallback(asyncTaskResult);
                }
            };

        g->getAnalogDataAsync(inChannelName, finishedCB);
        offset += g->getTimingInfo(inChannelName).getNumTimes();
        ++counter;
    }
}

SpLogicalTrace_t
GpioSeries::getLogicalData(const std::string & inChannelName)
{
    SpLogicalTrace_t trace = std::make_shared<LogicalTrace>(m_gaplessTimingInfo[inChannelName], inChannelName);
    for (const auto & g : m_gpios)
    {
        const SpLogicalTrace_t gTrace = g->getLogicalData(inChannelName);
        for (const auto & kv : gTrace->getValues())
        {
            trace->addValue(kv.first, kv.second);
        }
    }
    return trace;
}

void
GpioSeries::getLogicalDataAsync(const std::string & inChannelName, GpioGetLogicalDataCB_t inCallback)
{
    std::weak_ptr<Gpio> weakThis = shared_from_this();

    AsyncTaskResult<SpLogicalTrace_t> asyncTaskResult;
    asyncTaskResult.setValue(std::make_shared<LogicalTrace>(m_gaplessTimingInfo[inChannelName], inChannelName));

    isize_t counter = 0;
    bool isLast = false;

    for (const auto & g : m_gpios)
    {
        isLast = (counter == (m_gpios.size() - 1));

        GpioGetLogicalDataCB_t finishedCB =
            [weakThis, &asyncTaskResult, isLast, inCallback] (AsyncTaskResult<SpLogicalTrace_t> inAsyncTaskResult)
            {
                auto sharedThis = weakThis.lock();
                if (!sharedThis)
                {
                    return;
                }

                if (inAsyncTaskResult.getException())
                {
                    asyncTaskResult.setException(inAsyncTaskResult.getException());
                }
                else
                {
                    // only continue copying if previous segments didn't throw
                    if (!asyncTaskResult.getException())
                    {
                        const auto traceSegment = inAsyncTaskResult.get();
                        auto traceSeries = asyncTaskResult.get();
                        for (const auto & kv : traceSegment->getValues())
                        {
                            traceSeries->addValue(kv.first, kv.second);
                        }
                    }
                }

                if (isLast)
                {
                    inCallback(asyncTaskResult);
                }
            };

        g->getLogicalDataAsync(inChannelName, finishedCB);
        ++counter;
    }
}

isx::TimingInfo 
GpioSeries::getTimingInfo(const std::string & inChannelName) const
{
    return m_gaplessTimingInfo.at(inChannelName);
}

isx::TimingInfos_t
GpioSeries::getTimingInfosForSeries(const std::string & inChannelName) const
{
    TimingInfos_t tis;
    for (const auto & g : m_gpios)
    {
        tis.emplace_back(g->getTimingInfo(inChannelName));
    }
    return tis;
}

isx::TimingInfo 
GpioSeries::getTimingInfo() const 
{
    return m_generalGaplessTimingInfo;
}
    
isx::TimingInfos_t
GpioSeries::getTimingInfosForSeries() const 
{
    TimingInfos_t tis;
    for (const auto & g : m_gpios)
    {
        tis.emplace_back(g->getTimingInfo());
    }
    return tis;
}

void
GpioSeries::cancelPendingReads()
{
    for (const auto & g : m_gpios)
    {
        g->cancelPendingReads();
    }
}

} // namespace isx
