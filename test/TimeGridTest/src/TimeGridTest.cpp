#include "TimeGridTest.h"
#include "iscpxTimeGrid.h"

void TimeGridTest::testConstructor() {
	iscpx::Time start;
	uint32_t numTimes = 20;
	uint16_t step = 50;
	iscpx::TimeGrid timeGrid(start, numTimes, step);
	ISCPX_COMPARE_STRINGS(timeGrid.getStart().toString(), start.toString());
	QCOMPARE(timeGrid.getNumTimes(), numTimes);
	QCOMPARE(timeGrid.getStep(), step);
}

void TimeGridTest::testGetStart() {
	iscpx::Time start;
	iscpx::TimeGrid timeGrid(start, 20, 50);
	ISCPX_COMPARE_STRINGS(timeGrid.getStart().toString(), start.toString());
}

void TimeGridTest::testGetLength() {
	iscpx::Time start;
	uint32_t numTimes = 20;
	uint16_t step = 50;
	iscpx::TimeGrid timeGrid(start, numTimes, step);
	uint64_t length = numTimes * step;
	QCOMPARE(timeGrid.getLength(), length);
}

QTEST_APPLESS_MAIN(TimeGridTest)

