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

    std::sort(m_gpios.begin(), m_gpios.end(), [](SpGpio_t a, SpGpio_t b)
    {
        return a->getTimingInfo().getStart() < b->getTimingInfo().getStart();
    });

    checkGpioSeriesMembers(m_gpios);

    m_timingInfo = makeGlobalTimingInfo(getTimingInfosForSeries());

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
GpioSeries::isAnalog() const
{
    for (const auto & g : m_gpios)
    {
        if (!g->isAnalog())
        {
            return false;
        }
    }
    return m_gpios.size() > 0;
}

std::string
GpioSeries::getFileName() const
{
    return "**GpioSeries";
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

TimingInfo
GpioSeries::getGaplessTimingInfo()
{
    return makeGaplessTimingInfo(getTimingInfosForSeries());
}

SpFTrace_t
GpioSeries::getAnalogData()
{
    SpFTrace_t trace = std::make_shared<FTrace_t>(getGaplessTimingInfo());
    float * v = trace->getValues();
    for (const auto & g : m_gpios)
    {
        SpFTrace_t partialTrace = g->getAnalogData();
        float * vPartial = partialTrace->getValues();
        const isize_t numSamples = partialTrace->getTimingInfo().getNumTimes();
        memcpy((char *)v, (char *)vPartial, sizeof(float) * numSamples);
        v += numSamples;
    }
    return trace;
}

void
GpioSeries::getAnalogDataAsync(GpioGetAnalogDataCB_t inCallback)
{
    std::weak_ptr<Gpio> weakThis = shared_from_this();

    AsyncTaskResult<SpFTrace_t> asyncTaskResult;
    asyncTaskResult.setValue(std::make_shared<FTrace_t>(getGaplessTimingInfo()));

    isize_t counter = 0;
    bool isLast = false;
    isize_t offset = 0;

    for (const auto & g : m_gpios)
    {
        isLast = (counter == (m_gpios.size() - 1));

        GpioGetAnalogDataCB_t finishedCB =
            [weakThis, &asyncTaskResult, offset, isLast, inCallback] (AsyncTaskResult<SpFTrace_t> inAsyncTaskResult)
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

        g->getAnalogDataAsync(finishedCB);
        offset += g->getTimingInfo().getNumTimes();
        ++counter;
    }
}

SpLogicalTrace_t
GpioSeries::getLogicalData(const std::string & inChannelName)
{
    SpLogicalTrace_t trace = std::make_shared<LogicalTrace>(getGaplessTimingInfo(), inChannelName);
    for (const auto & g : m_gpios)
    {
        for (const auto & kv : g->getLogicalData(inChannelName)->getValues())
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
    asyncTaskResult.setValue(std::make_shared<LogicalTrace>(getGaplessTimingInfo(), inChannelName));

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

const isx::TimingInfo &
GpioSeries::getTimingInfo() const
{
    return m_timingInfo;
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
