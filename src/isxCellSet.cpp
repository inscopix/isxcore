#include "isxCellSet.h"
#include "isxCellSetFile.h"
#include "isxConditionVariable.h"
#include "isxIoTask.h"

namespace isx
{

CellSet::CellSet()
{
}

CellSet::CellSet(const std::string & inFileName)
{
    m_file = std::make_shared<CellSetFile>(inFileName);
    m_valid = true;
}

CellSet::CellSet(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo)
{
    m_file = std::make_shared<CellSetFile>(inFileName, inTimingInfo, inSpacingInfo);
    m_valid = true;
}

CellSet::~CellSet()
{
}

bool
CellSet::isValid() const
{
    return m_valid;
}

std::string
CellSet::getFileName() const
{
    return m_file->getFileName();
}

const isize_t
CellSet::getNumCells()
{
    return m_file->numberOfCells();
}

isx::TimingInfo
CellSet::getTimingInfo() const
{
    return m_file->getTimingInfo();
}

isx::SpacingInfo
CellSet::getSpacingInfo() const
{
    return m_file->getSpacingInfo();
}

SpFTrace_t
CellSet::getTrace(isize_t inIndex)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getTrace");
    SpFTrace_t outTrace;
    getTraceAsync(inIndex,
        [&outTrace, &cv, &mutex](const SpFTrace_t & inTrace)
        {
            mutex.lock("getTrace async");
            outTrace = inTrace;
            mutex.unlock();
            cv.notifyOne();
        }
    );
    cv.wait(mutex);
    mutex.unlock();
    return outTrace;
}

void
CellSet::getTraceAsync(isize_t inIndex, GetTraceCB_t inCallback)
{
    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<CellSet> weakThis = shared_from_this();

    uint64_t readRequestId = 0;
    {
        ScopedMutex locker(m_pendingReadsMutex, "getTrace");
        readRequestId = m_readRequestCount++;
    }

    auto readIoTask = std::make_shared<IoTask>(
        [weakThis, this, inIndex, inCallback]()
        {
            auto sharedThis = weakThis.lock();
            if (sharedThis)
            {
                inCallback(m_file->readTrace(inIndex));
            }
        },
        [weakThis, this, readRequestId, inCallback](AsyncTaskStatus inStatus)
        {
            auto sharedThis = weakThis.lock();
            if (!sharedThis)
            {
                return;
            }

            auto rt = unregisterReadRequest(readRequestId);

            checkAsyncTaskStatus(rt, inStatus, "CellSet::getTraceAsync");
            if (inStatus == AsyncTaskStatus::ERROR_EXCEPTION)
            {
                inCallback(SpFTrace_t());
            }
        }
    );

    {
        ScopedMutex locker(m_pendingReadsMutex, "getTrace");
        m_pendingReads[readRequestId] = readIoTask;
    }
    readIoTask->schedule();
}

SpImage_t
CellSet::getImage(isize_t inIndex)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getImage");
    SpImage_t outImage;
    getImageAsync(inIndex,
        [&outImage, &cv, &mutex](const SpImage_t & inImage)
        {
            mutex.lock("getImage async");
            outImage = inImage;
            mutex.unlock();
            cv.notifyOne();
        }
    );
    cv.wait(mutex);
    mutex.unlock();
    return outImage;
}

void
CellSet::getImageAsync(isize_t inIndex, GetImageCB_t inCallback)
{
    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<CellSet> weakThis = shared_from_this();

    uint64_t readRequestId = 0;
    {
        ScopedMutex locker(m_pendingReadsMutex, "getImage");
        readRequestId = m_readRequestCount++;
    }

    auto readIoTask = std::make_shared<IoTask>(
        [weakThis, this, inIndex, inCallback]()
        {
            auto sharedThis = weakThis.lock();
            if (sharedThis)
            {
                inCallback(m_file->readSegmentationImage(inIndex));
            }
        },
        [weakThis, this, readRequestId, inCallback](AsyncTaskStatus inStatus)
        {
            auto sharedThis = weakThis.lock();
            if (!sharedThis)
            {
                return;
            }

            auto rt = unregisterReadRequest(readRequestId);

            checkAsyncTaskStatus(rt, inStatus, "CellSet::getImageAsync");
            if (inStatus == AsyncTaskStatus::ERROR_EXCEPTION)
            {
                inCallback(SpImage_t());
            }
        }
    );

    {
        ScopedMutex locker(m_pendingReadsMutex, "getImage");
        m_pendingReads[readRequestId] = readIoTask;
    }
    readIoTask->schedule();
}

void
CellSet::writeImageAndTrace(
        isize_t inIndex,
        SpImage_t & inImage,
        SpFTrace_t & inTrace)
{
    // Get a new shared pointer to the file, so we can guarantee the write.
    std::shared_ptr<CellSetFile> file = m_file;
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("CellSet::writeImageAndTrace");
    auto writeIoTask = std::make_shared<IoTask>(
        [file, inIndex, inImage, inTrace]()
        {
            file->writeCellData(inIndex, *inImage, *inTrace);
        },
        [&cv, &mutex](AsyncTaskStatus inStatus)
        {
            if (inStatus != AsyncTaskStatus::COMPLETE)
            {
                ISX_LOG_ERROR("An error occurred while writing image and trace data to a CellSet.");
            }
            // will only be able to take lock when client reaches cv.wait
            mutex.lock("CellSetwriteImageAndTrace finished");
            mutex.unlock();
            cv.notifyOne();
        });
    writeIoTask->schedule();
    cv.wait(mutex);
    mutex.unlock();
}

bool
CellSet::isCellValid(isize_t inIndex)
{
    return m_file->isCellValid(inIndex);
}

void
CellSet::setCellValid(isize_t inIndex, bool inIsValid)
{
    return m_file->setCellValid(inIndex, inIsValid);
}

void
CellSet::cancelPendingReads()
{
    ScopedMutex locker(m_pendingReadsMutex, "cancelPendingReads");
    for (auto & pr: m_pendingReads)
    {
        pr.second->cancel();
    }
    m_pendingReads.clear();
}

SpAsyncTaskHandle_t
CellSet::unregisterReadRequest(uint64_t inReadRequestId)
{
    ScopedMutex locker(m_pendingReadsMutex, "unregisterPendingRead");
    auto ret = m_pendingReads[inReadRequestId];
    m_pendingReads.erase(inReadRequestId);
    return ret;
}

}
