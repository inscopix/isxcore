#ifndef ISX_TIME_H
#define ISX_TIME_H

#include <string>
#include <cstdint>
#include <QDateTime>
#include <memory>

namespace isx {

/*!
 * An absolute time stamp to floating point precision in the Gregorian calendar.
 */
class Time {

public:

    /*!
     * Default constructor.
     *
     * Initially the date and time is set to 1970/01/01 00:00:00.000 UTC.
     */
    Time();

    /*!
     * Fully specified constructor.
     *
     * \param   year    The year in the common era [1970, 2^16].
     * \param   mon     The month of the year in [1-12].
     * \param   day     The day of the month [1-31].
     * \param   hour    The hour of the day [0-23].
     * \param   mins    The minutes past the hour [0-59].
     * \param   secs    The seconds past the minute [0-59].
     * \param   ms      The milliseconds past the second [0-1000).
     * \param   utcOff  The time zone in hours offset from UTC [-14, 14].
     */
    Time(   uint16_t year,
            uint8_t mon,
            uint8_t day,
            uint8_t hour,
            uint8_t mins,
            uint8_t secs = 0,
            double ms = 0,
            int8_t utcOff = 0);

    /*!
     * Convert this to a string of the form "YYYYMMDD-hhmmss(.z*)".
	 *
     * \param   msPrec  The number of millisecond digits.
     * \return  The output string.
     */
    std::string toString(int msPrec = 3) const;

    /*!
     * Returns a time with the given seconds added to this.
     *
     * \param   s       The seconds to add to this.
     * \return  A time s seconds after this.
     */
    isx::Time addSecs(double s) const;

    /*!
     * Returns the seconds from the given time to this.
     *
     * \param   from    The time from which to calculate.
     * \return  The seconds from the given time to this.
     */
    double secsFrom(isx::Time from) const;

    /*!
     * \return  The current time.
     */
    static std::unique_ptr<isx::Time> now();

private:

    //! The Qt DateTime that stores the base time up to second precision.
    QDateTime m_QDateTime;

    //! The floating point offset in seconds from the base time in [0, 1).
    double m_offset;

}; // class

} // namespace

#endif // ISX_TIME_H
