#include "isxMosaicMovie.h"
#include "isxVideoFrame.h"
#include "isxException.h"
#include "isxLog.h"
#include "isxJsonUtils.h"

#include <fstream>

namespace isx
{

MosaicMovie::MosaicMovie()
    : m_valid(false)
{
}

MosaicMovie::MosaicMovie(const std::string & inFileName)
    : m_valid(false)
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
    const DataType inDataType)
    : m_valid(false)
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

    m_file = std::make_shared<MosaicMovieFile>(inFileName, inTimingInfo, inSpacingInfo, inDataType);
    m_valid = true;
}

MosaicMovie::~MosaicMovie(){}

bool
MosaicMovie::isValid() const
{
    return m_valid;
}

void
MosaicMovie::getFrame(isize_t inFrameNumber, SpU16VideoFrame_t & outFrame)
{
    getFrameTemplate<U16VideoFrame_t>(inFrameNumber, outFrame);
}

void
MosaicMovie::getFrame(isize_t inFrameNumber, SpF32VideoFrame_t & outFrame)
{
    getFrameTemplate<F32VideoFrame_t>(inFrameNumber, outFrame);
}

void
MosaicMovie::getFrameAsync(isize_t inFrameNumber, MovieGetU16FrameCB_t inCallback)
{
    getFrameAsyncTemplate<U16VideoFrame_t>(inFrameNumber, inCallback);
}

void
MosaicMovie::getFrameAsync(isize_t inFrameNumber, MovieGetF32FrameCB_t inCallback)
{
    getFrameAsyncTemplate<F32VideoFrame_t>(inFrameNumber, inCallback);
}

SpAsyncTaskHandle_t
MosaicMovie::unregisterReadRequest(uint64_t inReadRequestId)
{
    ScopedMutex locker(m_pendingReadsMutex, "unregisterPendingRead");
    auto ret = m_pendingReads[inReadRequestId];
    m_pendingReads.erase(inReadRequestId);
    return ret;
}
    
void
MosaicMovie::cancelPendingReads()
{
    ScopedMutex locker(m_pendingReadsMutex, "cancelPendingReads");
    for (auto & pr: m_pendingReads)
    {
        pr.second->cancel();
    }
    m_pendingReads.clear();
}

void
MosaicMovie::writeFrame(const SpU16VideoFrame_t & inVideoFrame)
{
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
}

const isx::TimingInfo &
MosaicMovie::getTimingInfo() const
{
    return m_file->getTimingInfo();
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
MosaicMovie::getName() const
{
    return m_file->getFileName();
}

void
MosaicMovie::serialize(std::ostream & strm) const
{
    strm << getName();
}

} // namespace isx
