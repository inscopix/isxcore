#ifndef MOSAIC_TIMEGRID_H
#define MOSAIC_TIMEGRID_H

#include <cstddef>
#include <QDateTime>

namespace mosaic {

/*!
 * A regularly spaced sample of time points.
 */
class TimeGrid {

public:

    /*!
     * TimeGrid constructor.
     *
     * \param nTimes	The number of samples.
     * \param start	The start time of the samples.
     * \param step	The step time of the samples.
     */
    TimeGrid(QDateTime start, qint32 nTimes, qint16 step);

    /*!
     * TimeGrid destructor.
     */
    ~TimeGrid();

    /*!
     * Get the start time of the samples.
     */
    QDateTime getStart() const;

    /*!
     * Get the end time of the samples.
     */
    QDateTime getEnd() const;

    /*!
     * Get the duration of a single sample in milliseconds.
     */
    qint16 getStep() const;

    /*!
     * Get the number of time samples.
     */
    qint32 getNTimes() const;

    /*!
     * Get the length of all samples in milliseconds.
     */
    qint64 getLength() const;

private:

    //! The start time of the samples.
    QDateTime start;

    //! The number of time samples.
    qint32 nTimes;

    //! The duration of a single sample in milliseconds.
    qint16 step;

}; // class

} // namespace

#endif // MOSAIC_TIMEGRID_H
