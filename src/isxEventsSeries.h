#ifndef ISX_EVENTS_SERIES_H
#define ISX_EVENTS_SERIES_H

#include "isxEvents.h"
#include "isxCoreFwd.h"
#include "isxAsync.h"
#include "isxMutex.h"

#include <fstream>
#include <map>
#include <memory>

namespace isx
{

/// A temporal series of EVENTS sets.
///
class EventsSeries
    : public Events
    , public std::enable_shared_from_this<EventsSeries>
{
public:

    /// Empty constructor.
    ///
    /// This creates an invalid events series and is for allocation purposes only.
    EventsSeries();

    /// Read constructor.
    ///
    /// This opens existing events files and reads information from their headers.
    ///
    /// \param  inFileNames     The paths of the events files to read.
    /// \throw  ExceptionFileIO If reading one of the files fails.
    /// \throw  ExceptionDataIO If parsing one of the files fails.
    EventsSeries(const std::vector<std::string> & inFileNames);

    /// Destructor.
    ///
    ~EventsSeries();

    // Overrides
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

private:

    /// True if the events series is valid, false otherwise.
    bool m_valid = false;

    TimingInfo                  m_gaplessTimingInfo; ///< only really useful for global number of times
    std::vector<SpEvents_t>     m_events;
    static const std::string    s_fileName;

}; // class EventsSeries

} // namespace isx

#endif // ISX_EVENTS_SERIES_H
