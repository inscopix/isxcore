#include "iscpxDateTime.h"

namespace iscpx {

DateTime::DateTime() {
	QDate epoch(1970, 1, 1);
	m_QDateTime = QDateTime(epoch).toUTC();
}

std::string
DateTime::toString(const std::string& format) const {
	QString qFormat = QString::fromStdString(format);
	QString qStr = m_QDateTime.toString(qFormat);
	return qStr.toStdString();
}

iscpx::DateTime
DateTime::addMilliSecs(uint64_t ms) const {
	iscpx::DateTime dateTime;
	dateTime.m_QDateTime = m_QDateTime.addMSecs(ms);
	return dateTime;
}

iscpx::DateTime
DateTime::fromString(const std::string& str, const std::string& format) {
	iscpx::DateTime dateTime;
	QString qStr = QString::fromStdString(str);
	QString qFormat = QString::fromStdString(format);
	dateTime.m_QDateTime = QDateTime::fromString(qStr, qFormat);
	return dateTime;
}

} // namespace
