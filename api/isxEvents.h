#ifndef ISX_EVENTS_H
#define ISX_EVENTS_H

#include "isxTimingInfo.h"
#include "isxCoreFwd.h"
#include "isxTrace.h"
#include "isxLogicalTrace.h"
#include "isxAsyncTaskResult.h"

#include <string>
#include <functional>

namespace isx
{

/// Interface for Events data.
class Events
{

public:

    /// The type of callback for reading a logical trace from disk
    using GetLogicalDataCB_t = std::function<SpLogicalTrace_t()>;
    /// The type of callback for getting a logical trace asynchronously
    using EventsGetLogicalDataCB_t = std::function<void(AsyncTaskResult<SpLogicalTrace_t>)>;

    /// \return True if the Events file is valid, false otherwise.
    ///
    virtual
    bool
    isValid() const = 0;


    /// \return     The name of the file.
    ///
    virtual
    const std::string &
    getFileName() const = 0;

    /// \return the number of cells contained in the file
    ///
    virtual
    isize_t
    numberOfCells() = 0;

    /// \return a list of the cell names contained in this file
    ///
    virtual
    const std::vector<std::string>
    getCellNamesList() const = 0;

    /// \return the logical trace for the requested cell name or nullptr if the file doesn't contain data for that cell
    /// \param inCellName the name of the requested channel (as returned by getCellNamesList())
    virtual
    SpLogicalTrace_t
    getLogicalData(const std::string & inCellName) = 0;

    /// Get an logical trace asynchronously
    /// \param inCellName the name of the requested cell name (as returned by getCellNamesList())
    /// \param inCallback
    virtual
    void
    getLogicalDataAsync(const std::string & inCellName, EventsGetLogicalDataCB_t inCallback) = 0;

    /// \return     The timing information read from the Events set.
    virtual
    isx::TimingInfo 
    getTimingInfo() const = 0;

    /// \return     The TimingInfos of a GpioSeries.
    ///             For a regular Events set this will contain one TimingInfo object
    ///             matching getTimingInfo.
    virtual
    isx::TimingInfos_t
    getTimingInfosForSeries() const = 0;

    /// Cancel all pending read requests (schedule with getTraceAsync/getImageAsync).
    ///
    virtual
    void
    cancelPendingReads() = 0;

}; // class Events

/// Read a Events set from a file.
/// \param  inFileName  The path of the Events file.
/// \return             The Events set read.
SpEvents_t
readEvents(const std::string & inFileName);

/// Write a Events set to a file.
/// \param  inFileName  The path of the Events file.
/// \return             The Events set to write to.
SpWritableEvents_t
writeEvents(const std::string & inFileName);

/// Attempt to read a series of Events sets from files.
/// \param  inFileNames The paths of the Events files.
/// \return             The series of Events sets.
SpEvents_t
readEventsSeries(const std::vector<std::string> & inFileNames);

} // namespace isx

#endif /// ISX_EVENTS_H
