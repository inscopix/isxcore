#include "isxBehavMovie.h"
#include "isxVideoFrame.h"
#include "isxException.h"
#include "isxLog.h"
#include "isxIoQueue.h"
#include "isxConditionVariable.h"
#include "isxIoTaskTracker.h"

#include <fstream>

namespace isx
{

BehavMovie::BehavMovie()
    : m_valid(false)
{
}

BehavMovie::BehavMovie(const std::string & inFileName, const DataSet::Properties & inProperties)
    : m_valid(false)
    , m_ioTaskTracker(new IoTaskTracker<VideoFrame>())
{
    m_file = std::make_shared<BehavMovieFile>(inFileName, inProperties);
    m_valid = true;
}

bool
BehavMovie::isValid() const
{
    return m_valid;
}

SpVideoFrame_t
BehavMovie::getFrame(isize_t inFrameNumber)
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
BehavMovie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    std::weak_ptr<BehavMovie> weakThis = shared_from_this();
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
BehavMovie::cancelPendingReads()
{
    m_ioTaskTracker->cancelPendingTasks();
}

const isx::TimingInfo &
BehavMovie::getTimingInfo() const
{
    return m_file->getTimingInfo();
}

const isx::TimingInfos_t &
BehavMovie::getTimingInfosForSeries() const
{
    return m_file->getTimingInfosForSeries();
}

const isx::SpacingInfo &
BehavMovie::getSpacingInfo() const
{
    return m_file->getSpacingInfo();
}

DataType
BehavMovie::getDataType() const
{
    return m_file->getDataType();
}

std::string
BehavMovie::getFileName() const
{
    return m_file->getFileName();
}

void
BehavMovie::serialize(std::ostream & strm) const
{
    strm << getFileName();
}

} // namespace isx
