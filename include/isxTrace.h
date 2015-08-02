#ifndef ISX_TRACE_H
#define ISX_TRACE_H

#include <cinttypes>

#include "isxTime.h"
#include "isxTimeGrid.h"

namespace isx {

/*!
 * A function of time with a discrete domain.
 */
template <class T = float>
class Trace {

public:

    /*!
     * Trace constructor.
     *
     * \param   start       The start time of the trace.
     * \param   numTimes    The number of time points in the trace.
     * \param   step        The step time of the trace in milliseconds.
     */
    Trace(isx::Time start, uint32_t numTimes, uint16_t step);

    /*!
     * Read access to a range value by index.
     *
     * \param   i   The index.
     * \return  The ith range value.
     */
    T getValue(uint32_t i) const;

    /*!
     * Read access to a range value by time.
     *
     * \param   t   The time.
     * \return      The range value at time t.
     */
    //T getValue(mosaic::Time t) const;

    /*!
     * Write access to a range value by index.
     *
     * \param   i   The index.
     * \param   val The new value.
     * \return  The new ith range value.
     */
    void setValue(uint32_t i, T val);

private:

    //! The discrete domain of the function.
    isx::TimeGrid m_Domain;

    //! The discrete range of the function.
    T* m_Range;

}; // class

} // namespace

#endif // ISX_TRACE_H
