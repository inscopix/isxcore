#include "TimeGrid.h"

namespace mosaic {

TimeGrid::TimeGrid(QDateTime start, qint32 nTimes, qint16 step) {
	this->start = start;
	this->nTimes = nTimes;
	this->step = step;
}

TimeGrid::~TimeGrid() {

}

QDateTime TimeGrid::getStart() const {
	return this->start;
}

QDateTime TimeGrid::getEnd() const {
	return this->start.addMSecs(this->getLength());
}

qint16 TimeGrid::getStep() const {
	return this->step;
}

qint32 TimeGrid::getNTimes() const {
	return this->nTimes;
}

qint64 TimeGrid::getLength() const {
	return this->nTimes * this->step;
}

}
