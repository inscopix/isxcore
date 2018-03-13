#ifndef ISX_WRITABLE_EVENTS_H
#define ISX_WRITABLE_EVENTS_H

#include "isxEvents.h"

namespace isx
{

/// Interface for writable Events data sets
///
class WritableEvents : public Events
{
public:

    /// Set the timing info for the file
    /// \param inTimingInfo the timing info
    virtual 
    void 
    setTimingInfo(const isx::TimingInfo & inTimingInfo) = 0;


    /// Write data for one event
    /// \param inSignalIdx     signal index that uniquely identifies the signal and corresponds to the index in the channel names list
    /// \param inTimeStampUSec event time in microsecs since start time of the recording
    /// \param inValue  event value
    virtual 
    void 
    writeDataPkt(
        const uint64_t inSignalIdx,
        const uint64_t inTimeStampUSec,
        const float inValue) = 0;

    /// Close file for writing
    /// \param inNewChannelNames the channel names list, corresponding to the signal indeces used in the data packets
    virtual 
    void 
    closeForWriting(const std::vector<std::string> & inNewChannelNames) = 0; 

};

} // namespace isx

#endif // ifndef ISX_WRITABLE_EVENTS_H