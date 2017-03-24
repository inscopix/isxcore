#include "isxCore.h"
#include "isxDispatchQueue.h"
#include "isxIoQueue.h"
#include "isxVersion.h"
#include "isxException.h"
#include "isxLogger.h"
#include "isxReportUtils.h"

namespace isx
{
    isize_t getDataTypeSizeInBytes(DataType inDataType)
    {
        switch (inDataType)
        {
            case DataType::U8:
            {
                return sizeof(uint8_t);
            }
            case DataType::U16:
            {
                return sizeof(uint16_t);
            }
            case DataType::F32:
            {
                return sizeof(float);
            }
            default:
            {
                return 0;
            }
        }
    }

    std::ostream & operator<<(std::ostream & inStream, DataType inDataType)
    {
        switch (inDataType)
        {
            case DataType::U8:
            {
                inStream << "U8";
                break;
            }
            case DataType::U16:
            {
                inStream << "U16";
                break;
            }
            case DataType::F32:
            {
                inStream << "F32";
                break;
            }
            default:
            {
                inStream << "UNKNOWN";
                break;
            }
        }
        return inStream;
    }

    void CoreInitialize(const std::string & inLogFileName)
    {
        DispatchQueue::initializeDefaultQueues();
        IoQueue::initialize();        
        Logger::initialize(inLogFileName);
        reportSessionStart();
        reportSystemInfo();
    }
    bool CoreIsInitialized()
    {   
        return DispatchQueue::isInitialized()
            && IoQueue::isInitialized();
    }
    void CoreShutdown()
    {
        reportSessionEnd();
        IoQueue::destroy();
        DispatchQueue::destroyDefaultQueues();
    }

    int CoreVersionMajor()
    {
        return APP_VERSION_MAJOR;
    }
    int CoreVersionMinor()
    {
        return APP_VERSION_MINOR;
    }

    int CoreVersionPatch()
    {
        return APP_VERSION_PATCH;
    }

    std::vector<int>
    CoreVersionVector()
    {
        return { CoreVersionMajor() , CoreVersionMinor(), CoreVersionPatch() };
    }
}
