#ifndef ISX_NVISTA3_GPIO_FILE_H
#define ISX_NVISTA3_GPIO_FILE_H

#include "isxCore.h"
#include "isxAsync.h"
#include "isxTimingInfo.h"
#include "isxEventBasedFileV2.h"
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <memory>

namespace isx
{

/// A class that parses an nVoke GPIO file and separates the data packets by stream
///
class NVista3GpioFile
{
public:

    /// Constructs an invalid file object
    NVista3GpioFile();

    /// \param inFileName  the name of the file to read
    /// \param inOutputDir the directory that is going to contain the files for individual streams
    NVista3GpioFile(const std::string & inFileName, const std::string & inOutputDir);

    /// Destructor
    ///
    ~NVista3GpioFile();

    /// \return id this is a valid object
    ///
    bool isValid();

    /// \return the file name for the GPIO original file
    ///
    const std::string & getFileName();

    /// Set a check in callback for reporting progress
    void setCheckInCallback(AsyncCheckInCB_t inCheckInCB);

    /// Parses the original file and writes signals from different channels to separate files
    /// \throw isx::ExceptionDataIO  if unrecognized packets are read from the file
    /// \throw isx::ExceptionFileIO  if there is a problem reading or writing files
    /// \return whether the process completed or it was cancelled
    AsyncTaskStatus parse();

    /// Get a list of all the output files this object produces when parsing the original one
    const std::string & getOutputFileName() const;

private:

#pragma pack(push, 1)

    /// Each packet has a header that allows us to identify it.
    struct PktHeader
    {
        uint32_t m_type;            ///< Type of packet (e.g. command, response, event).
        uint32_t m_sequence;        ///< Incrementing number. May not be useful for us.
        uint32_t m_payloadSize;     ///< The size of the payload in 32-bit words.
    };

#pragma pack(pop)

    /// Possible types of events.
    enum class Event : uint32_t
    {
        CAPTURE_ALL = 0x4001,
        CAPTURE_GPIO, // 0x4002
        BNC_GPIO_IN_1, // 0x4003
        BNC_GPIO_IN_2, // 0x4004
        BNC_GPIO_IN_3, // 0x4005
        BNC_GPIO_IN_4, // 0x4006
        DIGITAL_GPI, // 0x4007
        EX_LED, // 0x4008
        OG_LED, // 0x4009
        DI_LED, // 0x400A
        FRAME_COUNT, // 0x400B
        BNC_TRIG, // 0x400C
        BNC_SYNC, // 0x400D
        WAVEFORM, // 0x400F
        MAX, // 0x4010
    };

    /// Possible channels to write.
    enum class Channel : uint32_t
    {
        FRAME_COUNTER,
        DIGITAL_GPI_0,
        DIGITAL_GPI_1,
        DIGITAL_GPI_2,
        DIGITAL_GPI_3,
        DIGITAL_GPI_4,
        DIGITAL_GPI_5,
        DIGITAL_GPI_6,
        DIGITAL_GPI_7,
        BNC_GPIO_IN_1,
        BNC_GPIO_IN_2,
        BNC_GPIO_IN_3,
        BNC_GPIO_IN_4,
        EX_LED,
        OG_LED,
        DI_LED,
        EFOCUS,
        BNC_TRIG,
        BNC_SYNC,
    };

    const static std::map<Channel, std::string> s_channelNames;

    const static std::map<Channel, SignalType> s_channelTypes;

    /// The signature sync word.
    const static uint32_t s_syncWord = 0x0000AA55;

    /// The signature event half-word.
    const static uint32_t s_eventSignature = 0x40;

    /// True if the movie file is valid, false otherwise.
    bool m_valid = false;

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

    /// Read a variable from the file.
    ///
    template <typename T>
    T read(T & outVar)
    {
        m_file.read(reinterpret_cast<char *>(&outVar), sizeof(outVar));
        return outVar;
    }

    void skipBytes(const size_t inNumBytes);
    void skipWords(const size_t inNumWords);

    std::vector<EventBasedFileV2::DataPkt> m_packets;
    std::map<Channel, uint64_t> m_indices;
    uint64_t m_firstTime = 0;

    void addPkt(const Channel inChannel, const uint64_t inTimeStamp, const float inValue);

}; // class NVista3GpioFile

} // namespace isx

#endif // ISX_NVISTA3_GPIO_FILE_H
