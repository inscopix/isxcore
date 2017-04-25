#include "isxMosaicGpio.h"
#include "isxGpioFile.h"
#include "isxMutex.h"
#include "isxConditionVariable.h"
#include "isxIoTask.h"
#include "isxIoTaskTracker.h"

namespace isx
{

MosaicGpio::MosaicGpio()
    : m_file(new GpioFile())
{

}

MosaicGpio::MosaicGpio(const std::string & inFileName)
    : m_file(new GpioFile(inFileName))
    , m_analogIoTaskTracker(new IoTaskTracker<DTrace_t>())
    , m_logicalIoTaskTracker(new IoTaskTracker<LogicalTrace>())
{

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
MosaicGpio::isAnalog() const
{
    return m_file->isAnalog();
}

const std::string & 
MosaicGpio::getFileName() const
{
    return m_file->getFileName();
}

const isize_t 
MosaicGpio::numberOfChannels()
{
    return m_file->numberOfChannels();
}

const std::vector<std::string> 
MosaicGpio::getChannelList() const
{
    return m_file->getChannelList();
}

SpDTrace_t 
MosaicGpio::getAnalogData()
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getAnalogGpio");
    AsyncTaskResult<SpDTrace_t> asyncTaskResult;
    getAnalogDataAsync([&asyncTaskResult, &cv, &mutex](AsyncTaskResult<SpDTrace_t> inAsyncTaskResult)
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
MosaicGpio::getAnalogDataAsync(GpioGetAnalogDataCB_t inCallback)
{
    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<MosaicGpio> weakThis = shared_from_this();
    GetAnalogDataCB_t getAnalogCB = [weakThis, this]()
        {
            auto sharedThis = weakThis.lock();
            if (sharedThis)
            {
                return m_file->getAnalogData();
            }
            return SpDTrace_t();
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

const isx::TimingInfo & 
MosaicGpio::getTimingInfo() const
{
    return m_file->getTimingInfo();
}

}