#ifndef ISCPX_TIMEGRID_H
#define ISCPX_TIMEGRID_H

#include <cinttypes>

#include "iscpxDateTime.h"

namespace iscpx {

/*!
 * A regularly spaced sample of time points.
 */
class TimeGrid {

public:

    /*!
     * TimeGrid constructor.
     *
     * \param start	    The start time of the samples.
     * \param numTimes	The number of samples.
     * \param step	    The step time of the samples.
     */
    TimeGrid(iscpx::DateTime start, uint32_t numTimes, uint16_t step);

    /*!
     * Get the start time of the samples.
     */
    iscpx::DateTime getStart() const;

    /*!
     * Get the end time of the samples.
     */
    iscpx::DateTime getEnd() const;

    /*!
     * Get the duration of a single sample in milliseconds.
     */
    uint16_t getStep() const;

    /*!
     * Get the number of time samples.
     */
    uint32_t getNumTimes() const;

    /*!
     * Get the length of all samples in milliseconds.
     */
    uint64_t getLength() const;

private:

    //! The start time of the samples.
    iscpx::DateTime m_Start;

    //! The number of time samples.
    uint32_t m_NumTimes;

    //! The duration of a single sample in milliseconds.
    uint16_t m_Step;

}; // class

} // namespace

#endif // ISCPX_TIMEGRID_H
