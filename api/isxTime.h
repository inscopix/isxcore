#ifndef ISX_TIME_H
#define ISX_TIME_H

#include <cstdint>

#include "isxCore.h"
#include "isxObject.h"
#include "isxRatio.h"

namespace isx
{

/// A temporal duration in seconds that is non-negative.
///
class DurationInSeconds : public Ratio
{

public:

    /// Constructor with positive rational duration in seconds.
    ///
    /// \param  num     Numerator of rational duration.
    /// \param  den     Denominator of rational duration.
    DurationInSeconds(isize_t num = 0, isize_t den = 1);

    /// Fully specified constructor.
    ///
    /// \param  ratio   The duration in seconds.
    DurationInSeconds(const isx::Ratio & ratio);

};

/// An absolute date/time stamp.
///
class Time : public Object
{

public:

    /// Construct a time with seconds sinces the Unix epoch.
    ///
    /// \param   secsSinceEpoch Duration in seconds since the Unix epoch.
    /// \param   utcOffset      Time zone offset from UTC in seconds [-50400, 50400].
    Time(const DurationInSeconds & secsSinceEpoch = 0, int32_t utcOffset = 0);

    /// Fully specified constructor.
    ///
    /// \param   year       The year in the common era [1970, 2^16).
    /// \param   mon        The month of the year in [1-12].
    /// \param   day        The day of the month [1-31].
    /// \param   hour       The hour of the day [0-23].
    /// \param   mins       The minutes past the hour [0-59].
    /// \param   secs       The seconds past the minute [0-59].
    /// \param   secsOffset The sub-second offset in seconds [0-1).
    /// \param   utcOffset  The time zone offset from UTC in seconds [-50400, 50400].
    Time(   uint16_t year,
            uint8_t mon,
            uint8_t day,
            uint8_t hour,
            uint8_t mins,
            uint8_t secs = 0,
            const DurationInSeconds & secsOffset = 0,
            int32_t utcOffset = 0);

    /// \return     The seconds since the epoch.
    ///
    DurationInSeconds getSecsSinceEpoch() const;

    /// \return the utcOffset
    ///
    int32_t
    getUtcOffset() const;

    /// Add a duration in seconds to this and return the result.
    ///
    /// \param   duration   The duration in seconds to add to this.
    /// \return             The result of adding the duration in seconds to this.
    Time operator +(const DurationInSeconds & duration) const;

    /// Add a duration in seconds to this in-place.
    ///
    /// \param   duration   The duration in seconds to add to this.
    Time & operator +=(const DurationInSeconds & duration);

    /// Subtract a duration in seconds from this and return the result.
    ///
    /// \param   duration   The duration in seconds to subtract from this.
    /// \return             The result of subtracting the duration in seconds from this.
    Time operator -(const DurationInSeconds & duration) const;

    /// Subtract a duration in seconds from this in-place.
    ///
    /// \param   duration   The duration in seconds to subtract from this.
    Time & operator -=(const DurationInSeconds & duration);

    /// Find the seconds from one time to this.
    ///
    /// \param   from       The time from which to measure.
    /// \return             The result of adding the duration in seconds to this.
    Ratio operator -(const Time & from) const;

    /// \return     True if this is exactly equal to another time, false otherwise.
    ///
    bool operator ==(const Time & other) const;

    /// \return     True if this is not exactly equal to another time, false otherwise.
    ///
    bool operator !=(const Time & other) const;

    /// \return     True if this is earlier than another time, false other.
    ///
    bool operator <(const Time & other) const;

    /// \return     True if this is earlier than or equal to another time, false other.
    ///
    bool operator <=(const Time & other) const;

    /// \return     True if this is later than another time, false other.
    ///
    bool operator >(const Time & other) const;

    /// \return     True if this is later than or equal to another time, false other.
    ///
    bool operator >=(const Time & other) const;
    
    /// \return     The current time.
    ///
    static Time now();

    /// This method works on the Ratio contained in Time objects. 
    /// \return     A time with its Ratio set to the largest ratio with inRatio's denom
    ///             that is not greater than the other ratio in value
    Time
    floorToDenomOf(const Ratio & inRatio) const;

    // Overrides
    void serialize(std::ostream & strm) const override;

private:

    /// The rational number of seconds since the Unix epoch.
    DurationInSeconds m_secsSinceEpoch;

    /// The time zone offset from UTC in seconds
    int32_t m_utcOffset;

    /// Minimum UTC offset in seconds.
    static const int32_t s_minUtcOffset;

    /// Maximum UTC offset in seconds.
    static const int32_t s_maxUtcOffset;

}; // class

} // namespace

#endif // ISX_TIME_H
