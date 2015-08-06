#include "TimeGridTest.h"
#include "isxTimeGrid.h"

void TimeGridTest::testConstructor() {
	isx::Time start;
	uint32_t numTimes = 20;
	double step = 50;
	isx::TimeGrid timeGrid(start, numTimes, step);
	ISX_COMPARE_STRINGS(timeGrid.getStart().toString(), start.toString());
	QCOMPARE(timeGrid.getNumTimes(), numTimes);
	QCOMPARE(timeGrid.getStep(), step);
}

void TimeGridTest::testGetStart() {
	isx::Time start;
	isx::TimeGrid timeGrid(start, 20, 50);
	ISX_COMPARE_STRINGS(timeGrid.getStart().toString(), start.toString());
}

void TimeGridTest::testGetLength() {
	isx::Time start;
	uint32_t numTimes = 20;
	double step = 50;
	isx::TimeGrid timeGrid(start, numTimes, step);
	double length = numTimes * step;
	QCOMPARE(timeGrid.getLength(), length);
}

QTEST_APPLESS_MAIN(TimeGridTest)

