#ifndef ISX_TRACE_H
#define ISX_TRACE_H

#include <cstdint>

#include "isxTime.h"
#include "isxTimingInfo.h"

namespace isx
{

/// A function of time with a discrete domain and a scalar range.
///
template <class T = float>
class Trace
{
public:

    /// Fully specified constructor.
    ///
    /// \param   start       The start time of the trace.
    /// \param   step        The step time of the trace in milliseconds.
    /// \param   numTimes    The number of time points in the trace.
    Trace(isx::Time start, isx::Ratio step, isize_t numTimes);

    /// Destructor.
    ///
    ~Trace();

    /// Read access to a range value by index.
    ///
    /// \param   index      The index.
    /// \return             The value at the index.
    T getValue(isize_t index) const;

    /// Write access to a range value by index.
    ///
    /// \param   index      The index.
    /// \param   value      The new value.
    void setValue(isize_t index, T value);

private:

    /// The temporal domain of the function.
    isx::TimingInfo m_timingInfo;

    /// The range of the function.
    T* m_data;

}; // class

} // namespace

#endif // ISX_TRACE_H
