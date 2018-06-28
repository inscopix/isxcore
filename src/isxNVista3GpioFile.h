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

/// Thrown when a GPIO packet read from a file is bad.
///
class BadGpioPacket : public Exception
{
public:

    /// Constructor
    ///
    /// \param  file    The name of the file from where the exception is thrown.
    /// \param  line    The line in the file from where the exception is thrown.
    /// \param  message The error message.
    explicit BadGpioPacket(const std::string & file, int line, const std::string & message);

    /// Destructor
    ///
    ~BadGpioPacket() override;
};

/// A class that parses an nVoke GPIO file and separates the data packets by stream
///
class NVista3GpioFile
{
public:

#pragma pack(push, 1)

    /// Each packet has a header that allows us to identify it.
    struct PktHeader
    {
        uint32_t type;            ///< Type of packet (e.g. command, response, event).
        uint32_t sequence;        ///< Incrementing number. May not be useful for us.
        uint32_t payloadSize;     ///< The size of the payload in 32-bit words.
    };

    /// These are the specific payloads. See internal documentation for more details.
    /// TODO : These are like defined in other headers used by the Hub. It would good
    /// to use the same source for these, but copying due to time pressure.

    struct CountPayload
    {
        uint32_t tscHigh;
        uint32_t tscLow;
        uint32_t fc;
    };

    struct AllPayload
    {
        CountPayload count;
        uint32_t digitalGpi; // 8-bit usable
        uint16_t bncGpio1;
        uint16_t bncGpio2;
        uint16_t bncGpio3;
        uint16_t bncGpio4;
        uint16_t exLed;
        uint16_t ogLed;
        uint16_t diLed;
        uint16_t eFocus; // 14-bits usable
        uint32_t trigSyncFlash; // 3-bits usable
    };

    struct AllGpioPayload
    {
        CountPayload count;
        uint16_t digitalGpi; // 8-bits usable
        uint16_t bncTrig; // 1-bit usable
        uint16_t bncGpio1;
        uint16_t bncGpio2;
        uint16_t bncGpio3;
        uint16_t bncGpio4;
    };

    struct GpioPayload
    {
        CountPayload count;
        uint32_t bncGpio; // 16-bits usable
    };

    struct DigitalGpiPayload
    {
        CountPayload count;
        uint32_t digitalGpi; // 16-bits usable
    };

    struct LedPayload
    {
        CountPayload count;
        uint32_t led; // 16-bits usable
    };

    struct TrigPayload
    {
        CountPayload count;
        uint32_t bncTrig; // 1-bit usable
    };

    struct SyncPayload
    {
        CountPayload count;
        uint32_t bncSync; // 1-bit usable
    };

    struct WaveformPayload
    {
        uint32_t bufferNum; // 16-bits usable
        uint32_t count;
    };

    struct AdpDumpHeader
    {
        uint64_t secsSinceEpochNum;
        uint64_t secsSinceEpochDen;
        int32_t utcOffset;
        uint32_t eventDataOffset;
        uint64_t eventCount;
    };

#pragma pack(pop)

    /// Possible types of events.
    enum class Event : uint32_t
    {
        CAPTURE_ALL = 0x4001,
        CAPTURE_GPIO, // 0x4002
        BNC_GPIO_1, // 0x4003
        BNC_GPIO_2, // 0x4004
        BNC_GPIO_3, // 0x4005
        BNC_GPIO_4, // 0x4006
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

    /// The signature sync word.
    const static uint32_t s_syncWord = 0x0000AA55;

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
        BNC_GPIO_1,
        BNC_GPIO_2,
        BNC_GPIO_3,
        BNC_GPIO_4,
        EX_LED,
        OG_LED,
        DI_LED,
        EFOCUS,
        TRIG,
        SYNC,
        FLASH,
        BNC_TRIG,
        BNC_SYNC,
    };

    const static std::map<Channel, std::string> s_channelNames;

    const static std::map<Channel, SignalType> s_channelTypes;

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

    /// Read a value of arbitrary size from the file.
    template <typename T>
    T read()
    {
        T output;
        m_file.read(reinterpret_cast<char *>(&output), sizeof(output));
        return output;
    }

    /// Read a value from the file, with a check for the number of words.
    template <typename T>
    T read(const uint32_t inNumWords)
    {
        const size_t actualSize = sizeof(T);
        const size_t expectedSize = size_t(inNumWords * sizeof(uint32_t));
        if (actualSize != expectedSize)
        {
            ISX_THROW(BadGpioPacket, "Expected to read ", expectedSize, " bytes, ",
                    "but actual payload is ", actualSize, " bytes.");
        }
        return read<T>();
    }

    void skipBytes(const size_t inNumBytes);
    void skipWords(const size_t inNumWords);

    std::vector<EventBasedFileV2::DataPkt> m_packets;
    std::map<Channel, uint64_t> m_indices;
    std::map<Channel, float> m_lastValues;

    /// Add a packet to the output file.
    void addPkt(const Channel inChannel, const uint64_t inTimeStamp, const float inValue);

    /// Add digital GPI packets based on the packed payload value to the output file.
    void addDigitalGpiPkts(const uint64_t inTsc, const uint16_t inDigitalGpi);

    /// Add sensor TRIG, SYNC, FLASH packets based on the packed payload value to the output file.
    void addTrigSyncFlashPkts(const uint64_t inTsc, const uint16_t inTrigSyncFlash);

    /// Add all four GPIO packets to the output file.
    template <typename T>
    void addGpioPkts(const uint64_t inTsc, const T inPayload)
    {
        addPkt(Channel::BNC_GPIO_1, inTsc, roundGpioValue(inPayload.bncGpio1));
        addPkt(Channel::BNC_GPIO_2, inTsc, roundGpioValue(inPayload.bncGpio2));
        addPkt(Channel::BNC_GPIO_3, inTsc, roundGpioValue(inPayload.bncGpio3));
        addPkt(Channel::BNC_GPIO_4, inTsc, roundGpioValue(inPayload.bncGpio4));
    }

    /// Read and parse a single GPIO payload, then add corresponding packet to the output file.
    void readParseAddGpioPayload(const uint32_t inExpectedSize, const Channel inChannel);

    /// Read and parse a single LED payload, then add corresponding packet to the output file.
    void readParseAddLedPayload(const uint32_t inExpectedSize, const Channel inChannel);

    /// Parse the high and low components of the TSC.
    static uint64_t parseTsc(const CountPayload & inCount);

    /// Read and parse a packet payload based on its header.
    /// \throw  BadGpioPacket   If the payload size indicated in the header does
    ///                         not match the expected size of the payload.
    void readParseAddPayload(const PktHeader & inHeader);

    /// Round a BNC GPIO value to prevent detecting changes due to noise alone.
    ///
    template<typename T>
    static float roundGpioValue(const T inValue)
    {
        const T precision = 16;
        return float(precision * (inValue / precision));
    }

}; // class NVista3GpioFile

} // namespace isx

#endif // ISX_NVISTA3_GPIO_FILE_H
