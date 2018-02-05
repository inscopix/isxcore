#ifndef ISX_TRACE_H
#define ISX_TRACE_H

#include <cstdint>
#include <memory>
#include "isxTimingInfo.h"
#include "isxAssert.h"

namespace isx
{

/// A function of time with a discrete domain and a scalar range.
///
template <class T = float>
class Trace
{
public:
    /// Default constructor
    ///
    Trace() :
        m_timingInfo(TimingInfo())
    {

    }

    /// Fully specified constructor.
    ///
    /// \param   inTimingInfo       the timing information for the trace.
    Trace(const TimingInfo & inTimingInfo)
        : m_timingInfo(inTimingInfo)
    {
        isize_t numTimes = m_timingInfo.getNumTimes();
        ISX_ASSERT(numTimes > 0);
        m_values.reset(new T[numTimes]);
    }

    /// Destructor.
    ///
    ~Trace()
    {

    }

    /// Get the timing information for the trace
    ///
    const TimingInfo & getTimingInfo() const
    {
        return m_timingInfo;
    }

    /// \return a raw pointer to the first sample in memory
    ///
    T *
    getValues()
    {
        if (m_values)
        {
            return &m_values[0];
        }
        return 0;
    }

    /// Read access to a range value by index.
    ///
    /// \param   index      The index.
    /// \return             The value at the index.
    T getValue(isize_t index) const
    {
        return m_values[index];
    }

    /// Write access to a range value by index.
    ///
    /// \param   index      The index.
    /// \param   value      The new value.
    void setValue(isize_t index, T value)
    {
        m_values[index] = value;
    }

    /// Set the dropped frames vector for this timing info object
    /// \param inDroppedFrames The vector of dropped indexes
    void setDroppedFrames(const std::vector<isize_t> & inDroppedFrames)
    {
        m_timingInfo.setDroppedFrames(inDroppedFrames);
    }



private:

    /// The temporal domain of the function.
    TimingInfo m_timingInfo;
    std::unique_ptr<T[]> m_values = 0;


}; // class

    /// type for trace of floats
    ///
    typedef Trace<float> FTrace_t;

    /// shared_ptr type an trace of floats
    ///
    typedef std::shared_ptr<FTrace_t> SpFTrace_t;

    // type for trace of floats
    ///
    typedef Trace<double> DTrace_t;

    /// shared_ptr type an trace of floats
    ///
    typedef std::shared_ptr<DTrace_t> SpDTrace_t;

} // namespace

#endif // ISX_TRACE_H
