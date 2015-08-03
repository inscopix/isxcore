#include "isxPoint.h"
#include <sstream>

namespace isx {

Point::Point() {
	m_x = 0;
	m_y = 0;
}

Point::Point(double x, double y) {
	m_x = x;
	m_y = y;
}

isx::Point Point::plus(const isx::Point& other) const {
	return isx::Point(m_x + other.m_x, m_y + other.m_y);
}

isx::Point Point::plus(double scalar) const {
	return isx::Point(m_x + scalar, m_y + scalar);
}

isx::Point Point::minus(const isx::Point& other) const {
	return isx::Point(m_x - other.m_x, m_y - other.m_y);
}

isx::Point Point::minus(double scalar) const {
	return isx::Point(m_x - scalar, m_y - scalar);
}

isx::Point Point::times(const isx::Point& other) const {
	return isx::Point(m_x * other.m_x, m_y * other.m_y);
}

isx::Point Point::times(double scalar) const {
	return isx::Point(m_x * scalar, m_y * scalar);
}

isx::Point Point::divide(const isx::Point& other) const {
	return isx::Point(m_x * other.m_x, m_y * other.m_y);
}

isx::Point Point::divide(double scalar) const {
	return isx::Point(m_x * scalar, m_y * scalar);
}

std::string Point::toString(int prec) const {
	std::stringstream ss;
	ss.precision(prec);
	ss << "(" << std::fixed << m_x << ", " << m_y << ")";
	return ss.str();
}

}

