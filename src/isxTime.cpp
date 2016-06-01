#include <math.h>
#include <QTimeZone>
#include <QDateTime>
#include <stdexcept>

#include "isxTime.h"

namespace isx
{

Time::Time(const isx::Ratio& secsSinceEpoch, int32_t utcOffset)
: m_secsSinceEpoch(secsSinceEpoch)
, m_utcOffset(utcOffset)
{
    isx::Time::verifyUtcOffset(utcOffset);
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
    if (year < 1970)
    {
        throw std::runtime_error("Year must be in [1970, 2^16).");
    }
    if (mon < 1 || mon > 12)
    {
        throw std::runtime_error("Month must be in [1, 12].");
    }
    if (day < 1 || day > 31)
    {
        throw std::runtime_error("Day must be in [1, 31].");
    }
    if (hour > 23)
    {
        throw std::runtime_error("Hour must be in [0, 23].");
    }
    if (mins > 59)
    {
        throw std::runtime_error("Minutes must be in [0, 59].");
    }
    if (secs > 59)
    {
        throw std::runtime_error("Seconds must be in [0, 59].");
    }

    isx::Time::verifyUtcOffset(utcOffset);

    QDate date(year, mon, day);
    if (!date.isValid())
    {
        throw std::runtime_error("Date " + date.toString().toStdString() + " is not valid.");
    }

    QTime time(hour, mins, secs);
    QTimeZone timeZone(utcOffset);
    QDateTime dateTime(date, time, timeZone);

    int64_t secsSinceEpoch = dateTime.toMSecsSinceEpoch() / 1000;

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

bool
Time::operator ==(const isx::Time& other) const
{
    return this->m_secsSinceEpoch == other.m_secsSinceEpoch;
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

void
Time::verifyUtcOffset(int32_t utcOffset)
{
    if (utcOffset < -50400 || utcOffset > 50400)
    {
        throw std::runtime_error("UTC offset must be in [-50400, 50400].");
    }
}

} // namespace
