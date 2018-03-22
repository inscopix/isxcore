#ifndef ISX_EVENT_BASED_FILE_V2_H
#define ISX_EVENT_BASED_FILE_V2_H

#include <fstream>
#include <cstring>
#include "isxDataSet.h"
#include "isxEventBasedFile.h"
#include "isxFileTypes.h"
#include "isxEvents.h"
#include "isxJsonUtils.h"

namespace isx
{

/// This class describes the file format for any data that  can be described
/// as a <timestamp, value> pair. Examples of this type of data are GPIO (either
/// continuously sampled or event-triggered) and the output of calcium event detection.
/// The file format can acommodate different signals of different types (continuous or sparse) and
/// different sampling rates together.
///
/// This file is composed of data packets (described by DataPkt) saved in binary data and
/// a JSON file footer containing information regarding the signals described in the file.
///
/// The JSON footer contains start and end times for the recording,
/// a list of signals, their sampling periods, their number of packets, and their temporal offsets
/// from the beginning of the recording. All timestamps saved in the data packets are offsets
/// from the start time in microseconds.
class EventBasedFileV2 : public EventBasedFile
{
public:
    #pragma pack(push, 1)
    struct DataPkt
    {
        DataPkt(uint64_t inOffsetMicroSecs = 0, float inVal = 0.f, uint64_t inSignal = 0)
        {
            offsetMicroSecs = inOffsetMicroSecs;
            signal = inSignal;
            value = inVal;
        }

        uint64_t offsetMicroSecs = 0;   ///< offset in microseconds from the beginning of the recording
        uint64_t signal = 0;            ///< this is the index of the signal in the channel list (file footer)
                                        ///< and should uniquely identify each packet
        float value = 0.f;              ///< used for digital (0/1) or floating-point valued signals

    };
    #pragma pack(pop)

    /// Read constructor.
    EventBasedFileV2(const std::string & inFileName);

    /// Write constructor.
    EventBasedFileV2(
        const std::string & inFileName,
        DataSet::Type inType,
        const std::vector<std::string> & inChannelNames,
        const std::vector<DurationInSeconds> & inChannelSteps,
        const std::vector<SignalType> & inChannelTypes);

    EventBasedFileV2();

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

    /// \return the logical trace for the requested channel or nullptr if the file doesn't contain data for that channel
    /// \param inChannelName the name of the requested channel (as returned by getChannelList())
    SpLogicalTrace_t
    getLogicalData(const std::string & inChannelName) override;

    SpFTrace_t
    getAnalogData(const std::string & inChannelName) override;

    const isx::TimingInfo
    getTimingInfo() const override;

    /// \return The timing info associated with a channel.
    ///         For sparse signals, the start time will match the start time of this file,
    ///         the step duration will match that given on construction, and the number of samples
    ///         will be invented to give a regular TimingInfo object.
    ///         For dense signals, the start time will match the start time of this file, and
    ///         the step duration and number of samples should be accurate.
    const TimingInfo
    getTimingInfo(const std::string & inChannelName) const;

    /// Set some extra timing information that's special for event
    /// based files.
    void setTimingInfo(const Time & inStartTime, const Time & inEndTime);

    /// Writes a data packet to the file
    ///
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

    // These times tell us when we started and stopped listening for events
    // over all channels.
    Time                            m_startTime;
    Time                            m_endTime;

    // The duration of a sample associated with each "dense" channel.
    // For a sparse signal, this represents the rate at which we listened
    // for for events (e.g. for events detected from cell activity, this
    // would match the sample rate of the cell activity).
    std::vector<DurationInSeconds>  m_steps;

    // The type of signal contained in each channel.
    std::vector<SignalType>         m_signalTypes;

    // The time of the first sample in each channel.
    // This used to make the channel specific timing info from
    // EventBasedFileV2::getTimingInfo(const std::string &) return
    // a TimingInfo object with the offset start time rather than the
    // global start time over all channels.
    std::vector<uint64_t>           m_startOffsets;

    // The number of samples in each channel.
    std::vector<uint64_t>           m_numSamples;

    // Used to distinguish between GPIO, events, etc.
    DataSet::Type                   m_dataType;

    std::fstream                    m_file;

    std::ios::pos_type              m_headerOffset;

    bool                            m_valid = false;

    bool                            m_openForWrite = false;
    bool                            m_closedForWriting = false;

    EventMetrics_t                  m_traceMetrics;

    const static size_t             s_fileVersion = 1;
};

} // namespace isx

#endif //ISX_EVENT_BASED_FILE_V2_H
