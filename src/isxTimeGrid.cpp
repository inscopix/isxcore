#include "isxTimeGrid.h"

namespace isx {

TimeGrid::TimeGrid() {
	m_Start = isx::Time();
	m_NumTimes = 100;
	m_Step = 0.05;
}

TimeGrid::TimeGrid(isx::Time start, uint32_t numTimes, double step) {
	m_Start = start;
	m_NumTimes = numTimes;
	m_Step = step;
}

isx::Time
TimeGrid::getStart() const {
	return m_Start;
}

isx::Time
TimeGrid::getEnd() const {
	return m_Start.addSecs(this->getLength());
}

double
TimeGrid::getStep() const {
	return m_Step;
}

uint32_t
TimeGrid::getNumTimes() const {
	return m_NumTimes;
}

double
TimeGrid::getLength() const {
	return m_NumTimes * m_Step;
}

} // namespace
