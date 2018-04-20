#include <math.h>
#include <QTimeZone>
#include <QDateTime>
#include <cmath>

#include "isxAssert.h"
#include "isxTime.h"

namespace isx
{

DurationInSeconds::DurationInSeconds(isize_t num, isize_t den)
    : Ratio(num, den)
{
}

DurationInSeconds::DurationInSeconds(const Ratio & ratio)
    : Ratio(ratio)
{
    ISX_ASSERT(ratio >= 0);
}

DurationInSeconds
DurationInSeconds::fromMilliseconds(const uint64_t inMilliseconds)
{
    return DurationInSeconds(inMilliseconds, 1E3);
}

uint64_t
DurationInSeconds::toMilliseconds() const
{
    return uint64_t(toDouble() * 1E3);
}

DurationInSeconds
DurationInSeconds::fromMicroseconds(const uint64_t inMicroseconds)
{
    return DurationInSeconds(inMicroseconds, 1E6);
}

uint64_t
DurationInSeconds::toMicroseconds() const
{
    return uint64_t(toDouble() * 1E6);
}

const int32_t Time::s_minUtcOffset = -50400;
const int32_t Time::s_maxUtcOffset = 50400;

Time::Time(const DurationInSeconds & secsSinceEpoch, int32_t utcOffset)
: m_secsSinceEpoch(secsSinceEpoch)
, m_utcOffset(utcOffset)
{
    ISX_ASSERT(utcOffset >= s_minUtcOffset || utcOffset <= s_maxUtcOffset);
}

Time::Time( uint16_t year,
            uint8_t mon,
            uint8_t day,
            uint8_t hour,
            uint8_t mins,
            uint8_t secs,
            const DurationInSeconds & secsOffset,
            int32_t utcOffset)
{
    ISX_ASSERT(year >= 1970);
    ISX_ASSERT(mon >= 1 || mon <= 12);
    ISX_ASSERT(day >= 1 || day <= 31);
    ISX_ASSERT(hour <= 23);
    ISX_ASSERT(mins <= 60);
    ISX_ASSERT(secs <= 60);
    ISX_ASSERT(secsOffset >= 0 || secsOffset < 1);
    ISX_ASSERT(utcOffset >= s_minUtcOffset || utcOffset <= s_maxUtcOffset);

    QDate date(year, mon, day);
    ISX_ASSERT(date.isValid());

    QTime time(hour, mins, secs);
    QTimeZone timeZone(utcOffset);
    QDateTime dateTime(date, time, timeZone);

    int64_t secsSinceEpoch = std::floor(dateTime.toMSecsSinceEpoch() / 1000);

    m_secsSinceEpoch = secsOffset + secsSinceEpoch;
    m_utcOffset = utcOffset;
}

DurationInSeconds
Time::getSecsSinceEpoch() const
{
    return m_secsSinceEpoch;
}

Time
Time::operator +(const DurationInSeconds & duration) const
{
    return Time(m_secsSinceEpoch + duration);
}

Time &
Time::operator +=(const DurationInSeconds & duration)
{
    m_secsSinceEpoch += duration;
    return *this;
}

Time
Time::operator -(const DurationInSeconds & duration) const
{
    return Time(m_secsSinceEpoch - duration);
}

Time &
Time::operator -=(const DurationInSeconds & duration)
{
    m_secsSinceEpoch -= duration;
    return *this;
}

Ratio
Time::operator -(const Time & from) const
{
    return m_secsSinceEpoch - from.m_secsSinceEpoch;
}

int32_t
Time::getUtcOffset() const
{
    return m_utcOffset;
}

bool
Time::operator ==(const Time & other) const
{
    return m_secsSinceEpoch == other.m_secsSinceEpoch;
}

bool
Time::operator !=(const Time & other) const
{
    return m_secsSinceEpoch != other.m_secsSinceEpoch;
}

bool
Time::operator <(const Time & other) const
{
    return m_secsSinceEpoch < other.m_secsSinceEpoch;
}

bool
Time::operator <=(const Time & other) const
{
    return m_secsSinceEpoch <= other.m_secsSinceEpoch;
}

bool
Time::operator >(const Time & other) const
{
    return m_secsSinceEpoch > other.m_secsSinceEpoch;
}

bool
Time::operator >=(const Time & other) const
{
    return m_secsSinceEpoch >= other.m_secsSinceEpoch;
}

void
Time::serialize(std::ostream& strm) const
{
    QTimeZone timeZone(m_utcOffset);
    QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(m_secsSinceEpoch.toMilliseconds(), timeZone);

    std::string dateTimeStr = dateTime.toString("yyyy/MM/dd-hh:mm:ss.zzz").toStdString();

    strm << dateTimeStr;
}

std::string
Time::getAsIso8601String() const
{
    double msDouble = m_secsSinceEpoch.toDouble() * 1000.0;
    int64_t msInt = int64_t(floor(msDouble + 0.5));

    auto qdt = QDateTime::fromMSecsSinceEpoch(msInt);
    qdt.setTimeSpec(Qt::OffsetFromUTC);
    qdt.setUtcOffset(m_utcOffset);
    auto qs = qdt.toString(Qt::ISODateWithMs);

    return qs.toStdString();
}

Time
Time::now()
{
    QDateTime nowDateTime = QDateTime::currentDateTime();
    QDate nowDate = nowDateTime.date();
    QTime nowTime = nowDateTime.time();
    const auto secsOffset = DurationInSeconds::fromMilliseconds(nowTime.msec());
    int32_t utcOffset = nowDateTime.timeZone().offsetFromUtc(nowDateTime);
    return Time(
            nowDate.year(),
            nowDate.month(),
            nowDate.day(),
            nowTime.hour(),
            nowTime.minute(),
            nowTime.second(),
            secsOffset,
            utcOffset);
}

Time
Time::floorToDenomOf(const Ratio & inRatio) const
{
    return Time(m_secsSinceEpoch.floorToDenomOf(inRatio), m_utcOffset);
}

} // namespace
