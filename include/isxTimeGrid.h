#ifndef ISX_TIMEGRID_H
#define ISX_TIMEGRID_H

#include <cinttypes>
#include "isxTime.h"

namespace isx {

/*!
 * A regularly spaced sample of time points.
 */
class TimeGrid {

public:

    /*
     * Default constructor.
     */
    TimeGrid();

    /*!
     * \param start	    The start time of the samples.
     * \param numTimes	The number of samples.
     * \param step	    The step time of the samples.
     */
    TimeGrid(isx::Time start, uint32_t numTimes, double step);

    /*!
     * Get the start time of the samples.
     */
    isx::Time getStart() const;

    /*!
     * Get the end time of the samples.
     */
    isx::Time getEnd() const;

    /*!
     * Get the duration of a single sample in seconds.
     */
    double getStep() const;

    /*!
     * Get the number of time samples.
     */
    uint32_t getNumTimes() const;

    /*!
     * Get the length of all samples in seconds.
     */
    double getLength() const;

private:

    //! The start time of the samples.
    isx::Time m_Start;

    //! The number of time samples.
    uint32_t m_NumTimes;

    //! The duration of a single sample in milliseconds.
    double m_Step;

}; // class

} // namespace

#endif // ISX_TIMEGRID_H
