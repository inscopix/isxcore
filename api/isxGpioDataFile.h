#ifndef ISX_GPIO_DATA_FILE_H
#define ISX_GPIO_DATA_FILE_H
#include "isxCore.h"
#include "isxAsync.h"
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <memory>


namespace isx
{
    /// Definitions used to read GPIO raw file that 
    /// nVoke outputs
    const isize_t SYNC_PKT_LENGTH           = 7;
    const isize_t GPIO_PKT_LENGTH           = 11;
    const isize_t LED_PKT_LENGTH            = 13;
    const isize_t ANALOG_FOLLOW_PKT_LENGTH  = 32;
    const isize_t MAX_PACKET_SIZE           = ANALOG_FOLLOW_PKT_LENGTH;

    #pragma pack(push, 1)
    /// Sync packets are sent every 2 seconds and recorded within the data
    /// They are used to find the beginning of the data stream
    struct SyncPkt
    {
        uint32_t syncVal0;      ///< The sync packet is 7 bytes long, this was a reasonable representation of it
        uint16_t syncVal1;      ///< Same
        uint8_t  syncVal2;      ///< Same

        /// Returns the values of a valid sync packet
        static SyncPkt syncValues()
        {
            SyncPkt pkt;
            pkt.syncVal0 = 0x55555555;
            pkt.syncVal1 = 0x5555;
            pkt.syncVal2 = 0x5D;
            return pkt;
        }

        /// \return if the objects are equal
        /// \param other the other object to compare
        bool
        operator ==(const SyncPkt & other) const
        {
            return ((syncVal0 == other.syncVal0) &&
                (syncVal1 == other.syncVal1) &&
                (syncVal2 == other.syncVal2));
        }


    };  

    /// Generic header used to differentiate GPIO, LED and Analog Follow packets
    ///
    struct GenericPktHeader
    {
        uint8_t dataType;                      ///< The data type code
        uint8_t pktLength;                     ///< The length of the packet in bytes
        uint8_t eventCounter;                  ///< The event counter used to check if any packets were lost

    }; 

    const uint8_t GPIO_STATE_MASK      = 0x80;
    const uint8_t GPIO_MODE_MASK       = 0x03;

    /// The GPIO data packet
    ///
    struct GpioPkt
    {
        GenericPktHeader header;            ///< The packet header
        uint8_t stateMode;            ///< Byte encoding both GPIO state and GPIO mode
        uint8_t timeSecs[4];          ///< Time from Unix epoch in secs
        uint8_t timeUSecs[3];         ///< Micro secs portion of the time stamp
    };

    const uint8_t LED_POWER_0_MASK     = 0x10;
    const uint8_t LED_STATE_MASK       = 0x03;
    const uint8_t LED_MODE_MASK        = 0x07;
    const uint8_t LED_GPIO_FOLLOW_MASK = 0x30;

    /// The LED data packet
    ///
    struct LedPkt
    {
        GenericPktHeader header;            ///< The packet header
        uint8_t ledPower;             ///< LED power bits 1:8
        uint8_t powerState;           ///< LED power bit 0 and LED state (on/off)
        uint8_t followMode;           ///< GPIO follow channel and acquisition mode
        uint8_t timeSecs[4];          ///< Time from Unix epoch in secs
        uint8_t timeUSecs[3];         ///< Micro secs portion of the time stamp
    };

    const uint8_t AF_POWER_LAST        = 0x80;
    const uint8_t AF_POWER_FIRST       = 0x7F;


    /// The Analog Follow data packet
    struct AnalogFollowPkt
    {
        GenericPktHeader header;            ///< The packet header
        uint8_t ogMaxPower[2];        ///< OG LED max power bits 7, OG LED max power bits 0:6
        uint8_t analogSamples[20];    ///< 10 samples in each packet, 2 bytes for each
        uint8_t timeSecs[4];          ///< Time from Unix epoch in secs
        uint8_t timeUSecs[3];         ///< Micro secs portion of the time stamp

    };

    #pragma pack(pop)


    /// A class that parses an nVoke GPIO file and separates the data packets by stream
    /// 
    class GpioDataFile
    {
    public:

        /// An enumerator of the different signals available in the GPIO file
        ///
        enum class SignalType 
        {
            GPIO1 = 0x01,     ///<
            GPIO2 = 0x02,     ///<
            GPIO3 = 0x03,     ///<
            GPIO4 = 0x04,     ///<
            SYNC  = 0x05,     ///<
            TRIG  = 0x06,     ///<
            GPIO4_AI = 0x07,  ///<
            EXLED = 0x08,     ///<
            OGLED = 0x09,     ///<
            DILED = 0x0A,     ///<
            SYNCPKT = 0x55    ///<
        };
 
        /// Default contructor
        /// Constructs an invalid file object
        GpioDataFile();

        /// Constructor for a valid file object
        /// \param inFileName  the name of the file to read
        /// \param inOutputDir the directory that is going to contain the files for individual streams
        GpioDataFile(const std::string & inFileName, const std::string & inOutputDir);

        /// Destructor
        ///
        ~GpioDataFile();

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
        /// \param outFileNames  a vector containing the filenames of the output files
        void getOutputFileNames(std::vector<std::string> & outFileNames);

    private:
        /// \return whether the input data packet is a sync packet or not. 
        /// \param data pointer to a data packet.
        bool isSyncPacket(uint8_t * data);

        /// Checks whether any packets have been lost for each signal type
        /// \param data a pointer to the buffer data representing the generic packet header
        void checkEventCounter(uint8_t * data);
        

        /// Parse the unix time from a data packet. 
        /// \param inTime a 4 byte array containing the byte data to convert
        uint32_t getUnixTimeInSecs(uint8_t * inTime);

        /// Parse the microsecs portion of the timestamp from a data packet. 
        /// \param inTime a 3 byte array containing the byte data to convert
        uint32_t getUsecs(uint8_t * inTime);

        /// Writes a GPIO packet to a file
        /// \param inPkt the data packet
        void writeGpioPkt(const std::vector<uint8_t> & inPkt);

        /// Writes a LED packet to a file
        /// \param inPkt the data packet
        void writeLedPkt(const std::vector<uint8_t> & inPkt);

        /// Writes an analog follow packet to a file
        /// \param inPkt the data packet
        void writeAnalogFollowPkt(const std::vector<uint8_t> & inPkt);

        /// Close and delete output file streams
        /// 
        void closeFiles();

        /// Writes the data to an output file
        /// \param inFileSuffix     The signal type that will be used a suffix for the output file
        /// \param inMode           LED mode or GPIO mode used during acquisition (see s_gpioModeMap and s_ledModeMap)
        /// \param inTriggerFollow  The channel used during trigger or follow mode
        /// \param inTimeStampSec   The Seconds from unix epoch
        /// \param inTimeStampUSec  The micro seconds portion of the timestamp
        /// \param inState          Whether on or off (if in digital mode)
        /// \param inPowerLevel     The LED power level in units of mW/mm^2
        void writeToFile(
            const std::string & inFileSuffix, 
            const std::string & inMode,
            const std::string & inTriggerFollow, 
            const uint32_t inTimeStampSec, 
            const uint32_t inTimeStampUSec, 
            bool inState, 
            double inPowerLevel);

        /// Writes the json header for an output file
        /// \param file             The file stream
        /// \param inSignal         The signal type for this file
        /// \param inMode           LED mode or GPIO mode used during acquisition (see s_gpioModeMap and s_ledModeMap)
        /// \param inTriggerFollow  The channel used during trigger or follow mode
        void writeHeader(
            std::ofstream & file, 
            const std::string & inSignal, 
            const std::string & inMode,
            const std::string & inTriggerFollow);

        /// True if the movie file is valid, false otherwise.
        bool m_valid = false;

        /// The name of the data file.
        std::string m_fileName;

        /// The directory where output files will be written.
        std::string m_outputDir;

        /// A map containing the names of the output files and their streams. 
        std::map<std::string, std::ofstream *> m_outputFiles;

        /// A map containing the the names of the output files and their corresponding modes
        /// This is used to check for consistency in the packets
        std::map<std::string, std::string> m_outputModes; 

        /// A map used to keep track of event counters per channel (channel, counter)
        std::map<uint8_t, uint8_t> m_signalEventCounters;

        /// The file stream
        std::fstream m_file;

        std::ios::pos_type m_fileSizeInBytes = 0;

        /// Check in callback for reporting progress
        AsyncCheckInCB_t m_checkInCB;

        static std::map<uint8_t, std::string> s_dataTypeMap;
        static std::map<uint8_t, std::string> s_gpioModeMap;
        static std::map<uint8_t, std::string> s_ledGpioFollowMap;
        static std::map<uint8_t, std::string> s_ledStateMap;
        static std::map<uint8_t, std::string> s_ledModeMap;

    };
}
#endif // ISX_GPIO_DATA_FILE_H