#include <math.h>
#include "iscpxTime.h"
#include "QTimeZone.h"

namespace iscpx {

Time::Time() {
	QDate epoch(1970, 1, 1);
	QTime midnight(0, 0, 0, 0);
	QTimeZone utc(0);
	m_QDateTime = QDateTime(epoch, midnight, utc);
	m_offset = 0.0;
}

Time::Time(const std::string& str) {
	QString qStr = QString::fromStdString(str);
	QString qFormat("yyyyMMdd-hhmmss.zzz");
	m_QDateTime = QDateTime::fromString(qStr, qFormat);
	m_offset = 0.0;
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

iscpx::Time
Time::addMilliSecs(double ms) const {
	iscpx::Time dateTime;
	double msFloored = floor(ms);
	dateTime.m_QDateTime = m_QDateTime.addMSecs(static_cast<qint64>(msFloored));
	dateTime.m_offset = m_offset + (ms - msFloored);
	return dateTime;
}

double
Time::milliSecsFrom(iscpx::Time from) const {
	double fromEpochMs = from.m_QDateTime.toMSecsSinceEpoch();
	double thisEpochMs = m_QDateTime.toMSecsSinceEpoch();
	double diffEpochMs = thisEpochMs - fromEpochMs;
	double diffOffset = m_offset - from.m_offset;
	return diffEpochMs + diffOffset;
}

} // namespace
