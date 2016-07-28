#include "isxMosaicMovie.h"
#include "isxVideoFrame.h"
#include "isxException.h"
#include "isxLog.h"
#include "isxJsonUtils.h"
#include "isxMutex.h"
#include "isxIoQueue.h"
#include "isxConditionVariable.h"

#include <fstream>

namespace isx
{

// The number of milliseconds to wait for the read constructor.
// This is short because this should only need to read the header.
const uint32_t readConstructorWaitMs = 1000;

// The number of milliseconds to wait for the write constructor.
// This is long to allow for zero-ing out of large files.
const uint32_t writeConstructorWaitMs = 10000;

// The number of milliseconds to wait for getting one frame.
// This is short because reading one frame should be fast.
const uint32_t getFrameWaitMs = 1000;

// The number of milliseconds to wait for writing a frame.
// This is short because writing one frame should be fast.
const uint32_t writeFrameWaitMs = 1000;

MosaicMovie::MosaicMovie()
    : m_valid(false)
{
}

MosaicMovie::MosaicMovie(const std::string & inFileName)
    : m_valid(false)
{
    std::shared_ptr<MosaicMovieFile> file = std::make_shared<MosaicMovieFile>();

    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("MosaicMovie read");
    IoQueue::instance()->enqueue(
        IoQueue::IoTask(
            [&file, &inFileName, &cv, &mutex]()
            {
                mutex.lock("MosaicMovieFile read initialize");
                file->initialize(inFileName);
                mutex.unlock();
                cv.notifyOne();
            }
            ,
            [](AsyncTaskStatus inStatus)
            {
                if (inStatus == AsyncTaskStatus::ERROR_EXCEPTION)
                {
                    ISX_LOG_ERROR("An exception occurred while initializing a MosaicMovieFile for reading.");
                }
                else if (inStatus != AsyncTaskStatus::COMPLETE)
                {
                    ISX_LOG_ERROR("An error occurred while initializing a MosaicMovieFile for reading.");
                }
            }
        )
    );
    bool didNotTimeOut = cv.waitForMs(mutex, readConstructorWaitMs);
    mutex.unlock();
    if (didNotTimeOut == false)
    {
        ISX_THROW(isx::ExceptionDataIO, "MosaicMovie read timed out.");
    }

    m_file = file;
    m_valid = true;
}

MosaicMovie::MosaicMovie(
    const std::string & inFileName,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo)
    : m_valid(false)
{
    std::shared_ptr<MosaicMovieFile> file = std::make_shared<MosaicMovieFile>();

    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("MosaicMovie write");
    IoQueue::instance()->enqueue(
        IoQueue::IoTask(
            [&file, &inFileName, &inTimingInfo, &inSpacingInfo, &cv, &mutex]()
            {
                mutex.lock("MosaicMovieFile write initialize");
                file->initialize(inFileName, inTimingInfo, inSpacingInfo);
                mutex.unlock();
                cv.notifyOne();
            }
            ,
            [](AsyncTaskStatus inStatus)
            {
                if (inStatus == AsyncTaskStatus::ERROR_EXCEPTION)
                {
                    ISX_LOG_ERROR("An exception occurred while initializing a MosaicMovieFile for writing.");
                }
                else if (inStatus != AsyncTaskStatus::COMPLETE)
                {
                    ISX_LOG_ERROR("An error occurred while initializing a MosaicMovieFile for writing.");
                }
            }
        )
    );
    bool didNotTimeOut = cv.waitForMs(mutex, writeConstructorWaitMs);
    mutex.unlock();
    if (didNotTimeOut == false)
    {
        ISX_THROW(isx::ExceptionDataIO, "MosaicMovie write timed out.");
    }

    m_file = file;
    m_valid = true;
}

bool
MosaicMovie::isValid() const
{
    return m_valid;
}

SpU16VideoFrame_t
MosaicMovie::getFrame(isize_t inFrameNumber)
{
    SpU16VideoFrame_t outVideoFrame;
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getFrame");
    getFrameAsync(inFrameNumber,
        [&outVideoFrame, &cv, &mutex](const SpU16VideoFrame_t & inVideoFrame)
        {
            mutex.lock("getFrame async");
            outVideoFrame = inVideoFrame;
            mutex.unlock();
            cv.notifyOne();
        }
    );
    bool didNotTimeOut = cv.waitForMs(mutex, getFrameWaitMs);
    mutex.unlock();
    if (didNotTimeOut == false)
    {
        ISX_THROW(isx::ExceptionDataIO, "getFrame timed out.\n");
    }
    return outVideoFrame;
}

SpU16VideoFrame_t
MosaicMovie::getFrame(const Time & inTime)
{
    isize_t frameNum = getTimingInfo().convertTimeToIndex(inTime);
    return getFrame(frameNum);
}

void
MosaicMovie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    std::shared_ptr<MosaicMovieFile> file = m_file;
    IoQueue::instance()->enqueue(
        IoQueue::IoTask(
            [file, inFrameNumber, inCallback]()
            {
                inCallback(file->readFrame(inFrameNumber));
            },
            [inCallback](AsyncTaskStatus inStatus)
            {
                if (inStatus == AsyncTaskStatus::ERROR_EXCEPTION)
                {
                    ISX_LOG_ERROR("An exception occurred while reading a frame from a MosaicMovie file.");
                    inCallback(SpU16VideoFrame_t());
                }
                else if (inStatus != AsyncTaskStatus::COMPLETE)
                {
                    ISX_LOG_ERROR("An error occurred while reading a frame from a MosaicMovie file.");
                }
            }
        )
    );
}

void
MosaicMovie::getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback)
{
    isize_t frameNumber = getTimingInfo().convertTimeToIndex(inTime);
    getFrameAsync(frameNumber, inCallback);
}

void
MosaicMovie::writeFrame(const SpU16VideoFrame_t & inVideoFrame)
{
    if (!m_valid)
    {
        ISX_THROW(isx::ExceptionFileIO, "Writing frame to invalid movie.");
    }
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("writeFrame");
    std::shared_ptr<MosaicMovieFile> file = m_file;
    IoQueue::instance()->enqueue(
        IoQueue::IoTask(
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
            }
        )
    );
    bool didNotTimeOut = cv.waitForMs(mutex, writeFrameWaitMs);
    mutex.unlock();
    if (didNotTimeOut == false)
    {
        ISX_THROW(isx::ExceptionDataIO, "writeFrame timed out.\n");
    }
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
