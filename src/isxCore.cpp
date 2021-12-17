#include "isxCore.h"
#include "isxDispatchQueue.h"
#include "isxIoQueue.h"
#include "isxException.h"
#include "isxLogger.h"
#include "isxReportUtils.h"
#include "isxPathUtils.h"

#include <QString>
#include <QStringList>

#include <algorithm>
#include <array>
#include <cmath>

extern "C"
{
#include "libavformat/avformat.h"
}

namespace
{
#ifdef NDEBUG
    void dummyAvLogFunction(void *, int, const char *, va_list)
    {
    }
#endif
} // namespace

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
    
    size_t
    calculateWidthOfPaddedName(const size_t inNumElems)
    {
        size_t width = 1;
        if (inNumElems > 10)
        {
            width = size_t(std::floor(std::log10(inNumElems - 1)) + 1);
        }
        return width;
    }

    std::string
    createNumberPaddedName(const std::string inPrefix, const size_t inNumber, const size_t inWidth)
    {
        return inPrefix + convertNumberToPaddedString(inNumber, inWidth);
    }

    void CoreInitialize(const std::string & inLogFileName)
    {
        DispatchQueue::initializeDefaultQueues();
        IoQueue::initialize();        
        Logger::initialize(inLogFileName);
        reportSessionStart();
        reportSystemInfo();

        av_register_all();

#ifdef NDEBUG
        av_log_set_callback(dummyAvLogFunction);
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
        return "Unknown";
#else
        return QSysInfo::machineHostName().toStdString();
#endif
    }

    std::vector<std::string>
    splitString(const std::string & inString, const char inDelim)
    {
        std::stringstream ss(inString);
        std::vector<std::string> tokens;
        while (ss.good())
        {
            std::string token;
            std::getline(ss, token, inDelim);
            tokens.push_back(token);
        }
        return tokens;
    }

    std::string
    trimString(const std::string & inString)
    {
        const auto firstNonSpace = inString.find_first_not_of(' ');
        const auto lastNonSpace = inString.find_last_not_of(' ');
        return inString.substr(firstNonSpace, (lastNonSpace - firstNonSpace) + 1);
    }

    std::ifstream &
    getLine(std::ifstream & inStream, std::string & outLine)
    {
        std::getline(inStream, outLine);
        outLine.erase(std::remove(outLine.begin(), outLine.end(), '\r'), outLine.end());
        outLine.erase(std::remove(outLine.begin(), outLine.end(), '\n'), outLine.end());
        return inStream;
    }

    void copyCppStringToCString(const std::string & inSource, char * inDest, size_t inDestCapacity)
    {
        if (!inSource.empty())
        {
            size_t numChars = std::min(inDestCapacity, inSource.size() + 1);
            std::string str = inSource;
            if (numChars != inSource.size() + 1)
            {
                /// string is truncated, make sure we have a null character
                str = inSource.substr(0, numChars-1);
            }
            snprintf(inDest, inDestCapacity, "%s", str.c_str());
        }
    }

    void closeFileStreamWithChecks(std::fstream & inFile, const std::string & inFileName)
    {
        if(inFile.is_open() && inFile.good())
        {
            inFile.close();
            if (!inFile.good())
            {
                ISX_LOG_ERROR("Error closing the stream for file", inFileName,
                " eof: ", inFile.eof(), 
                " bad: ", inFile.bad(), 
                " fail: ", inFile.fail());
            }
        }
    }

    std::string
    makeOutputFilePath(const std::string & inFilePath, const std::string & inSuffix)
    {
        return getDirName(inFilePath) + "/" + getBaseName(inFilePath) + inSuffix;
    }

    std::string
    toLower(const std::string & inStr)
    {
        return QString::fromStdString(inStr).toLower().toStdString();
    }

    bool versionAtLeast(
            const std::string & inVersion,
            const size_t inMajor,
            const size_t inMinor,
            const size_t inPatch)
    {
        // This implementation cannot use QVersionNumber right now, because it
        // was introduced in Qt 5.6, but the MATLAB API is built against Qt 5.5.
        const QString version = QString::fromStdString(inVersion);
        QStringList versionParts = version.split('-');
        if (versionParts.empty())
        {
            ISX_LOG_WARNING("Failed to parse version ", inVersion);
            return false;
        }

        versionParts = versionParts[0].split('.');
        if (versionParts.empty())
        {
            ISX_LOG_WARNING("Failed to parse version ", inVersion);
            return false;
        }

        std::array<size_t, 3> versionNumbers = {{0, 0, 0}};
        for (int i = 0; i < std::min(int(versionNumbers.size()), versionParts.size()); ++i)
        {
            bool ok = true;
            versionNumbers[i] = size_t(versionParts[i].toInt(&ok));
            if (!ok)
            {
                ISX_LOG_WARNING("Failed to parse version ", inVersion);
                return false;
            }
        }

        const size_t major = versionNumbers[0];
        if (major == inMajor)
        {
            const size_t minor = versionNumbers[1];
            if (minor == inMinor)
            {
                const size_t patch = versionNumbers[2];
                return patch >= inPatch;
            }
            else
            {
                return minor > inMinor;
            }
        }
        else
        {
            return major > inMajor;
        }

        return true;
    }

} // namespace isx
