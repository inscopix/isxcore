#ifndef ISX_TIME_H
#define ISX_TIME_H

#include <string>
#include <cstdint>
#include <memory>
#include "isxObject.h"

namespace isx {

/*!
 * An absolute time stamp to floating point precision in the Gregorian calendar.
 */
class Time : public isx::Object {

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
     * \param   offset  The floating point offset in seconds [0-1).
     */
    Time(   uint16_t year,
            uint8_t mon,
            uint8_t day,
            uint8_t hour,
            uint8_t mins,
            uint8_t secs = 0,
            double offset = 0);

    /*!
     * Copy constructor.
     */
    Time(const Time& other);

    /*!
     * Copy assignment.
     */
    Time& operator =(const Time& other);

    /*!
     * Destructor.
     */
    ~Time();

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
    double secsFrom(const isx::Time& from) const;

    /*!
     * \return  True if this is exactly equal to other, false otherwise.
     */
    bool operator ==(const isx::Time& other) const;

    // Overrides
    virtual void serialize(std::ostream& strm) const;

private:

    //! The actual implementation depends on Qt and is hidden here.
    class Impl;
    std::unique_ptr<Impl> m_impl;

}; // class

} // namespace

#endif // ISX_TIME_H
