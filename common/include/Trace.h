#ifndef MOSAIC_TRACE_H
#define MOSAIC_TRACE_H

#include <cstddef>
#include "TimeGrid.h"

namespace mosaic {

/*!
 * A function of time with a discrete domain.
 */
template <class T = float>
class Trace {

public:

    /*!
     * Trace constructor.
     */
    Trace(QDateTime start, qint32 nTimes, qint16 step);

    /*!
     * Time destructor.
     */
    ~Trace();

    /*!
     * Read access to a range value by index.
     *
     * \param   i   The index.
     * \return  The ith range value.
     */
    T getValue(qint32 i) const;

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
    void setValue(qint32 i, T val);

private:

    //! The discrete domain of the function.
    mosaic::TimeGrid domain;

    //! The discrete range of the function.
    T *range;

}; // class

} // namespace

#endif // MOSAIC_TRACE_H
