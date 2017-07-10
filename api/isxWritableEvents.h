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

    /// Write a cell header preceding the data for that cell
    /// \param inCellName the name of the cell
    /// \param inNumPackets the number of data packets that will follow this header
    virtual 
    void 
    writeCellHeader(
        const std::string & inCellName,
        const isx::isize_t inNumPackets) = 0;

    /// Write data for one event
    /// \param inTimeStampUSec event time in microsecs since unix epoch
    /// \param inValue  event value
    virtual 
    void 
    writeDataPkt(
        const uint64_t inTimeStampUSec,
        const float inValue) = 0;

    /// Close file for writing
    /// 
    virtual 
    void 
    closeForWriting() = 0; 

};

} // namespace isx

#endif // ifndef ISX_WRITABLE_EVENTS_H