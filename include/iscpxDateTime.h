#ifndef ISCPX_DATETIME_H
#define ISCPX_DATETIME_H

#include <string>
#include <cstdint>
#include <QDateTime>

namespace iscpx
{

/*!
 * A date/time stamp that marks an absolute time in the Gregorian calendar.
 */
class DateTime
{

public:

    /*!
     * DateTime constructor.
     *
     * Initially the date time is set to 1970/01/01 00:00:00.000 UTC.
     */
    DateTime();

    /*!
     * Convert this to a string.
	 *
	 * The format argument determines the format of the output.
	 *
	 * The following expressions may be used for the date.
	 *
	 * - yyyy: year as a four digit number (0001 to 9999)
	 * - MM: month as a two digit number (01 to 12)
	 * - dd: day as a two digit number (01 to 31)
	 *
	 * The following expressions may be used for the time.
	 *
	 * - hh: hour as a two digit number (00 to 23)
	 * - mm: minute as a two digit number (00 to 59)
	 * - ss: second as a two digit number (00 to 59)
	 * - zzz: millisecond as a three digit number (000 to 999)
     *
     * \param   format 	The format of the output string.
     * \return  		The output string.
     */
    std::string toString(const std::string& format = "yyyyMMdd-hhmmsszzz") const;

    /*!
     * Returns a date time with a number of milliseconds added to this.
     *
     * \param   ms      The number of milliseconds to add.
     * \return  A date time s seconds after this.
     */
    iscpx::DateTime addMilliSecs(uint64_t ms) const;

    /*!
     * Create a DateTime object from a string in a format.
     *
     * See toString() for more details about accepted formats.
     *
     * \param   str     The string to create a DateTime from.
     * \param   format  The format of the string.
     * \sa      toString()
     */
    static iscpx::DateTime fromString(const std::string& str, const std::string& format);

private:

    //! The Qt DateTime that handles most of the implementation.
    QDateTime m_QDateTime;

}; // class

} // namespace

#endif // ISCPX_DATETIME_H
