#include "TimeTest.h"
#include "isxTime.h"

void TimeTest::testConstructor() {
	isx::Time time;
	ISX_COMPARE_STRINGS(time.toString(), "19700101-000000.000");
}

void TimeTest::testConstructorWithString() {
	std::string timeStr = "20151022-110159.293";
	isx::Time time(timeStr);
	ISX_COMPARE_STRINGS(time.toString(), timeStr);
}

void TimeTest::testAddMSecsZero() {
	isx::Time time;
	isx::Time newTime = time.addMilliSecs(0.0);
	ISX_COMPARE_STRINGS(newTime.toString(), "19700101-000000.000");
}

void TimeTest::testAddMSecsInt() {
	isx::Time time;
	isx::Time newTime = time.addMilliSecs(7.0);
	ISX_COMPARE_STRINGS(newTime.toString(), "19700101-000000.007");
}

void TimeTest::testAddMSecsFloat() {
	isx::Time time;
	isx::Time newTime = time.addMilliSecs(7.543);
	ISX_COMPARE_STRINGS(newTime.toString(4), "19700101-000000.0075");
}

QTEST_APPLESS_MAIN(TimeTest)

