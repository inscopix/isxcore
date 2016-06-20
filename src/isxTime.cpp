#include <math.h>
#include <QTimeZone>
#include <QDateTime>
#include <cmath>

#include "isxAssert.h"
#include "isxTime.h"

// The minimum and maximum UTC offsets in seconds.
#define MIN_UTC_OFFSET -50400
#define MAX_UTC_OFFSET 50400

namespace isx
{

Time::Time(const isx::Ratio& secsSinceEpoch, int32_t utcOffset)
: m_secsSinceEpoch(secsSinceEpoch)
, m_utcOffset(utcOffset)
{
    ISX_ASSERT(secsSinceEpoch >= 0);
    ISX_ASSERT(utcOffset >= MIN_UTC_OFFSET || utcOffset <= MAX_UTC_OFFSET);
}

Time::Time( uint16_t year,
            uint8_t mon,
            uint8_t day,
            uint8_t hour,
            uint8_t mins,
            uint8_t secs,
            const isx::Ratio& secsOffset,
            int32_t utcOffset)
{
    ISX_ASSERT(year >= 1970);
    ISX_ASSERT(mon >= 1 || mon <= 12);
    ISX_ASSERT(day >= 1 || day <= 31);
    ISX_ASSERT(hour <= 23);
    ISX_ASSERT(mins <= 60);
    ISX_ASSERT(secs <= 60);
    ISX_ASSERT(secsOffset >= 0 || secsOffset < 1);
    ISX_ASSERT(utcOffset >= MIN_UTC_OFFSET || utcOffset <= MAX_UTC_OFFSET);

    QDate date(year, mon, day);
    ISX_ASSERT(date.isValid());

    QTime time(hour, mins, secs);
    QTimeZone timeZone(utcOffset);
    QDateTime dateTime(date, time, timeZone);

    int64_t secsSinceEpoch = std::floor(dateTime.toMSecsSinceEpoch() / 1000);

    m_secsSinceEpoch = secsOffset + secsSinceEpoch;
    m_utcOffset = utcOffset;
}

isx::Time
Time::addSecs(const isx::Ratio& secs) const
{
    isx::Ratio secsSinceEpoch = m_secsSinceEpoch + secs;
    return isx::Time(secsSinceEpoch);
}

isx::Ratio
Time::secsFrom(const isx::Time& from) const
{
    return m_secsSinceEpoch - from.m_secsSinceEpoch;
}

int32_t
Time::getUtcOffset() const
{
    return m_utcOffset;
}
    
bool
Time::operator ==(const isx::Time& other) const
{
    return m_secsSinceEpoch == other.m_secsSinceEpoch;
}
    
bool
Time::operator !=(const isx::Time& other) const
{
    return m_secsSinceEpoch != other.m_secsSinceEpoch;
}

bool
Time::operator <(const isx::Time& other) const
{
    return m_secsSinceEpoch < other.m_secsSinceEpoch;
}

bool
Time::operator <=(const isx::Time& other) const
{
    return m_secsSinceEpoch <= other.m_secsSinceEpoch;
}

bool
Time::operator >(const isx::Time& other) const
{
    return m_secsSinceEpoch > other.m_secsSinceEpoch;
}
    
bool
Time::operator >=(const isx::Time& other) const
{
    return m_secsSinceEpoch >= other.m_secsSinceEpoch;
}
    
void
Time::serialize(std::ostream& strm) const
{
    double secsDouble = m_secsSinceEpoch.toDouble();
    int64_t secsInt = floor(secsDouble);

    QTimeZone timeZone(m_utcOffset);
    QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(secsInt * 1000, timeZone);

    std::string dateTimeStr = dateTime.toString("yyyyMMdd-hhmmss").toStdString();
    isx::Ratio secsOffset = m_secsSinceEpoch - secsInt;
    std::string timeZoneStr = timeZone.displayName(dateTime).toStdString();

    strm << dateTimeStr << " " << secsOffset << " " << timeZoneStr;
}

isx::Time
Time::now()
{
    QDateTime nowDateTime = QDateTime::currentDateTime();
    QDate nowDate = nowDateTime.date();
    QTime nowTime = nowDateTime.time();
    isx::Ratio secsOffset(nowTime.msec(), 1000);
    int32_t utcOffset = nowDateTime.timeZone().offsetFromUtc(nowDateTime);
    return isx::Time(
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
Time::floorToDenomOf(const isx::Ratio & inRatio) const
{
    return Time(m_secsSinceEpoch.floorToDenomOf(inRatio), m_utcOffset);
}

} // namespace
