#include "isxVesselSetFactory.h"
#include "isxVesselSetSeries.h"
#include "isxAsync.h"
#include "isxAsyncTaskHandle.h"
#include "isxDispatchQueue.h"
#include "isxSeries.h"
#include "isxSeriesUtils.h"

#include <algorithm>
#include <cstring>
#include <cmath>

namespace isx
{
    VesselSetSeries::VesselSetSeries()
    {

    }

    VesselSetSeries::VesselSetSeries(const std::vector<std::string> & inFileNames, bool enableWrite)
        : m_valid(false)
    {
        ISX_ASSERT(inFileNames.size() > 0);
        if (inFileNames.size() == 0)
        {
            return;
        }

        for(const auto &fn : inFileNames)
        {
            m_vesselSets.emplace_back(readVesselSet(fn, enableWrite));
        }

        std::sort(m_vesselSets.begin(), m_vesselSets.end(), [](SpVesselSet_t a, SpVesselSet_t b)
        {
            return a->getTimingInfo().getStart() < b->getTimingInfo().getStart();
        });

        // vessel sets are sorted by start time now, check if they meet requirements
        std::string errorMessage;
        for (isize_t i = 1; i < m_vesselSets.size(); ++i)
        {
            if (!checkNewMemberOfSeries({m_vesselSets[i - 1]}, m_vesselSets[i], errorMessage))
            {
                ISX_THROW(ExceptionSeries, errorMessage);
            }
        }

        m_gaplessTimingInfo = makeGaplessTimingInfo(getTimingInfosForSeries());
        m_valid = true;
    }

    VesselSetSeries::~VesselSetSeries()
    {

    }

    bool
    VesselSetSeries::isValid() const
    {
        return m_valid;
    }

    void
    VesselSetSeries::closeForWriting()
    {
        for (auto & v: m_vesselSets)
        {
            v->closeForWriting();
        }
    }

    std::string
    VesselSetSeries::getFileName() const
    {
        std::vector<std::string> filePaths;
        for (const auto & vs : m_vesselSets)
        {
            filePaths.push_back(vs->getFileName());
        }
        return makeSeriesFilePathString("VesselSetSeries", filePaths);
    }

    const isize_t
    VesselSetSeries::getNumVessels()
    {
        return m_vesselSets[0]->getNumVessels();
    }

    isx::TimingInfo
    VesselSetSeries::getTimingInfo() const
    {
        return m_gaplessTimingInfo;
    }

    isx::TimingInfos_t
    VesselSetSeries::getTimingInfosForSeries() const
    {
        TimingInfos_t tis;
        for(const auto &vs : m_vesselSets)
        {
            tis.emplace_back(vs->getTimingInfo());
        }
        return tis;
    }

    isx::SpacingInfo
    VesselSetSeries::getSpacingInfo() const
    {
        return m_vesselSets[0]->getSpacingInfo();
    }

    SpFTrace_t
    VesselSetSeries::getTrace(isize_t inIndex)
    {
        SpFTrace_t trace = std::make_shared<FTrace_t>(m_gaplessTimingInfo);
        float * v = trace->getValues();

        for (const auto &vs : m_vesselSets)
        {
            SpFTrace_t partialTrace = vs->getTrace(inIndex);
            float * vPartial = partialTrace->getValues();
            isize_t numSamples = partialTrace->getTimingInfo().getNumTimes();
            memcpy((char *)v, (char *)vPartial, sizeof(float)*numSamples);
            v += numSamples;
        }
        return trace;
    }

    void
    VesselSetSeries::getTraceAsync(isize_t inIndex, VesselSetGetTraceCB_t inCallback)
    {
        std::weak_ptr<VesselSet> weakThis = shared_from_this();

        AsyncTaskResult<SpFTrace_t> asyncTaskResult;
        asyncTaskResult.setValue(std::make_shared<FTrace_t>(m_gaplessTimingInfo));

        isize_t counter = 0;
        bool isLast = false;
        isize_t offset = 0;

        for (const auto &vs : m_vesselSets)
        {
            isLast = (counter == (m_vesselSets.size() - 1));

            VesselSetGetTraceCB_t finishedCB =
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
                        isize_t numTimes = traceSegment->getTimingInfo().getNumTimes();
                        isize_t numBytes = numTimes * sizeof(float);
                        float * vals = traceSeries->getValues();
                        vals += offset;
                        memcpy((char *)vals, (char *)traceSegment->getValues(), numBytes);
                    }
                }

                if (isLast)
                {
                    inCallback(asyncTaskResult);
                }
            };

            vs->getTraceAsync(inIndex, finishedCB);
            isize_t numSamples = vs->getTimingInfo().getNumTimes();
            offset += numSamples;

            ++counter;
        }

    }

    SpImage_t
    VesselSetSeries::getImage(isize_t inIndex)
    {
        return m_vesselSets[0]->getImage(inIndex);
    }

    void
    VesselSetSeries::getImageAsync(isize_t inIndex, VesselSetGetImageCB_t inCallback)
    {
        return m_vesselSets[0]->getImageAsync(inIndex, inCallback);
    }

    SpVesselLine_t
    VesselSetSeries::getLineEndpoints(isize_t inIndex)
    {
        return m_vesselSets[0]->getLineEndpoints(inIndex);
    }

    void
    VesselSetSeries::getLineEndpointsAsync(isize_t inIndex, VesselSetGetLineEndpointsCB_t inCallback)
    {
        return m_vesselSets[0]->getLineEndpointsAsync(inIndex, inCallback);
    }

    SpFTrace_t
    VesselSetSeries::getDirectionTrace(isize_t inIndex)
    {
        if (m_vesselSets[0]->getDirectionTrace(0) == nullptr)
        {
            return nullptr;
        }
        
        SpFTrace_t direction = std::make_shared<Trace<float>>(m_gaplessTimingInfo);
        float * v = direction->getValues();

        for (const auto &vs : m_vesselSets)
        {
            SpFTrace_t partialDirection = vs->getDirectionTrace(inIndex);
            float * vPartial = partialDirection->getValues();
            isize_t numSamples = partialDirection->getTimingInfo().getNumTimes();
            memcpy((char *)v, (char *)vPartial, sizeof(float)*numSamples);
            v += numSamples;
        }
        return direction;
    }

    void
    VesselSetSeries::getDirectionTraceAsync(isize_t inIndex, VesselSetGetTraceCB_t inCallback)
    {
        std::weak_ptr<VesselSet> weakThis = shared_from_this();

        AsyncTaskResult<SpFTrace_t> asyncTaskResult;
        if (m_vesselSets[0]->getDirectionTrace(0) == nullptr)
        {
            asyncTaskResult.setValue(nullptr);
            inCallback(asyncTaskResult);
            return;
        }
        
        asyncTaskResult.setValue(std::make_shared<Trace<float>>(m_gaplessTimingInfo));

        isize_t counter = 0;
        bool isLast = false;
        isize_t offset = 0;

        for (const auto &vs : m_vesselSets)
        {
            isLast = (counter == (m_vesselSets.size() - 1));

            VesselSetGetTraceCB_t finishedCB =
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
                        auto directionSegment = inAsyncTaskResult.get();
                        auto directionSeries = asyncTaskResult.get();
                        isize_t numTimes = directionSegment->getTimingInfo().getNumTimes();
                        isize_t numBytes = numTimes * sizeof(float);
                        float * vals = directionSeries->getValues();
                        vals += offset;
                        memcpy((char *)vals, (char *)directionSegment->getValues(), numBytes);
                    }
                }

                if (isLast)
                {
                    inCallback(asyncTaskResult);
                }
            };

            vs->getDirectionTraceAsync(inIndex, finishedCB);
            isize_t numSamples = vs->getTimingInfo().getNumTimes();
            offset += numSamples;

            ++counter;
        }
    }

    SpVesselCorrelations_t
    VesselSetSeries::getCorrelations(isize_t inIndex, isize_t inFrameNumber)
    {
        if (inFrameNumber >= m_gaplessTimingInfo.getNumTimes())
        {
            ISX_THROW(ExceptionDataIO, "The index of the frame (", inFrameNumber,
                    ") is out of range (0-", m_gaplessTimingInfo.getNumTimes(), ").");
        }

        size_t vesselSetIndex = 0;
        size_t frameIndex = 0;
        std::tie(vesselSetIndex, frameIndex) = getSegmentAndLocalIndex(getTimingInfosForSeries(), inFrameNumber);

        SpVesselCorrelations_t correlations = m_vesselSets[vesselSetIndex]->getCorrelations(inIndex, frameIndex);
        return correlations;
    }

    void
    VesselSetSeries::getCorrelationsAsync(isize_t inIndex, isize_t inFrameNumber, VesselSetGetCorrelationsCB_t inCallback)
    {
        if (inFrameNumber >= m_gaplessTimingInfo.getNumTimes())
        {
            ISX_THROW(ExceptionDataIO, "The index of the frame (", inFrameNumber,
                    ") is out of range (0-", m_gaplessTimingInfo.getNumTimes(), ").");
        }

        size_t vesselSetIndex = 0;
        size_t frameIndex = 0;
        std::tie(vesselSetIndex, frameIndex) = getSegmentAndLocalIndex(getTimingInfosForSeries(), inFrameNumber);

        std::weak_ptr<VesselSet> weakThis = shared_from_this();

        m_vesselSets[vesselSetIndex]->getCorrelationsAsync(inIndex, frameIndex, [weakThis, inIndex, inCallback](AsyncTaskResult<SpVesselCorrelations_t> inAsyncTaskResult)
            {
                auto sharedThis = weakThis.lock();
                if (!sharedThis)
                {
                    return;
                }

                AsyncTaskResult<SpVesselCorrelations_t> asyncTaskResult;
                if (inAsyncTaskResult.getException())
                {
                    asyncTaskResult.setException(inAsyncTaskResult.getException());
                }
                else
                {
                    auto simpleCorrelations = inAsyncTaskResult.get();
                    asyncTaskResult.setValue(simpleCorrelations);
                }
                inCallback(asyncTaskResult);
            });
    }

    void
    VesselSetSeries::writeImageAndLineAndTrace(
        isize_t inIndex,
        const SpImage_t & inProjectionImage,
        const SpVesselLine_t & inLineEndpoints,
        SpFTrace_t & inTrace,
        const std::string & inName,
        const SpFTrace_t & inDirectionTrace,
        const SpVesselCorrelationsTrace_t & inCorrTrace)
    {
        // see comment above for VesselSetSeries::writeImageAndTrace
        ISX_ASSERT(false);
    }

    VesselSet::VesselStatus
    VesselSetSeries::getVesselStatus(isize_t inIndex)
    {
        return m_vesselSets[0]->getVesselStatus(inIndex);
    }

    Color
    VesselSetSeries::getVesselColor(isize_t inIndex)
    {
        return m_vesselSets[0]->getVesselColor(inIndex);
    }

    std::string
    VesselSetSeries::getVesselStatusString(isize_t inIndex)
    {
        return m_vesselSets[0]->getVesselStatusString(inIndex);
    }

    void
    VesselSetSeries::setVesselStatus(isize_t inIndex, VesselSet::VesselStatus inStatus)
    {
        for(const auto &vs : m_vesselSets)
        {
            vs->setVesselStatus(inIndex, inStatus);
        }
    }

    void
    VesselSetSeries::setVesselColor(isize_t inIndex, const Color& inColor)
    {
        for (const auto &vs : m_vesselSets)
        {
            vs->setVesselColor(inIndex, inColor);
        }
    }

    void
    VesselSetSeries::setVesselColors(const IdColorPairs &inColors)
    {
        for (const auto &vs : m_vesselSets)
        {
            vs->setVesselColors(inColors);
        }
    }

    std::string
    VesselSetSeries::getVesselName(isize_t inIndex)
    {
        return m_vesselSets[0]->getVesselName(inIndex);
    }

    void
    VesselSetSeries::setVesselName(isize_t inIndex, const std::string & inName)
    {
        for(const auto &vs : m_vesselSets)
        {
            vs->setVesselName(inIndex, inName);
        }
    }

    std::vector<bool>
    VesselSetSeries::getVesselActivity(isize_t inIndex) const
    {
        std::vector<bool> activity;
        for(const auto &vs : m_vesselSets)
        {
            std::vector<bool> segment_act = vs->getVesselActivity(inIndex);
            activity.push_back(segment_act.front());
        }
        return activity;
    }

    void
    VesselSetSeries::setVesselActive(isize_t inIndex, const std::vector<bool> & inActive)
    {
        if (inActive.size() != 1)
        {
            ISX_ASSERT(inActive.size() == m_vesselSets.size());
            for(isize_t i(0); i < m_vesselSets.size(); ++i)
            {
                m_vesselSets[i]->setVesselActive(inIndex, {inActive.at(i)});
            }
        }
        else
        {
            for(const auto &vs : m_vesselSets)
            {
                vs->setVesselActive(inIndex, inActive);
            }
        }

    }

    void
    VesselSetSeries::cancelPendingReads()
    {
        for (const auto &vs : m_vesselSets)
        {
            vs->cancelPendingReads();
        }
    }

    std::vector<uint16_t>
    VesselSetSeries::getEfocusValues ()
    {
        // ISX_ASSERT(m_vesselSets.size() == 1);
        return m_vesselSets[0]->getEfocusValues();
    }

    void
    VesselSetSeries::setEfocusValues (const std::vector<uint16_t> &inEfocus)
    {
        ISX_ASSERT(false);
        // placeholder
    };

    std::string
    VesselSetSeries::getExtraProperties() const
    {
        // TODO : Series does not check extra properties being consistent
        return m_vesselSets.front()->getExtraProperties();
    }

    void
    VesselSetSeries::setExtraProperties(const std::string & inProperties)
    {
        for (auto & vs : m_vesselSets)
        {
            vs->setExtraProperties(inProperties);
        }
    }

    SpacingInfo
    VesselSetSeries::getOriginalSpacingInfo() const
    {
        return m_vesselSets.front()->getOriginalSpacingInfo();
    }

    VesselSetType_t
    VesselSetSeries::getVesselSetType() const
    {
        return m_vesselSets.front()->getVesselSetType();
    }

    SizeInPixels_t
    VesselSetSeries::getCorrelationSize(size_t inIndex) const
    {
        return m_vesselSets.front()->getCorrelationSize(inIndex);
    }

    float
    VesselSetSeries::getMaxVelocity(size_t inIndex)
    {
        return m_vesselSets.front()->getMaxVelocity(inIndex);
    }

    bool
    VesselSetSeries::isCorrelationSaved() const
    {
        return m_vesselSets.front()->isCorrelationSaved();   
    }
} // namespace isx
