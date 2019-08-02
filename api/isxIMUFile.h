#ifndef ISX_IMUFILE_H
#define ISX_IMUFILE_H

#include <unordered_map>

#include "isxAsync.h"
#include "isxCore.h"
#include "isxTimingInfo.h"

namespace isx
{

/// A class that parses an .imu file and separates the data packets by stream
///
class IMUFile
{
public:

// Tighten structure
#pragma pack(push, 1)
    /// Structs
    struct IMUHeader {
        uint64_t epochTimeSecNum; ///<unix epoch time numerator
        uint64_t epochTimeSecDen; ///<unix epoch time denominator

        int32_t utcOffset;        ///<utc offset time
        uint16_t fileFormat;      ///<file writer software version marker
        uint16_t reserved;        ///<reserved

        uint64_t accCount;        ///<accelerometer data counter, true samples, no synthetic
        uint64_t accOffset;       ///<accelerometer data offset in byte units from beginning of file
        uint64_t accSize;         ///<accelerometer data size

        uint64_t magCount;        ///<magnetometer data counter, true samples, no synthetic
        uint64_t magOffset;       ///<magnetometer data offset in byte units from beginning of file
        uint64_t magSize;         ///<magnetometer data size

        uint64_t oriCount;        ///<orientation data counter, true samples, no synthetic
        uint64_t oriOffset;       ///<orientation data offset in byte units from beginning of file
        uint64_t oriSize;         ///<orientation data size

        uint64_t sessionOffset;   ///<session data offset
        uint64_t sessionSize;     ///<session data size (NOT USED HERE)
    }; // 112 bytes

    /// accelerometer structure data (in sync with orientation)
    struct AccPayload
    {
        uint64_t timeStamp;       ///<time stamp milliseconds
        int16_t accData[3];       ///<axis data (x, y ,z)
        int16_t reserved;         ///<64 bit packed
    }; // 16 bytes

    /// magnetometer structure data
    struct MagPayload
    {
        uint64_t timeStamp;       ///<time stamp milliseconds
        int16_t magData[3];       ///<axis data (x, y, z), two's complement
        int16_t reserved;         ///<64 bit packed
    }; // 16 bytes

    /// orientation structure data (in sync with accelerometer)
    struct OriPayload
    {
        uint64_t timeStamp;       ///<time stamp milliseconds
        int16_t oriData[3];       ///<axis data (yaw-pitch-roll) in radians, two's complement, S4.11
        int16_t reserved;         ///<temperature
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
    static constexpr uint8_t s_maxAccArrSize = 3; ///<max axis data elements for accelerometer, XYZ
    static constexpr uint8_t s_maxMagArrSize = 3; ///<max axis data elements for magnetometer, XYZ
    static constexpr uint8_t s_maxOriArrSize = 3; ///<max orientation data elements, yaw-pitch-roll
    static constexpr uint8_t s_accOriRate = 20; ///<step between acc/ori data packets in ms

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
