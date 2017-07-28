#include "isxMosaicEvents.h"
#include "isxTimeStampedDataFile.h"
#include "isxMutex.h"
#include "isxConditionVariable.h"
#include "isxIoTask.h"
#include "isxIoTaskTracker.h"

namespace isx
{

MosaicEvents::MosaicEvents()
    : m_file(new TimeStampedDataFile())
{

}

MosaicEvents::MosaicEvents(const std::string & inFileName, bool inOpenForWrite)
    : m_logicalIoTaskTracker(new IoTaskTracker<LogicalTrace>())
{
    if (inOpenForWrite)
    {
        m_file.reset(new TimeStampedDataFile(inFileName, isx::TimeStampedDataFile::StoredData::EVENTS));
    }
    else
    {
        m_file.reset(new TimeStampedDataFile(inFileName));
    }
    
}

MosaicEvents::~MosaicEvents()
{

}

bool
MosaicEvents::isValid() const 
{
    return m_file->isValid();
}

const std::string &
MosaicEvents::getFileName() const
{
    return m_file->getFileName();
}

isize_t
MosaicEvents::numberOfCells() 
{
    return m_file->numberOfChannels();
}

const std::vector<std::string>
MosaicEvents::getCellNamesList() const 
{
    return m_file->getChannelList();
}

SpLogicalTrace_t
MosaicEvents::getLogicalData(const std::string & inCellName) 
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getLogicalEvents");
    AsyncTaskResult<SpLogicalTrace_t> asyncTaskResult;
    getLogicalDataAsync(inCellName, [&asyncTaskResult, &cv, &mutex](AsyncTaskResult<SpLogicalTrace_t> inAsyncTaskResult)
        {
            mutex.lock("getLogicalEvents async");
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
MosaicEvents::getLogicalDataAsync(const std::string & inCellName, EventsGetLogicalDataCB_t inCallback) 
{
    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<MosaicEvents> weakThis = shared_from_this();
    GetLogicalDataCB_t getLogicalCB = [weakThis, this, &inCellName]()
        {
            auto sharedThis = weakThis.lock();
            if (sharedThis)
            {
                return m_file->getLogicalData(inCellName);
            }
            return SpLogicalTrace_t();
        };
    m_logicalIoTaskTracker->schedule(getLogicalCB, inCallback);
}

const isx::TimingInfo &
MosaicEvents::getTimingInfo() const 
{
    return m_file->getTimingInfo();
}

isx::TimingInfos_t
MosaicEvents::getTimingInfosForSeries() const 
{
    return TimingInfos_t{m_file->getTimingInfo()};
}

void
MosaicEvents::cancelPendingReads() 
{
    m_logicalIoTaskTracker->cancelPendingTasks();
}

void 
MosaicEvents::setTimingInfo(const isx::TimingInfo & inTimingInfo) 
{
    m_file->setTimingInfo(inTimingInfo);
}

void 
MosaicEvents::writeCellHeader(
    const std::string & inCellName,
    const isx::isize_t inNumPackets) 
{
    m_file->writeChannelHeader(inCellName, "", "", inNumPackets);
}

void 
MosaicEvents::writeDataPkt(
    const uint64_t inTimeStampUSec,
    const float inValue) 
{
    TimeStampedDataFile::DataPkt pkt(inTimeStampUSec, true, inValue);
    m_file->writeDataPkt(pkt);
}

void 
MosaicEvents::closeForWriting() 
{
    m_file->closeFileForWriting();
}

} /// namespace isx