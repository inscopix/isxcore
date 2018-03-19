#include "isxMosaicEvents.h"
#include "isxEventBasedFileV1.h"
#include "isxEventBasedFileV2.h"
#include "isxMutex.h"
#include "isxConditionVariable.h"
#include "isxIoTask.h"
#include "isxIoTaskTracker.h"

namespace isx
{

MosaicEvents::MosaicEvents()
    : m_file(new EventBasedFileV2())
{

}

MosaicEvents::MosaicEvents(const std::string & inFileName, bool inOpenForWrite)
    : m_logicalIoTaskTracker(new IoTaskTracker<LogicalTrace>())
{
    if (inOpenForWrite)
    {
        m_type = FileType::V2;
        m_file.reset(new EventBasedFileV2(inFileName, isx::DataSet::Type::EVENTS, true));
    }
    else
    {
        m_type = isx::getFileType(inFileName);

        switch (m_type)
        {
            case FileType::V2:
            m_file.reset(new EventBasedFileV2(inFileName));
            break;

            case FileType::V1:
            m_file.reset(new EventBasedFileV1(inFileName));
            break;

            default:
                ISX_THROW(isx::ExceptionUserInput, "Invalid file name. The file is not a valid events file.");
        }        
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
    return m_file->getChannelList().size();
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

isx::TimingInfo 
MosaicEvents::getTimingInfo() const 
{
    return m_file->getTimingInfo();
}

isx::TimingInfos_t
MosaicEvents::getTimingInfosForSeries() const 
{
    return TimingInfos_t{getTimingInfo()};
}

void
MosaicEvents::cancelPendingReads() 
{
    m_logicalIoTaskTracker->cancelPendingTasks();
}

void 
MosaicEvents::setTimingInfo(const isx::TimingInfo & inTimingInfo) 
{
    if (m_type == FileType::V2)
    {
        auto f = std::static_pointer_cast<isx::EventBasedFileV2>(m_file);
        Time start = inTimingInfo.getStart();
        Time end = inTimingInfo.getEnd();
        std::vector<DurationInSeconds> steps(numberOfCells(), DurationInSeconds(0, 1));
        f->setTimingInfo(start, end, steps);
               
    }
    else if (m_type == FileType::V1)
    {
        auto f = std::static_pointer_cast<isx::EventBasedFileV1>(m_file);
        f->setTimingInfo(inTimingInfo);
    }    
}

void 
MosaicEvents::writeDataPkt(
    const uint64_t inSignalIdx,
    const uint64_t inTimeStampUSec,
    const float inValue) 
{
    EventBasedFileV2::DataPkt pkt(inTimeStampUSec, inValue, inSignalIdx);
    ISX_ASSERT(m_type == FileType::V2);
    auto f = std::static_pointer_cast<isx::EventBasedFileV2>(m_file);
    f->writeDataPkt(pkt);  
}

void 
MosaicEvents::closeForWriting(const std::vector<std::string> & inNewChannelNames) 
{
    ISX_ASSERT(m_type == FileType::V2);
    auto f = std::static_pointer_cast<isx::EventBasedFileV2>(m_file);
    f->setChannelList(inNewChannelNames);
    f->closeFileForWriting();
}

bool 
MosaicEvents::hasMetrics() const 
{
    if (m_type == FileType::V2)
    {
        auto f = std::static_pointer_cast<isx::EventBasedFileV2>(m_file);
        return f->hasMetrics();
    }
    
    return false;
}

SpTraceMetrics_t 
MosaicEvents::getTraceMetrics(isize_t inIndex) const 
{
    if (m_type == FileType::V2)
    {
        auto f = std::static_pointer_cast<isx::EventBasedFileV2>(m_file);
        return f->getTraceMetrics(inIndex);
    }
    else
    {
        return nullptr;
    }
}

void
MosaicEvents::setTraceMetrics(isize_t inIndex, const SpTraceMetrics_t & inMetrics) 
{
    ISX_ASSERT(m_type == FileType::V2);
    auto f = std::static_pointer_cast<isx::EventBasedFileV2>(m_file);
    f->setTraceMetrics(inIndex, inMetrics);
}

} /// namespace isx