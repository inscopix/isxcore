#include <math.h>
#include "isxTime.h"
#include <QDateTime>
#include <QTimeZone>
#include <stdexcept>

#include <iostream>

namespace isx {

class Time::Impl {

public:

    Impl() {
        QDate epoch(1970, 1, 1);
        QTime midnight(0, 0, 0, 0);
        m_qDateTime = QDateTime(epoch, midnight);
        m_offset = 0.0;
    }

    Impl(   uint16_t year,
            uint8_t mon,
            uint8_t day,
            uint8_t hour,
            uint8_t mins,
            uint8_t secs,
            double offset) {

        if (year < 1970)
            throw std::runtime_error("Year must be in [1970, 2^16).");
        if (mon < 1 || mon > 12)
            throw std::runtime_error("Month must be in [1, 12].");
        if (day < 1 || day > 31)
            throw std::runtime_error("Day must be in [1, 31].");
        if (hour > 23)
            throw std::runtime_error("Hour must be in [0, 23].");
        if (mins > 59)
            throw std::runtime_error("Minutes must be in [0, 59].");
        if (secs > 59)
            throw std::runtime_error("Seconds must be in [0, 59].");
        if (offset < 0 || offset >= 1)
            throw std::runtime_error("Offset must be in [0, 1).");

        QDate date(year, mon, day);

        if (!date.isValid())
            throw std::runtime_error("Date " + date.toString().toStdString() + " is not valid.");

        QTime time(hour, mins, secs);
        m_qDateTime = QDateTime(date, time);
        m_offset = offset;
    }

    Impl(const Impl& other) {
        m_qDateTime = other.m_qDateTime;
        m_offset = other.m_offset;
    }

    Impl& operator =(const Impl& other) {
        m_qDateTime = other.m_qDateTime;
        m_offset = other.m_offset;
        return *this;
    }

    ~Impl() {};

    std::string toString(int msPrec) const {
        QString qStr = m_qDateTime.toString("yyyyMMdd-hhmmss");
        if (msPrec > 0) {
            QString milliSecStr = QString::number(m_offset, 'f', msPrec);
            qStr.append(milliSecStr.right(msPrec + 1));
        }
        return qStr.toStdString();
    }

    isx::Time addSecs(double s) const {
        isx::Time dateTime;
        double sFloored = floor(s);
        dateTime.m_impl->m_qDateTime = m_qDateTime.addSecs(static_cast<qint64>(sFloored));
        dateTime.m_impl->m_offset = m_offset + (s - sFloored);
        return dateTime;
    }

    double secsFrom(const isx::Time& from) const {
        double fromEpochSecs = from.m_impl->m_qDateTime.toMSecsSinceEpoch() / 1000;
        double thisEpochSecs = m_qDateTime.toMSecsSinceEpoch() / 1000;
        double diffEpochSecs = (thisEpochSecs - fromEpochSecs);
        double diffOffsetSecs = m_offset - from.m_impl->m_offset;
        return diffEpochSecs + diffOffsetSecs;
    }

    bool operator ==(const isx::Time::Impl& other) const {
        return m_offset == other.m_offset
            && m_qDateTime == other.m_qDateTime;
    }

    void serialize(std::ostream& strm) const {
        uint8_t prec = strm.precision();
        QString qStr = m_qDateTime.toString("yyyyMMdd-hhmmss");
        if (prec > 0) {
            QString milliSecStr = QString::number(m_offset, 'f', prec);
            qStr.append(milliSecStr.right(prec + 1));
        }
        strm << qStr.toStdString();
    }

private:

    //! The QDateTime used to store the base time up to second precision.
    QDateTime m_qDateTime;

    //! The offset in seconds from the base time in [0, 1).
    double m_offset;
};

Time::Time() {
    m_impl.reset(new Impl());
}

Time::Time( uint16_t year,
            uint8_t mon,
            uint8_t day,
            uint8_t hour,
            uint8_t mins,
            uint8_t secs,
            double ms) {
    m_impl.reset(new Impl(year, mon, day, hour, mins, secs, ms));
}

Time::Time(const Time& other) {
    m_impl.reset(new Impl(*(other.m_impl)));
}

Time&
Time::operator =(const Time& other) {
    m_impl.reset(new Impl(*(other.m_impl)));
    return *this;
}

Time::~Time() {
}

std::string
Time::toString(int msPrec) const {
    return m_impl->toString(msPrec);
}

isx::Time
Time::addSecs(double s) const {
    return m_impl->addSecs(s);
}

double
Time::secsFrom(const isx::Time& from) const {
    return m_impl->secsFrom(from);
}

bool
Time::operator ==(const isx::Time& other) const {
    return *m_impl == *(other.m_impl);
}

void
Time::serialize(std::ostream& strm) const {
    m_impl->serialize(strm);
}

} // namespace
