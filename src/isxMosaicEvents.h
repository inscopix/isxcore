#ifndef ISX_MOSAIC_EVENTS_H
#define ISX_MOSAIC_EVENTS_H

#include "isxWritableEvents.h"

#include <memory>

namespace isx
{

class TimeStampedDataFile;
template <typename T> class IoTaskTracker;

/// Class used to read and handle GPIO data
class MosaicEvents
    : public WritableEvents
    , public std::enable_shared_from_this<MosaicEvents>
{

public:

    /// Constructor
    ///
    MosaicEvents();

    /// Constructor
    /// \param inFileName the name of the Events data file
    /// \param inOpenForWrite whether to create a writable object or not
    MosaicEvents(const std::string & inFileName, bool inOpenForWrite = false);

    /// Destructor
    ///
    ~MosaicEvents();

    // Overrides. See isxEvents.h for documentation.
    bool
    isValid() const override;


    const std::string &
    getFileName() const override;

    isize_t
    numberOfCells() override;

    const std::vector<std::string>
    getCellNamesList() const override;

    SpLogicalTrace_t
    getLogicalData(const std::string & inCellName) override;

    void
    getLogicalDataAsync(const std::string & inCellName, EventsGetLogicalDataCB_t inCallback) override;

    const isx::TimingInfo &
    getTimingInfo() const override;

    isx::TimingInfos_t
    getTimingInfosForSeries() const override;

    void
    cancelPendingReads() override;

    void 
    setTimingInfo(const isx::TimingInfo & inTimingInfo) override;

    void 
    writeCellHeader(
        const std::string & inCellName,
        const isx::isize_t inNumPackets) override;

    void 
    writeDataPkt(
        const uint64_t inTimeStampUSec,
        const float inValue) override;

    void 
    closeForWriting() override;

private:

    std::shared_ptr<TimeStampedDataFile>              m_file;
    std::shared_ptr<IoTaskTracker<LogicalTrace>>      m_logicalIoTaskTracker;

}; // class MosaicEvents

} // namespace isx

#endif /// ISX_MOSAIC_EVENTS_H
