#include "TimeGridTest.h"
#include "iscpxDateTime.h"

void TimeGridTest::testConstructor()
{
	iscpx::DateTime start;
	uint32_t numTimes = 20;
	uint16_t step = 50;
	iscpx::TimeGrid timeGrid(start, numTimes, step);
	QVERIFY(timeGrid.getStart().toString() == start.toString());
	QVERIFY(timeGrid.getNumTimes() == numTimes);
	QVERIFY(timeGrid.getStep() == step);
}

void TimeGridTest::testGetStart()
{
	iscpx::DateTime start;
	iscpx::TimeGrid timeGrid(start, 20, 50);
	QVERIFY(timeGrid.getStart().toString() == start.toString());
}

void TimeGridTest::testGetLength()
{
	iscpx::DateTime start;
	uint32_t numTimes = 20;
	uint16_t step = 50;
	iscpx::TimeGrid timeGrid(start, numTimes, step);
	uint64_t length = numTimes * step;
	QVERIFY(timeGrid.getLength() == length);
}

QTEST_APPLESS_MAIN(TimeGridTest)

