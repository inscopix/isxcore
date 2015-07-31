#include "TimeTest.h"
#include "iscpxTime.h"

void TimeTest::testConstructor() {
	iscpx::Time time;
	ISCPX_COMPARE_STRINGS(time.toString(), "19700101-000000.000");
}

void TimeTest::testConstructorWithString() {
	std::string timeStr = "20151022-110159.293";
	iscpx::Time time(timeStr);
	ISCPX_COMPARE_STRINGS(time.toString(), timeStr);
}

void TimeTest::testAddMSecsZero() {
	iscpx::Time time;
	iscpx::Time newTime = time.addMilliSecs(0.0);
	ISCPX_COMPARE_STRINGS(newTime.toString(), "19700101-000000.000");
}

void TimeTest::testAddMSecsInt() {
	iscpx::Time time;
	iscpx::Time newTime = time.addMilliSecs(7.0);
	ISCPX_COMPARE_STRINGS(newTime.toString(), "19700101-000000.007");
}

void TimeTest::testAddMSecsFloat() {
	iscpx::Time time;
	iscpx::Time newTime = time.addMilliSecs(7.543);
	ISCPX_COMPARE_STRINGS(newTime.toString(4), "19700101-000000.0075");
}

QTEST_APPLESS_MAIN(TimeTest)

