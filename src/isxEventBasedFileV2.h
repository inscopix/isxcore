#ifndef ISX_EVENT_BASED_FILE_V2_H
#define ISX_EVENT_BASED_FILE_V2_H


#include <fstream>
#include <cstring>
#include "isxDataSet.h"
#include "isxEventBasedFile.h"
#include "isxFileTypes.h"

namespace isx
{


    enum class PktType : uint32_t
    {
        VALUE = 0
    };

    /*
        This class describes the file format for any data that  can be described 
        as a <timestamp, value> pair. Examples of this type of data are GPIO (either
        continuously sampled or event-triggered) and the output of calcium event detection. 
        The file format can acommodate different signals of different types (continuous or sparse) and 
        different sampling rates together. 

        This file is composed of data packets (described by DataPkt) saved in binary data and
        a json file footer containing information regarding the signals described in the file.  
        
        The json footer contains start and end times for the recording, 
        a list of signals, their sampling periods, their number of packets, and their temporal offsets
        from the beginning of the recording. All timestamps saved in the data packets are offsets
        from the start time in microseconds. If a channel contains data corresponding to 
        event-triggered data and is therefore not sampled continuously, the sampling period is 
        DurationInSeconds(0, 1). It is up to the client to pass this information when writing a file. 


    */

 
    class EventBasedFileV2 : public EventBasedFile
    {
    public: 
        #pragma pack(push, 1)
        struct DataPkt
        {
            DataPkt(uint64_t inOffsetMicroSecs = 0, float inVal = 0.f, uint64_t inSignal = 0)
            {                                
                type = uint32_t(PktType::VALUE);
                offsetMicroSecs = inOffsetMicroSecs;
                signal = inSignal;
                value = inVal;
            }            

            uint32_t type = 0;              ///< currently unused but useful for the future, when other types of packets might be introduced
            uint64_t offsetMicroSecs = 0;   ///< offset in microseconds from the beginning of the recording
            uint64_t signal = 0;            ///< this is the index of the signal in the channel list (file footer)
                                            ///< and should uniquely identify each packet                            
            float value = 0.f;              ///< used for digital (0/1) or floating-point valued signals
                
        };
        #pragma pack(pop)

        EventBasedFileV2();

        EventBasedFileV2(
            const std::string & inFileName, 
            DataSet::Type inType = DataSet::Type::GPIO, 
            bool inWrite = false);

        ~EventBasedFileV2();

        void 
        readAllTraces(std::vector<SpFTrace_t> & inContinuousTraces, std::vector<SpLogicalTrace_t> & inLogicalTraces);

        SignalType getSignalType(const std::string & inChannelName);

        bool 
        isValid() const override; 

        const std::string & 
        getFileName() const override;

        const std::vector<std::string> 
        getChannelList() const override;        

        void 
        setChannelList(const std::vector<std::string> & inNewChannelNames);

        /// \return the logical trace for the requested channel or nullptr if the file doesn't contain data for that channel
        /// \param inChannelName the name of the requested channel (as returned by getChannelList())
        SpLogicalTrace_t 
        getLogicalData(const std::string & inChannelName) override;

        SpFTrace_t 
        getAnalogData(const std::string & inChannelName) override;

        const isx::TimingInfo 
        getTimingInfo() const override;

        /// \return     The timing information read from the file.
        ///
        const TimingInfo  
        getTimingInfo(const std::string & inChannelName) const;

        /// Set the timing information that will be written in the file footer
        /// \param  
        void setTimingInfo(const Time & inStartTime, const Time & inEndTime, const std::vector<DurationInSeconds> & inSteps);

        /// Writes a data packet to the file
        /// \param 
        void writeDataPkt(const DataPkt & inData);

        /// Write file footer and close the file
        /// 
        void closeFileForWriting();

        bool 
        hasMetrics() const;

        SpTraceMetrics_t 
        getTraceMetrics(isize_t inIndex) const;

        void
        setTraceMetrics(isize_t inIndex, const SpTraceMetrics_t & inMetrics);


    private:

        /// Reads the file footer and initializes this object with that information
        void 
        readFileFooter();

        void 
        writeFileFooter();

        void 
        updateGeneralTimingInfo();

        std::string                     m_fileName;

        std::vector<std::string>        m_channelList; 

        Time                            m_startTime;

        Time                            m_endTime;

        std::vector<DurationInSeconds>  m_steps;
        std::vector<uint64_t>           m_startOffsets;
        std::vector<uint64_t>           m_numSamples;

        DataSet::Type                   m_dataType;

        
        /// The file stream
        std::fstream                    m_file;

        /// The header offset.
        std::ios::pos_type              m_headerOffset;

        bool                            m_valid = false;

        bool                            m_openForWrite = false;
        bool                            m_closedForWriting = false;

        EventMetrics_t                  m_traceMetrics;

        const static size_t             s_fileVersion = 0;

    };  
}

#endif //ISX_EVENT_BASED_FILE_V2_H