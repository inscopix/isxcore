#include "isxNVisionMovie.h"
#include "isxVideoFrame.h"
#include "isxException.h"
#include "isxLog.h"
#include "isxIoQueue.h"
#include "isxConditionVariable.h"
#include "isxIoTaskTracker.h"

#include <fstream>

namespace isx
{

NVisionMovie::NVisionMovie()
    : m_valid(false)
{
}

NVisionMovie::NVisionMovie(const std::string & inFileName)
    : m_valid(false)
    , m_ioTaskTracker(new IoTaskTracker<VideoFrame>())
{
    m_file = std::make_shared<NVisionMovieFile>(inFileName);
    m_valid = true;
}

bool
NVisionMovie::isValid() const
{
    return m_valid;
}

SpVideoFrame_t
NVisionMovie::getFrame(isize_t inFrameNumber)
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

    return asyncTaskResult.get();   // will throw if asyncTaskResult contains an exception
}

void
NVisionMovie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    std::weak_ptr<NVisionMovie> weakThis = shared_from_this();
    GetFrameCB_t getFrameCB = 
        [weakThis, this, inFrameNumber]()
        {
            auto sharedThis = weakThis.lock();
            if (sharedThis)
            {
                return m_file->readFrame(inFrameNumber);
            }
            return SpVideoFrame_t();
        };

    m_ioTaskTracker->schedule(getFrameCB, inCallback);
}
    
void
NVisionMovie::cancelPendingReads()
{
    m_ioTaskTracker->cancelPendingTasks();
}

const isx::TimingInfo &
NVisionMovie::getTimingInfo() const
{
    return m_file->getTimingInfo();
}

const isx::TimingInfos_t &
NVisionMovie::getTimingInfosForSeries() const
{
    return m_file->getTimingInfosForSeries();
}

const isx::SpacingInfo &
NVisionMovie::getSpacingInfo() const
{
    return m_file->getSpacingInfo();
}

DataType
NVisionMovie::getDataType() const
{
    return m_file->getDataType();
}

std::string
NVisionMovie::getFileName() const
{
    return m_file->getFileName();
}

void
NVisionMovie::serialize(std::ostream & strm) const
{
    strm << getFileName();
}


bool
NVisionMovie::hasFrameTimestamps() const
{
    return m_file->hasFrameTimestamps();
}

uint64_t
NVisionMovie::getFrameTimestamp(const isize_t inIndex)
{
    return m_file->readFrameTimestamp(inIndex);
}

std::string
NVisionMovie::getExtraProperties() const
{
    return m_file->getExtraProperties();
}

} // namespace isx
