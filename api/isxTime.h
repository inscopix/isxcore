#ifndef ISX_TIME_H
#define ISX_TIME_H

#include <cstdint>

#include "isxObject.h"
#include "isxRatio.h"

namespace isx
{

/// An absolute date/time stamp.
///
class Time : public isx::Object
{

public:

    /// Construct a time with seconds sinces the Unix epoch.
    ///
    /// \param  secsSinceEpoch  Seconds since the Unix epoch as a rational number.
    /// \param  utcOffset       Time zone offset from UTC in seconds [-50400, 50400].
    Time(const isx::Ratio& secsSinceEpoch = 0, int32_t utcOffset = 0);

    /// Fully specified constructor.
    ///
    /// \param   year       The year in the common era [1970, 2^16).
    /// \param   mon        The month of the year in [1-12].
    /// \param   day        The day of the month [1-31].
    /// \param   hour       The hour of the day [0-23].
    /// \param   mins       The minutes past the hour [0-59].
    /// \param   secs       The seconds past the minute [0-59].
    /// \param   secsOffset The rational number offset in seconds [0-1).
    /// \param   utcOffset  The time zone offset from UTC in seconds [-50400, 50400].
    Time(   uint16_t year,
            uint8_t mon,
            uint8_t day,
            uint8_t hour,
            uint8_t mins,
            uint8_t secs = 0,
            const isx::Ratio& secsOffset = 0,
            int32_t utcOffset = 0);

    /// Returns the result of adding a rational number of seconds to this.
    ///
    /// \param   secs   The seconds to add to this.
    /// \return         The result of adding a rational number of secons to this.
    isx::Time addSecs(const isx::Ratio& secs) const;

    /// Returns the number of seconds from another time to this.
    ///
    /// \param   from   The time to start counting from.
    /// \return  The seconds from the given time to this.
    isx::Ratio secsFrom(const isx::Time& from) const;
    
    /// \return the utcOffset
    ///
    int32_t
    getUtcOffset() const;

    /// \return     True if this is exactly equal to another time, false otherwise.
    ///
    bool operator ==(const isx::Time& other) const;

    /// \return     True if this is not exactly equal to another time, false otherwise.
    ///
    bool operator !=(const isx::Time& other) const;

    /// \return     True if this is earlier than another time, false other.
    ///
    bool operator <(const isx::Time& other) const;

    /// \return     True if this is earlier than or equal to another time, false other.
    ///
    bool operator <=(const isx::Time& other) const;

    /// \return     True if this is later than another time, false other.
    ///
    bool operator >(const isx::Time& other) const;

    /// \return     True if this is later than or equal to another time, false other.
    ///
    bool operator >=(const isx::Time& other) const;
    
    /// \return     The current time.
    ///
    static isx::Time now();

    /// This method works on the Ratio contained in Time objects. 
    /// \return     A time with its Ratio set to the largest ratio with inRatio's denom
    ///             that is not greater than the other ratio in value
    Time
    floorToDenomOf(const isx::Ratio & inRatio) const;

    // Overrides
    virtual void serialize(std::ostream& strm) const;

private:

    /// The rational number of seconds since the Unix epoch.
    isx::Ratio m_secsSinceEpoch;

    /// The time zone offset from UTC in seconds
    int32_t m_utcOffset;

}; // class

} // namespace

#endif // ISX_TIME_H
