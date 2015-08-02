#include "isxTimeGrid.h"

namespace isx {

TimeGrid::TimeGrid(isx::Time start, uint32_t numTimes, uint16_t step) {
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
	return m_Start.addMilliSecs(this->getLength());
}

uint16_t
TimeGrid::getStep() const {
	return m_Step;
}

uint32_t
TimeGrid::getNumTimes() const {
	return m_NumTimes;
}

uint64_t
TimeGrid::getLength() const {
	return m_NumTimes * m_Step;
}

} // namespace
