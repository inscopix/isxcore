#ifndef ISX_IMUFILE_H
#define ISX_IMUFILE_H

#include <unordered_map>

#include "isxAsync.h"
#include "isxCore.h"
#include "isxTimingInfo.h"

namespace isx
{

class IMUFile
{
public:

// Tighten structure
#pragma pack(push, 1)
    /// Structs
    struct IMUHeader {
        uint64_t epochTimeSecNum;
        uint64_t epochTimeSecDen;

        int32_t utcOffset;
        uint16_t fileFormat;
        uint16_t reserved;

        uint64_t accCount;
        uint64_t accOffset;
        uint64_t accSize;

        uint64_t magCount;
        uint64_t magOffset;
        uint64_t magSize;

        uint64_t oriCount;
        uint64_t oriOffset;
        uint64_t oriSize;

        uint64_t sessionOffset;
        uint64_t sessionSize; // Can be used to verify session integrity
    }; // 112 bytes

    struct AccPayload
    {
        uint64_t timeStamp;
        int16_t accData[3];
        int16_t reserved;
    }; // 16 bytes

    struct MagPayload
    {
        uint64_t timeStamp;
        int16_t magData[3];
        int16_t reserved;
    }; // 16 bytes

    struct OriPayload
    {
        uint64_t timeStamp;
        int16_t oriData[3];
        int16_t reserved;
    }; // 16 bytes
#pragma pack(pop)

    /// enum
    /// Signals
    enum IMUSignalType
    {
        ACC = 0,
        MAG,
        ORI
    };

    /// Functions
    /// Constructs an invalid file object
    IMUFile() = default;

    /// \param inFileName  the name of the file to read
    /// \param inOutputDir the directory that is going to contain the files for individual streams
    IMUFile(const std::string & inFileName, const std::string & inOutputDir);

    /// Destructor
    ///
    ~IMUFile();

    /// \return the file name for the GPIO original file
    ///
    const std::string & getFileName();

    /// Get output file this object produces when parsing the original one
    const std::string & getOutputFileName() const;

    /// Set a check in callback for reporting progress
    void setCheckInCallback(AsyncCheckInCB_t inCheckInCB);

    /// Parses the original file and writes signals from different channels to separate files
    /// \throw isx::ExceptionDataIO  if unrecognized packets are read from the file
    /// \throw isx::ExceptionFileIO  if there is a problem reading or writing files
    /// \return whether the process completed or it was cancelled
    AsyncTaskStatus parse();

    /// Constants
    static constexpr uint8_t s_dataTypes = 3;
    static constexpr uint8_t s_maxAccArrSize = 3;
    static constexpr uint8_t s_maxMagArrSize = 3;
    static constexpr uint8_t s_maxOriArrSize = 3;
    static constexpr uint8_t s_channelNum = s_maxAccArrSize + s_maxMagArrSize + s_maxOriArrSize;
    static constexpr uint8_t s_accOriRate = 20; /// 20 ms (or 50 Hz)

private:
    /// The name of the data file.
    std::string m_fileName;

    /// The directory where output files will be written.
    std::string m_outputDir;

    /// The output file name.
    std::string m_outputFileName;

    /// The file stream
    std::fstream m_file;

    /// Check in callback for reporting progress
    AsyncCheckInCB_t m_checkInCB;

    /// The session string
    std::string m_sessionStr;

    /// The output/display channels
    std::vector<std::string> m_channels;

};

} // def namespace


#endif //ISX_IMUFILE_H
