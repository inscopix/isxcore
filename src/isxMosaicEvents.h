#ifndef ISX_MOSAIC_EVENTS_H
#define ISX_MOSAIC_EVENTS_H

#include "isxWritableEvents.h"
#include "isxFileTypes.h"
#include <memory>

namespace isx
{

class EventBasedFile;
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

    isx::TimingInfo
    getTimingInfo() const override;

    isx::TimingInfos_t
    getTimingInfosForSeries() const override;

    void
    cancelPendingReads() override;

    void 
    setTimingInfo(const isx::TimingInfo & inTimingInfo) override;

    void 
    writeDataPkt(
        const uint64_t inSignalIdx,
        const uint64_t inTimeStampUSec,
        const float inValue) override;

    void 
    closeForWriting(const std::vector<std::string> & inNewChannelNames = {}) override;

    bool 
    hasMetrics() const override;

    SpTraceMetrics_t 
    getTraceMetrics(isize_t inIndex) const override;

    void
    setTraceMetrics(isize_t inIndex, const SpTraceMetrics_t & inMetrics) override;

private:
    FileType                                     m_type;
    std::shared_ptr<EventBasedFile>              m_file;
    std::shared_ptr<IoTaskTracker<LogicalTrace>>      m_logicalIoTaskTracker;

}; // class MosaicEvents

} // namespace isx

#endif /// ISX_MOSAIC_EVENTS_H
