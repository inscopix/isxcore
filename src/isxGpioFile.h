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
    struct DataPkt
    {
        /// Default constructor
        ///
        DataPkt() {}
        
        /// Alternative contructor
        /// \param inTimeStampSec   The timestamp in secs from unix epoch
        /// \param inTimeStampUSec  The microsecs portion of the timestamp
        /// \param inState          The state on/off
        /// \param inValue     Power level of LED lights
        DataPkt(
            const uint32_t inTimeStampSec, 
            const uint32_t inTimeStampUSec, 
            const bool inState, 
            const double inValue) :
            timeStampSec(inTimeStampSec),
            timeStampUSec(inTimeStampUSec),
            state(0),
            value(inValue)
        {
            if(inState)
            {
                state = 1;
            }
        }

        /// \return the timestamp for this packet
        ///
        Time getTime() const
        {
            DurationInSeconds secsFromUnixEpoch(isize_t(timeStampSec), 1);
            DurationInSeconds usecs(timeStampUSec, 1000000);
            secsFromUnixEpoch += usecs;
            return Time(secsFromUnixEpoch);
        }
        
        uint32_t    timeStampSec;       ///< Time from Unix epoch in secs
        uint32_t    timeStampUSec;      ///< Micro secs portion of the time stamp
        char        state;              ///< On/Off
        double      value;              ///< The recorded value, used for LED power and analog traces
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
    SpDTrace_t 
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
