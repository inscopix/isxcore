#ifndef ISX_TIME_H
#define ISX_TIME_H

#include <string>
#include <cstdint>
#include <QDateTime>

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
     * Constructor with a string in format "YYYYMMDD-hhmmss.zzz"
     *
     * \param   str     The string to create a Time from.
     */
    Time(const std::string& str);

    /*!
     * Convert this to a string of the form "YYYYMMDD-hhmmss(.z*)".
	 *
     * \param   msPrec  The number of millisecond digits.
     * \return  The output string.
     */
    std::string toString(int msPrec = 3) const;

    /*!
     * Returns a time with a number of milliseconds added to this.
     *
     * \param   ms      The number of milliseconds to add.
     * \return  A date time ms milliseconds after this.
     */
    isx::Time addMilliSecs(double ms) const;

    /*!
     * Returns the milliseconds from the given time to this.
     *
     * \param   from    The time to calculate milliseconds from.
     * \return  The milliseconds from the given time to this.
     */
    double milliSecsFrom(isx::Time from) const;

private:

    //! The Qt DateTime that stores the base time up to second precision.
    QDateTime m_QDateTime;

    //! The floating point offset in seconds from the base time in [0, 1).
    double m_offset;

}; // class

} // namespace

#endif // ISX_TIME_H
