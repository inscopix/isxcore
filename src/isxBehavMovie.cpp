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

BehavMovie::BehavMovie(const std::string & inFileName)
    : m_valid(false)
    , m_ioTaskTracker(new IoTaskTracker())
{
    // TODO sweet : decide if we want all IO on the IO thread or if
    // it's okay to read the header on the current thread.
    // If we decide on the former, use this code.

    //std::shared_ptr<MosaicMovieFile> file = std::make_shared<MosaicMovieFile>();
    //Mutex mutex;
    //ConditionVariable cv;
    //mutex.lock("MosaicMovie read");
    //IoQueue::instance()->enqueue(
    //    IoQueue::IoTask(
    //        [&file, &inFileName, &cv, &mutex]()
    //        {
    //            mutex.lock("MosaicMovieFile read initialize");
    //            file->initialize(inFileName);
    //            mutex.unlock();
    //            cv.notifyOne();
    //        }
    //        ,
    //        [](AsyncTaskStatus inStatus)
    //        {
    //            if (inStatus == AsyncTaskStatus::ERROR_EXCEPTION)
    //            {
    //                ISX_LOG_ERROR("An exception occurred while initializing a MosaicMovieFile for reading.");
    //            }
    //            else if (inStatus != AsyncTaskStatus::COMPLETE)
    //            {
    //                ISX_LOG_ERROR("An error occurred while initializing a MosaicMovieFile for reading.");
    //            }
    //        }
    //    )
    //);
    //cv.wait(mutex);
    //mutex.unlock();
    //m_file = file;

    m_file = std::make_shared<BehavMovieFile>(inFileName);
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
    SpVideoFrame_t outFrame;
    getFrameAsync(inFrameNumber,
        [&outFrame, &cv, &mutex](const SpVideoFrame_t & inFrame)
        {
            mutex.lock("getFrame async");
            outFrame = inFrame;
            mutex.unlock();
            cv.notifyOne();
        }
    );
    cv.wait(mutex);
    mutex.unlock();
    return outFrame;
}

void
BehavMovie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    std::weak_ptr<BehavMovie> weakThis = shared_from_this();
    m_ioTaskTracker->schedule([weakThis, this, inFrameNumber]()
    {
        auto sharedThis = weakThis.lock();
        if (sharedThis)
        {
            return m_file->readFrame(inFrameNumber);
        }
        return SpVideoFrame_t();
    },
    inCallback);
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
