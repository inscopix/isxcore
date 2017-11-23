#include "isxCore.h"
#include "isxDispatchQueue.h"
#include "isxIoQueue.h"
#include "isxException.h"
#include "isxLogger.h"
#include "isxReportUtils.h"
#include <sstream>

#include <QSysInfo>
#include <QString>

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
            case DataType::RGB888:
            {
                return 3 * sizeof(uint8_t);
            }
            default:
            {
                return 0;
            }
        }
    }

    std::string getDataTypeString(DataType inDataType)
    {
        switch (inDataType)
        {
        case DataType::U8:
        {
            return std::string("uint8");
        }
        case DataType::U16:
        {
            return std::string("uint16");
        }
        case DataType::F32:
        {
            return std::string("float");
        }
        case DataType::RGB888:
        {
            return std::string("rgb888");
        }
        default:
        {
            return std::string("");
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
            case DataType::RGB888:
            {
                inStream << "RGB888";
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

    std::string
    convertNumberToPaddedString(const size_t inNumber, const size_t inWidth)
    {
        std::stringstream ss;
        ss.width(inWidth);
        ss.fill('0');
        ss << inNumber;
        return ss.str();
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
        return ISX_VERSION_MAJOR;
    }
    int CoreVersionMinor()
    {
        return ISX_VERSION_MINOR;
    }

    int CoreVersionPatch()
    {
        return ISX_VERSION_PATCH;
    }

    int CoreVersionBuild()
    {
        return ISX_VERSION_BUILD;
    }

    bool isBeta()
    {
        return ISX_IS_BETA;
    }

    std::string CoreVersionString()
    {
        std::ostringstream ss;
        ss << CoreVersionMajor() << "." << CoreVersionMinor() << "." << CoreVersionPatch() << "." << CoreVersionBuild();
        if (isBeta())
        {
            ss << " beta";
        }
        return ss.str();
    }

    std::vector<int>
    CoreVersionVector()
    {
        return { CoreVersionMajor() , CoreVersionMinor(), CoreVersionPatch() };
    }

    std::string
    getHostName()
    {
        return QSysInfo::machineHostName().toStdString();
    }
}
