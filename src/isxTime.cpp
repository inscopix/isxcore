#include <math.h>
#include "isxTime.h"
#include <QTimeZone>

namespace isx {

Time::Time() {
	QDate epoch(1970, 1, 1);
	QTime midnight(0, 0, 0, 0);
	QTimeZone utc(0);
	m_QDateTime = QDateTime(epoch, midnight, utc);
	m_offset = 0.0;
}

Time::Time(uint16_t year, uint8_t mon, uint8_t day, uint8_t hour, uint8_t mins, uint8_t secs, double mSecs, int8_t utcOff) {
	if (year < 1970)
		throw std::domain_error("Year must be in [1970, 2^16).");
	if (mon < 1 || mon > 12)
		throw std::domain_error("Month must be in [1, 12].");
	if (day < 1 || day > 31)
		throw std::domain_error("Day must be in [1, 31].");
	if (hour > 23)
		throw std::domain_error("Hour must be in [0, 23].");
	if (mins > 59)
		throw std::domain_error("Minutes must be in [0, 59].");
	if (secs > 59)
		throw std::domain_error("Seconds must be in [0, 59].");
	if (mSecs < 0 || mSecs >= 1000)
		throw std::domain_error("Milliseconds must be in [0, 1000).");
	if (utcOff < -14 || utcOff > 14)
		throw std::domain_error("UTC offset must be in [-14, 14].");

	QDate date(year, mon, day);

	if (!date.isValid())
		throw std::invalid_argument("Date " + date.toString().toStdString() + " is not valid.");

	QTime time(hour, mins, secs);
	QTimeZone zone(utcOff);
	m_QDateTime = QDateTime(date, time, zone);
	m_offset = mSecs / 1000.0;
}

std::string
Time::toString(int msPrec) const {
	QString qStr = m_QDateTime.toString("yyyyMMdd-hhmmss");
	if (msPrec > 0) {
		QString milliSecStr = QString::number(m_offset, 'f', msPrec);
		qStr.append(milliSecStr.right(msPrec + 1));
	}
	return qStr.toStdString();
}

isx::Time
Time::addSecs(double s) const {
	isx::Time dateTime;
	double sFloored = floor(s);
	dateTime.m_QDateTime = m_QDateTime.addSecs(static_cast<qint64>(sFloored));
	dateTime.m_offset = m_offset + (s - sFloored);
	return dateTime;
}

double
Time::secsFrom(isx::Time from) const {
	double fromEpochSecs = from.m_QDateTime.toMSecsSinceEpoch() / 1000;
	double thisEpochSecs = m_QDateTime.toMSecsSinceEpoch() / 1000;
	double diffEpochSecs = (thisEpochSecs - fromEpochSecs);
	double diffOffsetSecs = m_offset - from.m_offset;
	return diffEpochSecs + diffOffsetSecs;
}

} // namespace
