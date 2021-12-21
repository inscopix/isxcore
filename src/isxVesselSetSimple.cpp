#include "isxVesselSetSimple.h"
#include "isxVesselSetFile.h"
#include "isxConditionVariable.h"
#include "isxIoTask.h"
#include "isxIoTaskTracker.h"

namespace isx
{

VesselSetSimple::VesselSetSimple()
{
}

VesselSetSimple::VesselSetSimple(const std::string & inFileName, bool enableWrite)
    : m_valid(false)
    , m_traceIoTaskTracker(new IoTaskTracker<FTrace_t>())
    , m_imageIoTaskTracker(new IoTaskTracker<Image>())
    , m_lineEndpointsIoTaskTracker(new IoTaskTracker<VesselLine>())
    , m_directionIoTaskTracker(new IoTaskTracker<Trace<float>>())
    , m_corrIoTaskTracker(new IoTaskTracker<VesselCorrelations>())
{
    m_file = std::make_shared<VesselSetFile>(inFileName, enableWrite);
    m_valid = true;
}

VesselSetSimple::VesselSetSimple(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo,
        const VesselSetType_t inVesselSetType)
    : m_valid(false)
    , m_traceIoTaskTracker(new IoTaskTracker<FTrace_t>())
    , m_imageIoTaskTracker(new IoTaskTracker<Image>())
    , m_lineEndpointsIoTaskTracker(new IoTaskTracker<VesselLine>())
    , m_directionIoTaskTracker(new IoTaskTracker<Trace<float>>())
    , m_corrIoTaskTracker(new IoTaskTracker<VesselCorrelations>())
{
    m_file = std::make_shared<VesselSetFile>(inFileName, inTimingInfo, inSpacingInfo, inVesselSetType);
    m_valid = true;
}

VesselSetSimple::~VesselSetSimple()
{
}

bool
VesselSetSimple::isValid() const
{
    return m_valid;
}

void
VesselSetSimple::closeForWriting()
{
    m_file->closeForWriting();
}

std::string
VesselSetSimple::getFileName() const
{
    return m_file->getFileName();
}

const isize_t
VesselSetSimple::getNumVessels()
{
    return m_file->numberOfVessels();
}

isx::TimingInfo
VesselSetSimple::getTimingInfo() const
{
    return m_file->getTimingInfo();
}

isx::TimingInfos_t
VesselSetSimple::getTimingInfosForSeries() const
{
    TimingInfos_t tis = TimingInfos_t{m_file->getTimingInfo()};
    return tis;
}

isx::SpacingInfo
VesselSetSimple::getSpacingInfo() const
{
    return m_file->getSpacingInfo();
}

SpFTrace_t
VesselSetSimple::getTrace(isize_t inIndex)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getTrace");
    AsyncTaskResult<SpFTrace_t> asyncTaskResult;
    getTraceAsync(inIndex, [&asyncTaskResult, &cv, &mutex](AsyncTaskResult<SpFTrace_t> inAsyncTaskResult)
        {
            mutex.lock("getTrace async");
            asyncTaskResult = inAsyncTaskResult;
            mutex.unlock();
            cv.notifyOne();
        }
    );
    cv.wait(mutex);
    mutex.unlock();

    return asyncTaskResult.get();   // throws is asyncTaskResult contains an exception
}

void
VesselSetSimple::getTraceAsync(isize_t inIndex, VesselSetGetTraceCB_t inCallback)
{
    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<VesselSetSimple> weakThis = shared_from_this();
    GetTraceCB_t getTraceCB = [weakThis, this, inIndex]()
        {
            auto sharedThis = weakThis.lock();
            if (sharedThis)
            {
                return m_file->readTrace(inIndex);
            }
            return SpFTrace_t();
        };
    m_traceIoTaskTracker->schedule(getTraceCB, inCallback);
}

SpImage_t
VesselSetSimple::getImage(isize_t inIndex)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getImage");
    AsyncTaskResult<SpImage_t> asyncTaskResult;
    getImageAsync(inIndex,
        [&asyncTaskResult, &cv, &mutex](AsyncTaskResult<SpImage_t> inAsyncTaskResult)
        {
            mutex.lock("getImage async");
            asyncTaskResult = inAsyncTaskResult;
            mutex.unlock();
            cv.notifyOne();
        }
    );
    cv.wait(mutex);
    mutex.unlock();

    return asyncTaskResult.get();   // throws if asyncTaskResults contains an exception
}

void
VesselSetSimple::getImageAsync(isize_t inIndex, VesselSetGetImageCB_t inCallback)
{
    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<VesselSetSimple> weakThis = shared_from_this();
    GetImageCB_t getImageCB = [weakThis, this, inIndex]()
        {
            auto sharedThis = weakThis.lock();
            SpImage_t im;
            if (sharedThis)
            {
                im = m_file->readProjectionImage();
            }
            return im;
        };
    m_imageIoTaskTracker->schedule(getImageCB, inCallback);
}

SpVesselLine_t
VesselSetSimple::getLineEndpoints(isize_t inIndex)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getLineEndpoints");
    AsyncTaskResult<SpVesselLine_t> asyncTaskResult;
    getLineEndpointsAsync(inIndex,
      [&asyncTaskResult, &cv, &mutex](AsyncTaskResult<SpVesselLine_t> inAsyncTaskResult)
      {
          mutex.lock("getLineEndpoints async");
          asyncTaskResult = inAsyncTaskResult;
          mutex.unlock();
          cv.notifyOne();
      }
    );
    cv.wait(mutex);
    mutex.unlock();

    return asyncTaskResult.get();   // throws if asyncTaskResults contains an exception
}

void
VesselSetSimple::getLineEndpointsAsync(isize_t inIndex, VesselSetGetLineEndpointsCB_t inCallback)
{
    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<VesselSetSimple> weakThis = shared_from_this();
    GetLineEndpointsCB_t getLineEndpointsCB = [weakThis, this, inIndex]()
    {
        auto sharedThis = weakThis.lock();
        SpVesselLine_t vl;
        if (sharedThis)
        {
            vl = m_file->readLineEndpoints(inIndex);
        }
        return vl;
    };
    m_lineEndpointsIoTaskTracker->schedule(getLineEndpointsCB, inCallback);
}

SpFTrace_t
VesselSetSimple::getDirectionTrace(isize_t inIndex)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getDirectionTrace");
    AsyncTaskResult<SpFTrace_t> asyncTaskResult;
    getDirectionTraceAsync(inIndex,
      [&asyncTaskResult, &cv, &mutex](AsyncTaskResult<SpFTrace_t> inAsyncTaskResult)
      {
          mutex.lock("getDirectionTrace async");
          asyncTaskResult = inAsyncTaskResult;
          mutex.unlock();
          cv.notifyOne();
      }
    );
    cv.wait(mutex);
    mutex.unlock();

    return asyncTaskResult.get();   // throws if asyncTaskResults contains an exception
}

void
VesselSetSimple::getDirectionTraceAsync(isize_t inIndex, VesselSetGetTraceCB_t inCallback)
{
    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<VesselSetSimple> weakThis = shared_from_this();
    GetTraceCB_t getDirectionTraceCB = [weakThis, this, inIndex]()
    {
        auto sharedThis = weakThis.lock();
        SpFTrace_t vl;
        if (sharedThis)
        {
            vl = m_file->readDirectionTrace(inIndex);
        }
        return vl;
    };
    m_directionIoTaskTracker->schedule(getDirectionTraceCB, inCallback);
}

SpVesselCorrelations_t
VesselSetSimple::getCorrelations(isize_t inIndex, isize_t inFrameNumber)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getCorrelations");
    AsyncTaskResult<SpVesselCorrelations_t> asyncTaskResult;
    getCorrelationsAsync(inIndex, inFrameNumber,
      [&asyncTaskResult, &cv, &mutex](AsyncTaskResult<SpVesselCorrelations_t> inAsyncTaskResult)
      {
          mutex.lock("getCorrelations async");
          asyncTaskResult = inAsyncTaskResult;
          mutex.unlock();
          cv.notifyOne();
      }
    );
    cv.wait(mutex);
    mutex.unlock();

    return asyncTaskResult.get();   // throws if asyncTaskResults contains an exception
}

void
VesselSetSimple::getCorrelationsAsync(isize_t inIndex, isize_t inFrameNumber, VesselSetGetCorrelationsCB_t inCallback)
{
    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<VesselSetSimple> weakThis = shared_from_this();
    GetCorrelationsCB_t getCorrelationsCB = [weakThis, this, inIndex, inFrameNumber]()
    {
        auto sharedThis = weakThis.lock();
        SpVesselCorrelations_t vl;
        if (sharedThis)
        {
            vl = m_file->readCorrelations(inIndex, inFrameNumber);
        }
        return vl;
    };
    m_corrIoTaskTracker->schedule(getCorrelationsCB, inCallback);
}

void
VesselSetSimple::writeImageAndLineAndTrace(
        isize_t inIndex,
        const SpImage_t & inProjectionImage,
        const SpVesselLine_t & inLineEndpoints,
        SpFTrace_t & inTrace,
        const std::string & inName,
        const SpFTrace_t & inDirectionTrace,
        const SpVesselCorrelationsTrace_t & inCorrTrace)
{
    // Get a new shared pointer to the file, so we can guarantee the write.
    std::shared_ptr<VesselSetFile> file = m_file;
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("VesselSetSimple::writeImageAndLineAndTrace");
    auto writeIoTask = std::make_shared<IoTask>(
        [file, inIndex, inProjectionImage, inLineEndpoints, inTrace, inName, inDirectionTrace, inCorrTrace]()
        {
            file->writeVesselData(inIndex, *inProjectionImage, inLineEndpoints, *inTrace, inName, inDirectionTrace, inCorrTrace);
        },
        [&cv, &mutex](AsyncTaskStatus inStatus)
        {
            if (inStatus != AsyncTaskStatus::COMPLETE)
            {
                ISX_LOG_ERROR("An error occurred while writing image and trace data to a VesselSet.");
            }
            // will only be able to take lock when client reaches cv.wait
            mutex.lock("VesselSetwriteImageAndLineAndTrace finished");
            mutex.unlock();
            cv.notifyOne();
        });
    writeIoTask->schedule();
    cv.wait(mutex);
    mutex.unlock();

    if(writeIoTask->getTaskStatus() == AsyncTaskStatus::ERROR_EXCEPTION)
    {
        std::rethrow_exception(writeIoTask->getExceptionPtr());
    }
}

VesselSet::VesselStatus
VesselSetSimple::getVesselStatus(isize_t inIndex)
{
    return m_file->getVesselStatus(inIndex);
}

Color
VesselSetSimple::getVesselColor(isize_t inIndex)
{
    return m_file->getVesselColor(inIndex);
}

std::string
VesselSetSimple::getVesselStatusString(isize_t inIndex)
{
    return m_file->getVesselStatusString(inIndex);
}

void
VesselSetSimple::setVesselStatus(isize_t inIndex, VesselSet::VesselStatus inStatus)
{
    m_file->setVesselStatus(inIndex, inStatus);
}

void
VesselSetSimple::setVesselColor(isize_t inIndex, const Color& inColor)
{
    m_file->setVesselColor(inIndex, inColor);
}

void
VesselSetSimple::setVesselColors(const IdColorPairs &inColors)
{
    m_file->setVesselColors(inColors);
}

std::string
VesselSetSimple::getVesselName(isize_t inIndex)
{
    return m_file->getVesselName(inIndex);
}

void
VesselSetSimple::setVesselName(isize_t inIndex, const std::string & inName)
{
    m_file->setVesselName(inIndex, inName);
}

std::vector<bool>
VesselSetSimple::getVesselActivity(isize_t inIndex) const
{
    return {m_file->isVesselActive(inIndex)};
}

void
VesselSetSimple::setVesselActive(isize_t inIndex, const std::vector<bool> & inActive)
{
    ISX_ASSERT(inActive.size() == 1);
    m_file->setVesselActive(inIndex, inActive.front());
}

void
VesselSetSimple::cancelPendingReads()
{
    m_imageIoTaskTracker->cancelPendingTasks();
    m_traceIoTaskTracker->cancelPendingTasks();
    m_lineEndpointsIoTaskTracker->cancelPendingTasks();
    m_directionIoTaskTracker->cancelPendingTasks();
    m_corrIoTaskTracker->cancelPendingTasks();
}

std::vector<uint16_t>
VesselSetSimple::getEfocusValues ()
{
    return m_file->getEfocusValues();
}

void
VesselSetSimple::setEfocusValues (const std::vector<uint16_t> &inEfocus)
{
    m_file->setEfocusValues(inEfocus);
};

std::string
VesselSetSimple::getExtraProperties() const
{
    return m_file->getExtraProperties();
}

void
VesselSetSimple::setExtraProperties(const std::string & inProperties)
{
    m_file->setExtraProperties(inProperties);
}

SpacingInfo
VesselSetSimple::getOriginalSpacingInfo() const
{
    return m_file->getOriginalSpacingInfo();
}

VesselSetType_t
VesselSetSimple::getVesselSetType() const
{
    return m_file->getVesselSetType();
}

SizeInPixels_t
VesselSetSimple::getCorrelationSize(size_t inIndex) const
{
    return m_file->getCorrelationSize(inIndex);
}

float
VesselSetSimple::getMaxVelocity(size_t inIndex)
{
    SpVesselLine_t vesselLine = getLineEndpoints(inIndex);
    TimingInfo timingInfo = getTimingInfo();
    const float fps = static_cast<float>(timingInfo.getStep().getInverse().toDouble());
    auto maxVelocity = vesselLine->computeMaxVelocity(fps);

    auto units = isx::getVesselSetUnits(m_file);
    if (units == isx::VesselSetUnits_t::MICRONS_PER_SECOND)
    {
        maxVelocity *= static_cast<float>(isx::getMicronsPerPixel(m_file));
    }

    return maxVelocity;
}

} // namespace isx
