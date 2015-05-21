#include "TimeGridTest.h"

void TimeGridTest::testConstructor()
{
	QDate date(1970, 1, 1);
	QDateTime start(date);
	qint32 nTimes = 20;
	qint16 step = 50;
	mosaic::TimeGrid timeGrid(start, nTimes, step);
	QVERIFY(timeGrid.getStart() == start);
	QVERIFY(timeGrid.getNTimes() == nTimes);
	QVERIFY(timeGrid.getStep() == step);
}

void TimeGridTest::testFailure()
{
	QVERIFY(5 == 4);
}

QTEST_APPLESS_MAIN(TimeGridTest)

