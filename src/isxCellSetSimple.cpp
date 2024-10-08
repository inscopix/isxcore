#include "isxCellSetSimple.h"
#include "isxCellSetFile.h"
#include "isxConditionVariable.h"
#include "isxIoTask.h"
#include "isxIoTaskTracker.h"

namespace isx
{

CellSetSimple::CellSetSimple()
{
}

CellSetSimple::CellSetSimple(const std::string & inFileName, bool enableWrite)
    : m_valid(false)
    , m_traceIoTaskTracker(new IoTaskTracker<FTrace_t>())
    , m_imageIoTaskTracker(new IoTaskTracker<Image>())
{
    m_file = std::make_shared<CellSetFile>(inFileName, enableWrite);
    m_valid = true;
}

CellSetSimple::CellSetSimple(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo,
        const bool inIsRoiSet)
    : m_valid(false)
    , m_traceIoTaskTracker(new IoTaskTracker<FTrace_t>())
    , m_imageIoTaskTracker(new IoTaskTracker<Image>())
{
    m_file = std::make_shared<CellSetFile>(inFileName, inTimingInfo, inSpacingInfo, inIsRoiSet);
    m_valid = true;
}

CellSetSimple::~CellSetSimple()
{
}

bool
CellSetSimple::isValid() const
{
    return m_valid;
}

void
CellSetSimple::closeForWriting()
{
    m_file->closeForWriting();
}

std::string
CellSetSimple::getFileName() const
{
    return m_file->getFileName();
}

const isize_t
CellSetSimple::getNumCells()
{
    return m_file->numberOfCells();
}

isx::TimingInfo
CellSetSimple::getTimingInfo() const
{
    return m_file->getTimingInfo();
}

isx::TimingInfos_t
CellSetSimple::getTimingInfosForSeries() const
{
    TimingInfos_t tis = TimingInfos_t{m_file->getTimingInfo()};
    return tis;
}

isx::SpacingInfo
CellSetSimple::getSpacingInfo() const
{
    return m_file->getSpacingInfo();
}

SpFTrace_t
CellSetSimple::getTrace(isize_t inIndex)
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
CellSetSimple::getTraceAsync(isize_t inIndex, CellSetGetTraceCB_t inCallback)
{
    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<CellSetSimple> weakThis = shared_from_this();
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
CellSetSimple::getImage(isize_t inIndex)
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
CellSetSimple::getImageAsync(isize_t inIndex, CellSetGetImageCB_t inCallback)
{
    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<CellSetSimple> weakThis = shared_from_this();
    GetImageCB_t getImageCB = [weakThis, this, inIndex]()
        {
            auto sharedThis = weakThis.lock();
            SpImage_t im;
            if (sharedThis)
            {
                im = m_file->readSegmentationImage(inIndex);
            }
            return im;
        };
    m_imageIoTaskTracker->schedule(getImageCB, inCallback);
}

void
CellSetSimple::writeImageAndTrace(
        isize_t inIndex,
        const SpImage_t & inImage,
        SpFTrace_t & inTrace,
        const std::string & inName)
{
    // Get a new shared pointer to the file, so we can guarantee the write.
    std::shared_ptr<CellSetFile> file = m_file;
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("CellSetSimple::writeImageAndTrace");
    auto writeIoTask = std::make_shared<IoTask>(
        [file, inIndex, inImage, inTrace, inName]()
        {
            file->writeCellData(inIndex, *inImage, *inTrace, inName);
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

    if(writeIoTask->getTaskStatus() == AsyncTaskStatus::ERROR_EXCEPTION)
    {
        std::rethrow_exception(writeIoTask->getExceptionPtr());
    }
}

CellSet::CellStatus
CellSetSimple::getCellStatus(isize_t inIndex)
{
    return m_file->getCellStatus(inIndex);
}

Color
CellSetSimple::getCellColor(isize_t inIndex)
{
    return m_file->getCellColor(inIndex);
}

std::string
CellSetSimple::getCellStatusString(isize_t inIndex)
{
    return m_file->getCellStatusString(inIndex);
}

void
CellSetSimple::setCellStatus(isize_t inIndex, CellSet::CellStatus inStatus)
{
    m_file->setCellStatus(inIndex, inStatus);
}

void
CellSetSimple::setCellColor(isize_t inIndex, const Color& inColor)
{
    m_file->setCellColor(inIndex, inColor);
}

void
CellSetSimple::setCellColors(const IdColorPairs &inColors)
{
    m_file->setCellColors(inColors);
}

std::string
CellSetSimple::getCellName(isize_t inIndex)
{
    return m_file->getCellName(inIndex);
}

void
CellSetSimple::setCellName(isize_t inIndex, const std::string & inName)
{
    m_file->setCellName(inIndex, inName);
}

std::vector<bool>
CellSetSimple::getCellActivity(isize_t inIndex) const
{
    return {m_file->isCellActive(inIndex)};
}

void
CellSetSimple::setCellActive(isize_t inIndex, const std::vector<bool> & inActive)
{
    ISX_ASSERT(inActive.size() == 1);
    m_file->setCellActive(inIndex, inActive.front());
}

void
CellSetSimple::cancelPendingReads()
{
    m_imageIoTaskTracker->cancelPendingTasks();
    m_traceIoTaskTracker->cancelPendingTasks();
}

bool
CellSetSimple::isRoiSet() const
{
    return m_file->isRoiSet();
}

isize_t
CellSetSimple::getSizeGlobalCS()
{
    return m_file->getSizeGlobalCS();
}

void
CellSetSimple::setSizeGlobalCS(const isize_t inSizeGlobalCS)
{
    m_file->setSizeGlobalCS(inSizeGlobalCS);
}

std::vector<int16_t>
CellSetSimple::getMatches()
{
    return m_file->getMatches();
}

void
CellSetSimple::setMatches(const std::vector<int16_t> & inMatches)
{
    m_file->setMatches(inMatches);
}

std::vector<uint16_t>
CellSetSimple::getEfocusValues ()
{
    return m_file->getEfocusValues();
}

void
CellSetSimple::setEfocusValues (const std::vector<uint16_t> &inEfocus)
{
    m_file->setEfocusValues(inEfocus);
};

std::vector<double>
CellSetSimple::getPairScores()
{
    return m_file->getPairScores();
}

void
CellSetSimple::setPairScores(const std::vector<double> & inPairScores)
{
    m_file->setPairScores(inPairScores);
}

std::vector<double>
CellSetSimple::getCentroidDistances()
{
    return m_file->getCentroidDistances();
}

void
CellSetSimple::setCentroidDistances(const std::vector<double> & inCentroidDistances)
{
    m_file->setCentroidDistances(inCentroidDistances);
}

bool 
CellSetSimple::hasMetrics() const 
{
    return m_file->hasMetrics();
}

SpImageMetrics_t 
CellSetSimple::getImageMetrics(isize_t inIndex) const 
{
    return m_file->getImageMetrics(inIndex);
}

void
CellSetSimple::setImageMetrics(isize_t inIndex, const SpImageMetrics_t & inMetrics)
{
    m_file->setImageMetrics(inIndex, inMetrics);
}

std::string
CellSetSimple::getExtraProperties() const
{
    return m_file->getExtraProperties();
}

void
CellSetSimple::setExtraProperties(const std::string & inProperties)
{
    m_file->setExtraProperties(inProperties);
}

SpacingInfo
CellSetSimple::getOriginalSpacingInfo() const
{
    return m_file->getOriginalSpacingInfo();
}

} // namespace isx

