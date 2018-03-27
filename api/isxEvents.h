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

/// This struct aggregates temporal statistics of traces. These stats are useful for determining whether a cell is good
/// or complete trash.
struct TraceMetrics
{
    /// Default contructor
    ///
    TraceMetrics() {}

    /// Convenience constructor to fill all members at once
    /// \param inSnr
    /// \param inMad
    /// \param inEventRate
    /// \param inEventAmpMedian
    /// \param inEventAmpSd
    /// \param inRiseMedian
    /// \param inRiseSd
    /// \param inDecayMedian
    /// \param inDecaySd
    TraceMetrics(
        float inSnr,
        float inMad,
        float inEventRate,
        float inEventAmpMedian,
        float inEventAmpSd, 
        float inRiseMedian,
        float inRiseSd,
        float inDecayMedian,
        float inDecaySd) :
        m_snr(inSnr),
        m_mad(inMad),
        m_eventRate(inEventRate),
        m_eventAmpMedian(inEventAmpMedian),
        m_eventAmpSD(inEventAmpSd),
        m_riseMedian(inRiseMedian),
        m_riseSD(inRiseSd),
        m_decayMedian(inDecayMedian),
        m_decaySD(inDecaySd) {}

    float m_snr         = 0.f; ///< The signal-to-noise ratio of the trace, the median amplitude divided by the median absolute deviation
    float m_mad         = 0.f; ///< The median absolute deviation of the trace
    float m_eventRate   = 0.f; ///< The event rate of the trace in Hz
    float m_eventAmpMedian = 0.f; ///< The median event amplitude of the trace
    float m_eventAmpSD  = 0.f; ///< The standard deviation from the median of the event amplitudes
    float m_riseMedian  = 0.f; ///< The median event rise time for events in seconds
    float m_riseSD      = 0.f; ///< The SD from the median of event rise times in seconds
    float m_decayMedian = 0.f; ///< The median event decay time in seconds
    float m_decaySD     = 0.f; ///< The SD from the median of decay time in seconds
};

using SpTraceMetrics_t = std::shared_ptr<TraceMetrics>;

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
    ///             Depending on the signal, you may not want to trust the step and number
    ///             of times, as they may simply spoofing until we have a proper way to
    ///             handle irregular timing info.
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

    /// \return true if the event set contains trace metrics
    /// 
    virtual 
    bool 
    hasMetrics() const = 0;

    /// Get all the quality assessment metrics for a given cell trace 
    /// \param inIndex the cell index
    virtual 
    SpTraceMetrics_t 
    getTraceMetrics(isize_t inIndex) const = 0;

    /// Set the quality assessment metrics for a cell trace
    /// \param inIndex the cell index
    /// \param inMetrics the metrics structure
    virtual 
    void
    setTraceMetrics(isize_t inIndex, const SpTraceMetrics_t & inMetrics) = 0;

}; // class Events

/// Read a Events set from a file.
/// \param  inFileName  The path of the Events file.
/// \return             The Events set read.
SpEvents_t
readEvents(const std::string & inFileName);

/// Write a Events set to a file.
/// \param  inFileName      The path of the Events file.
/// \param  inChannelNames  The names of the channels.
/// \param  inChannelSteps  The steps of the channels.
/// \return                 The Events set to write to.
SpWritableEvents_t
writeEvents(
        const std::string & inFileName,
        const std::vector<std::string> & inChannelNames,
        const std::vector<DurationInSeconds> & inChannelSteps);

/// Attempt to read a series of Events sets from files.
/// \param  inFileNames The paths of the Events files.
/// \return             The series of Events sets.
SpEvents_t
readEventsSeries(const std::vector<std::string> & inFileNames);

} // namespace isx

#endif /// ISX_EVENTS_H
