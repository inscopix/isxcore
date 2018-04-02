#include "isxMosaicMovie.h"
#include "isxVideoFrame.h"
#include "isxException.h"
#include "isxLog.h"
#include "isxJsonUtils.h"
#include "isxMutex.h"
#include "isxIoQueue.h"
#include "isxConditionVariable.h"
#include "isxIoTaskTracker.h"

#include <fstream>

namespace isx
{

MosaicMovie::MosaicMovie()
    : m_valid(false)
{
}

MosaicMovie::MosaicMovie(const std::string & inFileName)
    : m_valid(false)
    , m_ioTaskTracker(new IoTaskTracker<VideoFrame>())
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

    m_file = std::make_shared<MosaicMovieFile>(inFileName);
    m_valid = true;
}

MosaicMovie::MosaicMovie(
    const std::string & inFileName,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo,
    const DataType inDataType,
    const bool inHasFrameHeaderFooter)
    : m_valid(false)
    , m_ioTaskTracker(new IoTaskTracker<VideoFrame>())
{
    // TODO sweet : decide if we want all IO on the IO thread or if
    // it's okay to write the header on the current thread.
    // If we decide on the former, use this code.

    //std::shared_ptr<MosaicMovieFile> file = std::make_shared<MosaicMovieFile>();
    //Mutex mutex;
    //ConditionVariable cv;
    //mutex.lock("MosaicMovie write");
    //IoQueue::instance()->enqueue(
    //    IoQueue::IoTask(
    //        [&file, &inFileName, &inTimingInfo, &inSpacingInfo, &cv, &mutex]()
    //        {
    //            mutex.lock("MosaicMovieFile write initialize");
    //            file->initialize(inFileName, inTimingInfo, inSpacingInfo);
    //            mutex.unlock();
    //            cv.notifyOne();
    //        }
    //        ,
    //        [](AsyncTaskStatus inStatus)
    //        {
    //            if (inStatus == AsyncTaskStatus::ERROR_EXCEPTION)
    //            {
    //                ISX_LOG_ERROR("An exception occurred while initializing a MosaicMovieFile for writing.");
    //            }
    //            else if (inStatus != AsyncTaskStatus::COMPLETE)
    //            {
    //                ISX_LOG_ERROR("An error occurred while initializing a MosaicMovieFile for writing.");
    //            }
    //        }
    //    )
    //);
    //cv.wait(mutex);
    //mutex.unlock();
    //m_file = file;

    m_file = std::make_shared<MosaicMovieFile>(inFileName, inTimingInfo, inSpacingInfo, inDataType, inHasFrameHeaderFooter);
    m_valid = true;
}

bool
MosaicMovie::isValid() const
{
    return m_valid;
}

SpVideoFrame_t
MosaicMovie::getFrame(isize_t inFrameNumber)
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
MosaicMovie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    std::weak_ptr<MosaicMovie> weakThis = shared_from_this();
    GetFrameCB_t getFrameCB = [weakThis, this, inFrameNumber]()
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
MosaicMovie::cancelPendingReads()
{
    m_ioTaskTracker->cancelPendingTasks();
}

void
MosaicMovie::writeFrame(const SpVideoFrame_t & inVideoFrame)
{
    const TimingInfo & ti = getTimingInfo();
    if (!ti.isIndexValid(inVideoFrame->getFrameIndex()))
    {
        ISX_ASSERT(false, "Attempt to write invalid frame.");
        return;
    }

    // Get a new shared pointer to the file, so we can guarantee the write.
    std::shared_ptr<MosaicMovieFile> file = m_file;
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("writeFrame");
    auto writeIoTask = std::make_shared<IoTask>(
        [file, inVideoFrame]()
        {
            file->writeFrame(inVideoFrame);
        },
        [&cv, &mutex](AsyncTaskStatus inStatus)
        {
            if (inStatus != AsyncTaskStatus::COMPLETE)
            {
                ISX_LOG_ERROR("An error occurred while writing a frame to a MosaicMovieFile.");
            }
            mutex.lock("writeFrame finished");  // will only be able to take lock when client reaches cv.waitForMs
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

void
MosaicMovie::closeForWriting(const TimingInfo & inTimingInfo)
{
    m_file->closeForWriting(inTimingInfo);
}

SpVideoFrame_t
MosaicMovie::makeVideoFrame(isize_t inIndex, const bool inWithHeaderFooter)
{
    return m_file->makeVideoFrame(inIndex, inWithHeaderFooter);
}

const isx::TimingInfo &
MosaicMovie::getTimingInfo() const
{
    return m_file->getTimingInfo();
}

const isx::TimingInfos_t &
MosaicMovie::getTimingInfosForSeries() const
{
    return m_file->getTimingInfosForSeries();
}

const isx::SpacingInfo &
MosaicMovie::getSpacingInfo() const
{
    return m_file->getSpacingInfo();
}

DataType
MosaicMovie::getDataType() const
{
    return m_file->getDataType();
}

std::string
MosaicMovie::getFileName() const
{
    return m_file->getFileName();
}

void
MosaicMovie::serialize(std::ostream & strm) const
{
    strm << getFileName();
}

void
MosaicMovie::setExtraProperties(const std::string & inProperties)
{
    m_file->setExtraProperties(inProperties);
}

std::string
MosaicMovie::getExtraProperties() const
{
    return m_file->getExtraProperties();
}

SpVideoFrame_t
MosaicMovie::getFrameWithHeaderFooter(const size_t inFrameNumber)
{
    std::weak_ptr<MosaicMovie> weakThis = shared_from_this();
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getFrameWithHeaderFooter");
    AsyncTaskResult<SpVideoFrame_t> asyncTaskResult;

    GetFrameCB_t getFrameCB = [weakThis, this, inFrameNumber]()
    {
        auto sharedThis = weakThis.lock();
        if (sharedThis)
        {
            return m_file->readFrame(inFrameNumber, true);
        }
        return SpVideoFrame_t();
    };

    MovieGetFrameCB_t asyncCB = [&asyncTaskResult, &cv, &mutex](AsyncTaskResult<SpVideoFrame_t> inAsyncTaskResult)
    {
        mutex.lock("getFrame async");
        asyncTaskResult = inAsyncTaskResult;
        mutex.unlock();
        cv.notifyOne();
    };

    m_ioTaskTracker->schedule(getFrameCB, asyncCB);

    cv.wait(mutex);
    mutex.unlock();

    return asyncTaskResult.get();
}

} // namespace isx
