#include "isxVesselCorrelationsMovie.h"
#include "isxException.h"
#include "isxMutex.h"
#include "isxConditionVariable.h"


namespace isx
{

VesselCorrelationsMovie::VesselCorrelationsMovie()
{}

VesselCorrelationsMovie::VesselCorrelationsMovie(
    const SpVesselSet_t & inVesselSet,
    const size_t inVesselId)
    : m_vesselSet(inVesselSet)
    , m_vesselId(inVesselId)
    , m_gaplessTimingInfo(inVesselSet->getTimingInfo())
    , m_timingInfos(inVesselSet->getTimingInfosForSeries())
{
    if (!inVesselSet->isCorrelationSaved())
    {
        ISX_THROW(ExceptionFileIO, "No correlation heatmaps saved to input vessel set.");
    }
    
    m_spacingInfo = isx::SpacingInfo(isx::computeTriptychSize(inVesselSet->getCorrelationSize(inVesselId)));
}

bool
VesselCorrelationsMovie::isValid()
const
{
    return m_vesselSet->isValid();
}

SpVideoFrame_t
VesselCorrelationsMovie::getFrame(isize_t inFrameNumber)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getFrame");
    AsyncTaskResult<SpVideoFrame_t> asyncTaskResult;
    getFrameAsync(inFrameNumber,
        [&asyncTaskResult, &cv, &mutex](AsyncTaskResult<SpVideoFrame_t> inAsyncTaskResult)
        {
            mutex.lock("getFrame async");
            asyncTaskResult = inAsyncTaskResult;
            mutex.unlock();
            cv.notifyOne();
        }
    );
    cv.wait(mutex);
    mutex.unlock();

    return asyncTaskResult.get();   // throws if asyncTaskResult contains an exception
}

void
VesselCorrelationsMovie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    if (inFrameNumber >= m_gaplessTimingInfo.getNumTimes())
    {
        ISX_THROW(ExceptionDataIO, "The index of the frame (", inFrameNumber,
                ") is out of range (0-", m_gaplessTimingInfo.getNumTimes(), ").");
    }

    size_t seriesIndex = 0;
    size_t frameIndex = 0;
    std::tie(seriesIndex, frameIndex) = getSegmentAndLocalIndex(m_timingInfos, inFrameNumber);

    std::weak_ptr<VesselCorrelationsMovie> weakThis = shared_from_this();
    m_vesselSet->getCorrelationsAsync(m_vesselId, inFrameNumber, [weakThis, this, inCallback, inFrameNumber, seriesIndex, frameIndex](isx::AsyncTaskResult<isx::SpVesselCorrelations_t> inAsyncTaskResult)
    {
        auto sharedThis = weakThis.lock();
        if (!sharedThis)
        {
            return;
        }
    
        SpVesselCorrelations_t correlations = inAsyncTaskResult.get();
        AsyncTaskResult<SpVideoFrame_t> atr;
        if (correlations)
        {
            SpVideoFrame_t frame = std::make_shared<VideoFrame>(
                sharedThis->getSpacingInfo(),
                sharedThis->getSpacingInfo().getNumColumns() * sizeof(float),
                1,
                sharedThis->getDataType(),
                sharedThis->getTimingInfosForSeries().at(seriesIndex).convertIndexToStartTime(frameIndex),
                inFrameNumber);
            SpImage_t heatmap = correlations->getHeatmaps();
            std::memcpy(frame->getPixelsAsF32(), heatmap->getPixelsAsF32(), heatmap->getImageSizeInBytes());
            atr.setValue(frame);
        }
        else
        {
            atr.setValue(nullptr);
        }
        inCallback(atr);
    });
}

void
VesselCorrelationsMovie::cancelPendingReads()
{
    m_vesselSet->cancelPendingReads();
}

const TimingInfo &
VesselCorrelationsMovie::getTimingInfo() const
{
    return m_gaplessTimingInfo;
}

const TimingInfos_t &
VesselCorrelationsMovie::getTimingInfosForSeries() const
{
    return m_timingInfos;
}

const SpacingInfo &
VesselCorrelationsMovie::getSpacingInfo() const
{
    return m_spacingInfo;
}

DataType
VesselCorrelationsMovie::getDataType() const
{
    return DataType::F32;
}

std::string
VesselCorrelationsMovie::getFileName() const
{
    return m_vesselSet->getFileName();
}

void
VesselCorrelationsMovie::serialize(std::ostream & strm) const
{
    strm << getFileName();
}

std::string
VesselCorrelationsMovie::getExtraProperties() const
{
    return m_vesselSet->getExtraProperties();
}

} // namespace isx
