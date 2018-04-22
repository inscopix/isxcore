#ifndef ISX_EVENT_BASED_FILE_V1_H
#define ISX_EVENT_BASED_FILE_V1_H

#include <fstream>


#include "isxLogicalTrace.h"
#include "isxEventBasedFile.h"


namespace isx
{   
    
/// A class for a file containing gpio/event data
/// Note salpert 2/6/2018: This is a legacy class and is kept here for backwards compatibility. Use 
/// EventBasedFileV2 for any new use of this file type. 
class EventBasedFileV1 : public EventBasedFile
{
    
public:

    /// Type of data stored in time stamped files
    ///
    enum class StoredData
    {
        GPIO = 0,   ///<
        EVENTS      ///<
    };

    #pragma pack(push, 1)
    /// The data packet contained in the GPIO file
    /// Note: The value and state of the GPIO are both encoded in a 32-bit variable (m_value). 
    /// The value cannot be negative. Additionally, the state is stored in the data packet 
    /// using the MSB (sign bit) that would correspond to the 32-bit floating point value. 
    /// m_value = [state bit, 31-bit float value]
    class DataPkt
    {
    
    public:
        /// Default constructor
        ///
        DataPkt();

        /// Alternative constructor
        /// \param inTimeStampUSec  The timestamp in microseconds from unix epoch
        /// \param inState          The state on/off
        /// \param inValue          Power level of LED lights. This value cannot be negative.
        DataPkt(
            const uint64_t inTimeStampUSec,
            const bool inState,
            const float inValue);

        /// \return the state 
        ///
        bool getState();

        /// \return the value
        /// 
        float getValue();

        /// \return the timestamp for this packet
        ///
        Time getTime() const;

        uint64_t m_timeStampUSec = 0;               ///< Time from Unix epoch in microseconds
        union
        {
            uint64_t m_data = 0;                ///< Generic 64-bit structure to hold unformatted data
            struct
            {
                uint32_t m_value;               ///< The recorded value, used for LED power, analog traces, and digital state (0,1)
                uint32_t m_reserved;
            };
        };
    };    
    #pragma pack(pop)

    /// Empty constructor.
    ///
    /// This creates a valid c++ object but an invalid gpio file.
    EventBasedFileV1();

    /// Read constructor.
    ///
    /// This opens an existing gpio file and reads information from its
    /// header.
    ///
    /// \param  inFileName  The name of the gpio file.
    /// \throw  isx::ExceptionFileIO    If reading the gpio file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the gpio file fails.
    EventBasedFileV1(const std::string & inFileName); 
    
    /// Write constructor.
    ///
    /// This opens a gpio file for writing.
    ///
    /// \param  inFileName  The name of the gpio file.
    /// \param dataType     The type of data to be stored in the file.
    /// \param inIsAnalog   Indicates if the file contains analog data (valid for GPIO)
    /// \throw  isx::ExceptionFileIO    If opening the gpio file fails.
    EventBasedFileV1(const std::string & inFileName, StoredData dataType, bool inIsAnalog = false);

    /// Destructor.
    ///
    ~EventBasedFileV1();
    
    /// \return True if the gpio file is valid, false otherwise.
    ///
    bool 
    isValid() const override; 

    /// \return     The name of the file.
    ///
    const std::string & 
    getFileName() const override;
    
    /// \return a list of the channels contained in this file
    /// 
    const std::vector<std::string> 
    getChannelList() const override;

    /// \return the trace for the analog channel or nullptr if the file doesn't contain analog data
    /// 
    SpFTrace_t 
    getAnalogData(const std::string & inChannelName) override;

    /// \return the logical trace for the requested channel or nullptr if the file doesn't contain data for that channel
    /// \param inChannelName the name of the requested channel (as returned by getChannelList())
    SpLogicalTrace_t 
    getLogicalData(const std::string & inChannelName) override;

    /// \return     The timing information read from the GPIO.
    ///
    const isx::TimingInfo  
    getTimingInfo() const override;

    /// Set the timing information that will be written in the file footer
    /// \param inTimingInfo the timing information to set for the file. 
    void setTimingInfo(const isx::TimingInfo & inTimingInfo);

    /// Write the channel header to the file
    /// 
    /// \param inChannel the channel name 
    /// \param inMode the mode of operation for that channel
    /// \param inTriggerFollow the trigger follow setting for the channel
    /// \param inNumPackets the number of data packets following this channel header
    void writeChannelHeader(
        const std::string & inChannel,
        const std::string & inMode,
        const std::string & inTriggerFollow,
        const isx::isize_t inNumPackets);

    /// \return the type of data stored in this file
    ///
    StoredData
    getStoredDataType() const;

    /// \return true if this file contains data for an analog channel, false otherwise
    bool 
    isAnalog() const;

    /// Writes a data packet to the file
    /// \param inData the data packet to write
    void writeDataPkt(const DataPkt & inData);

    /// Write file footer and close the file
    /// 
    void closeFileForWriting();

    std::string getExtraProperties() const override;

    void setExtraProperties(const std::string & inProperties) override;

private:

    /// Reads the file footer and initializes this object with that information
    void 
    readFileFooter();

    void 
    writeFileFooter();

    /// Reads the channel header
    /// \param inChannelName the name of the requested channel (as returned by getChannelList())
    /// \return the header as a string
    std::string 
    readChannelHeader(const std::string & inChannelName);

    /// True if the gpio file is valid, false otherwise.
    bool m_valid = false;

    /// True if this Gpio file contains analog data, false otherwise.
    bool m_analog = false;

    /// The name of the gpio file.
    std::string m_fileName;

    /// The timing information of the gpio set.
    TimingInfo m_timingInfo;
    
    /// A map of the channels in this file and their offsets
    std::map<std::string, int> m_channelOffsets;

    /// The header offset.
    std::ios::pos_type m_headerOffset;

    /// The file stream
    std::fstream m_file;

    bool m_openForWrite = false;
    bool m_closedForWriting = false;

    StoredData m_dataType;


    const static size_t s_fileVersion = 1;
};
}
#endif // ISX_EVENT_BASED_FILE_V1_H
