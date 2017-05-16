#ifndef ISX_GPIO_FILE_H
#define ISX_GPIO_FILE_H

#include <string>
#include <fstream>
#include "isxCoreFwd.h"
#include "isxTimingInfo.h"
#include "isxTrace.h"
#include "isxLogicalTrace.h"


namespace isx
{   
    
/// A class for a file containing gpio data
///
class GpioFile
{
    
public:

    #pragma pack(push, 1)
    /// The data packet contained in the GPIO file
    /// Note: The value and state of the GPIO are both enconded in a 32-bit variable (m_value). 
    /// The value cannot be negative. Additionally, the state is stored in the data packet 
    /// using the MSB (sign bit) that would correspond to the 32-bit floating point value. 
    /// m_value = [state bit, 31-bit float value]
    class DataPkt
    {
    
    public:
        /// Default constructor
        ///
        DataPkt();

        /// Alternative contructor
        /// \param inTimeStampUSec  The timestamp in usecs from unix epoch
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

        uint64_t m_timeStampUSec;               ///< Time from Unix epoch in microsecs
        union
        {
            uint64_t m_data;                    ///< Generic 64-bit structure to hold unformatted data
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
    GpioFile();

    /// Read constructor.
    ///
    /// This opens an existing gpio file and reads information from its
    /// header.
    ///
    /// \param  inFileName  The name of the gpio file.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the gpio file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the gpio file fails.
    GpioFile(const std::string & inFileName);

    /// Destructor.
    ///
    ~GpioFile();
    
    /// \return True if the gpio file is valid, false otherwise.
    ///
    bool 
    isValid() const; 

    /// \return true if this file contains data for an analog channel, false otherwise
    bool 
    isAnalog() const;

    /// \return     The name of the file.
    ///
    const std::string & 
    getFileName() const;
    
    /// \return the number of channels contained in the file
    ///
    const isize_t 
    numberOfChannels();

    /// \return a list of the channels contained in this file
    /// 
    const std::vector<std::string> 
    getChannelList() const;

    /// \return the trace for the analog channel or nullptr if the file doesn't contain analog data
    /// 
    SpFTrace_t 
    getAnalogData();

    /// \return the logical trace for the requested channel or nullptr if the file doesn't contain data for that channel
    /// \param inChannelName the name of the requested channel (as returned by getChannelList())
    SpLogicalTrace_t 
    getLogicalData(const std::string & inChannelName);

    /// \return     The timing information read from the GPIO.
    ///
    const isx::TimingInfo & 
    getTimingInfo() const;


private:

    /// Reads the file footer and initializes this object with that information
    void 
    readFileFooter();

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

};
}
#endif // ISX_GPIO_FILE_H
