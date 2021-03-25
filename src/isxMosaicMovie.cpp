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

MosaicMovie::MosaicMovie(const std::string & inFileName, bool enableWrite)
    : m_valid(false)
    , m_ioTaskTracker(new IoTaskTracker<VideoFrame>())
{
    m_file = std::make_shared<MosaicMovieFile>(inFileName, enableWrite);
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

std::vector<uint16_t>
MosaicMovie::getFrameHeader(const size_t inFrameNumber)
{
    return m_file->readFrameHeader(inFrameNumber);
}

std::vector<uint16_t>
MosaicMovie::getFrameFooter(const size_t inFrameNumber)
{
    return m_file->readFrameFooter(inFrameNumber);
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
    writeAndWait([file, inVideoFrame]()
    {
        file->writeFrame(inVideoFrame);
    }, "writeFrame");
}

void
MosaicMovie::writeFrameWithHeaderFooter(const uint16_t * inBuffer)
{
    // Get a new shared pointer to the file, so we can guarantee the write.
    std::shared_ptr<MosaicMovieFile> file = m_file;
    writeAndWait([file, inBuffer]()
    {
        file->writeFrameWithHeaderFooter(inBuffer);
    }, "writeFrameWithHeaderFooterTogether");
}

void
MosaicMovie::writeFrameWithHeaderFooter(const uint16_t * inHeader, const uint16_t * inPixels, const uint16_t * inFooter)
{
    // Get a new shared pointer to the file, so we can guarantee the write.
    std::shared_ptr<MosaicMovieFile> file = m_file;
    writeAndWait([file, inHeader, inPixels, inFooter]()
    {
        file->writeFrameWithHeaderFooter(inHeader, inPixels, inFooter);
    }, "writeFrameWithHeaderFooterSeparate");
}

void
MosaicMovie::closeForWriting(const TimingInfo & inTimingInfo)
{
    m_file->closeForWriting(inTimingInfo);
}

SpVideoFrame_t
MosaicMovie::makeVideoFrame(isize_t inIndex)
{
    return m_file->makeVideoFrame(inIndex);
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

SpacingInfo
MosaicMovie::getOriginalSpacingInfo() const
{
    return m_file->getOriginalSpacingInfo();
}

void
MosaicMovie::setIntegratedBasePlate(const std::string & inIntegratedBasePlate)
{
    m_file->setIntegratedBasePlate(inIntegratedBasePlate);
}

void
MosaicMovie::writeAndWait(std::function<void()> inCallback, const std::string & inName)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock(inName);
    auto writeIoTask = std::make_shared<IoTask>(
        inCallback,
        [&cv, &mutex, &inName](AsyncTaskStatus inStatus)
        {
            if (inStatus != AsyncTaskStatus::COMPLETE)
            {
                ISX_LOG_ERROR("An error occurred while writing data to MosaicMovieFile.");
            }
            mutex.lock(inName + " finished");  // will only be able to take lock when client reaches cv.waitForMs
            mutex.unlock();
            cv.notifyOne();
        });
    writeIoTask->schedule();
    cv.wait(mutex);
    mutex.unlock();

    if (writeIoTask->getTaskStatus() == AsyncTaskStatus::ERROR_EXCEPTION)
    {
        std::rethrow_exception(writeIoTask->getExceptionPtr());
    }
}

bool
MosaicMovie::hasFrameTimestamps() const
{
    return m_file->hasFrameTimestamps();
}

uint64_t
MosaicMovie::getFrameTimestamp(const isize_t inIndex)
{
    return m_file->readFrameTimestamp(inIndex);
}

void
MosaicMovie::closeFileStream()
{
    return m_file->closeFileStream();
}

} // namespace isx
