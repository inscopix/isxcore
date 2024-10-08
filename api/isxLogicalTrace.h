#ifndef ISX_LOGICAL_TRACE_H
#define ISX_LOGICAL_TRACE_H

#include <memory>
#include <map>
#include "isxTimingInfo.h"

namespace isx
{

/// A function of time with a discrete domain and a scalar range.
///
class LogicalTrace
{
public:
    /// Default constructor
    ///
    LogicalTrace() :
        m_timingInfo(TimingInfo())
    {
       
    }

    /// Fully specified constructor.
    ///
    /// \param   inTimingInfo       the timing information for the trace.
    /// \param   inChannelName      the name of the trace
    LogicalTrace(const TimingInfo & inTimingInfo, const std::string & inChannelName)
        : m_timingInfo(inTimingInfo)
        , m_name(inChannelName)
    {
    }

    /// Destructor.
    ///
    ~LogicalTrace()
    {

    }

    /// Get the timing information for the trace
    ///
    const TimingInfo & getTimingInfo() const
    {
        return m_timingInfo;
    }

    /// \return the set of values and their timestamps
    ///
    const std::map<Time, float> & getValues() const
    {
        return m_values;
    }

    /// Set the values for this trace
    /// \param inValues the map of timestamps and values
    void setValues(const std::map<Time, float> & inValues)
    {
        m_values = inValues;
    }
    
    /// Adds a value to the time-value map
    /// \param inTime the timestamp
    /// \param inValue the value to add
    void addValue(const Time & inTime, float inValue)
    {
        m_values[inTime] = inValue;
    }

    /// \return the name of the trace
    /// 
    const std::string & getName() const
    {
        return m_name;
    }



private:

    /// The temporal domain of the function.
    TimingInfo m_timingInfo;
    std::map<Time, float>  m_values;
    std::string            m_name;


}; // class

/// Gets the plot coordinates for a logical trace.
///
/// \param  inTrace                 The trace to plot, which could be a series.
/// \param  inTis                   The timing infos associated with the trace.
/// \param  inCoordsForSquareWave   True if a square wave is being plotted, false otherwise.
/// \param  outX                    The x-coordinates for each member of the series.
/// \param  outY                    The y-coordinates for each member of the series.
void
getCoordinatesFromLogicalTrace(
        const SpLogicalTrace_t & inTrace,
        const TimingInfos_t & inTis,
        const bool inCoordsForSquareWave,
        std::vector<std::vector<double>> & outX,
        std::vector<std::vector<double>> & outY);

} // namespace

#endif // ISX_LOGICAL_TRACE_H
