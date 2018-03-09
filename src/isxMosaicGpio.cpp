#include "isxMosaicGpio.h"
#include "isxEventBasedFileV1.h"
#include "isxEventBasedFileV2.h"
#include "isxMutex.h"
#include "isxConditionVariable.h"
#include "isxIoTask.h"
#include "isxIoTaskTracker.h"

namespace isx
{

MosaicGpio::MosaicGpio()
    : m_file(new EventBasedFileV2())
{

}

MosaicGpio::MosaicGpio(const std::string & inFileName)
    : m_analogIoTaskTracker(new IoTaskTracker<FTrace_t>())
    , m_logicalIoTaskTracker(new IoTaskTracker<LogicalTrace>())
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
    }
}

MosaicGpio::~MosaicGpio()
{

}

bool 
MosaicGpio::isValid() const
{
    return m_file->isValid();
}


bool 
MosaicGpio::isAnalog(const std::string & inChannelName) const
{
    bool analog = false;
    if (m_type == FileType::V2)
    {
        auto f = std::static_pointer_cast<isx::EventBasedFileV2>(m_file);
        analog = f->getSignalType(inChannelName) == SignalType::DENSE;
    }
    else if (m_type == FileType::V1)
    {
        auto f = std::static_pointer_cast<isx::EventBasedFileV1>(m_file);
        analog = f->isAnalog();
    }
    return analog;
}

std::string
MosaicGpio::getFileName() const
{
    return m_file->getFileName();
}

isize_t 
MosaicGpio::numberOfChannels()
{
    return getChannelList().size();
}

const std::vector<std::string> 
MosaicGpio::getChannelList() const
{
    return m_file->getChannelList();
}

void 
MosaicGpio::getAllTraces(std::vector<SpFTrace_t> & outContinuousTraces, std::vector<SpLogicalTrace_t> & outLogicalTraces) 
{
    if (m_type == FileType::V2)
    {
        auto f = std::static_pointer_cast<isx::EventBasedFileV2>(m_file);
        f->readAllTraces(outContinuousTraces, outLogicalTraces);
    }
    else 
    {
        auto channels = getChannelList();
        auto f = std::static_pointer_cast<isx::EventBasedFileV1>(m_file);
        bool analog = f->isAnalog();

        if (analog)
        {
            for (auto & c : channels)
            {
                outContinuousTraces.emplace_back(f->getAnalogData(c));
                outLogicalTraces.emplace_back(f->getLogicalData(c));
            }
        }
        else
        {
            for (auto & c : channels)
            {
                outLogicalTraces.emplace_back(f->getLogicalData(c));
            }
        }
    }
    
}    


SpFTrace_t 
MosaicGpio::getAnalogData(const std::string & inChannelName)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getAnalogGpio");
    AsyncTaskResult<SpFTrace_t> asyncTaskResult;
    getAnalogDataAsync(inChannelName, [&asyncTaskResult, &cv, &mutex](AsyncTaskResult<SpFTrace_t> inAsyncTaskResult)
        {
            mutex.lock("getAnalogGpio async");
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
MosaicGpio::getAnalogDataAsync(const std::string & inChannelName, GpioGetAnalogDataCB_t inCallback)
{

    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<MosaicGpio> weakThis = shared_from_this();
    GetAnalogDataCB_t getAnalogCB = [weakThis, this, &inChannelName]()
        {
            auto sharedThis = weakThis.lock();
            if (sharedThis)
            {
                return m_file->getAnalogData(inChannelName);
            }
            return SpFTrace_t();
        };
    m_analogIoTaskTracker->schedule(getAnalogCB, inCallback); 
}

SpLogicalTrace_t 
MosaicGpio::getLogicalData(const std::string & inChannelName)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getLogicalGpio");
    AsyncTaskResult<SpLogicalTrace_t> asyncTaskResult;
    getLogicalDataAsync(inChannelName, [&asyncTaskResult, &cv, &mutex](AsyncTaskResult<SpLogicalTrace_t> inAsyncTaskResult)
        {
            mutex.lock("getLogicalGpio async");
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
MosaicGpio::getLogicalDataAsync(const std::string & inChannelName, GpioGetLogicalDataCB_t inCallback)
{
    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<MosaicGpio> weakThis = shared_from_this();
    GetLogicalDataCB_t getLogicalCB = [weakThis, this, &inChannelName]()
        {
            auto sharedThis = weakThis.lock();
            if (sharedThis)
            {
                return m_file->getLogicalData(inChannelName);
            }
            return SpLogicalTrace_t();
        };
    m_logicalIoTaskTracker->schedule(getLogicalCB, inCallback);
}

isx::TimingInfo 
MosaicGpio::getTimingInfo(const std::string & inChannelName) const
{
    if (m_type == FileType::V2)
    {
        auto f = std::static_pointer_cast<isx::EventBasedFileV2>(m_file);
        return f->getTimingInfo(inChannelName);
    }
    else 
    {
        auto f = std::static_pointer_cast<isx::EventBasedFileV1>(m_file);
        return f->getTimingInfo();
    }

}

isx::TimingInfos_t
MosaicGpio::getTimingInfosForSeries(const std::string & inChannelName) const
{
    return TimingInfos_t{getTimingInfo(inChannelName)};
}

isx::TimingInfo 
MosaicGpio::getTimingInfo() const 
{
    return m_file->getTimingInfo();
}
    
isx::TimingInfos_t
MosaicGpio::getTimingInfosForSeries() const 
{
    return TimingInfos_t{getTimingInfo()};
}

void
MosaicGpio::cancelPendingReads()
{
    m_analogIoTaskTracker->cancelPendingTasks();
    m_logicalIoTaskTracker->cancelPendingTasks();
}

}
