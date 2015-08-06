#include "TimeTest.h"
#include "isxTime.h"

void TimeTest::testConstructor() {
	isx::Time time;
	ISX_COMPARE_STRINGS(time.toString(), "19700101-000000.000");
}

void TimeTest::testConstructorWithString() {
	std::string timeStr = "20151022-110159.293";
	isx::Time time(2015, 10, 22, 11, 1, 59, 293);
	ISX_COMPARE_STRINGS(time.toString(), timeStr);
}

void TimeTest::testAddSecsZero() {
	isx::Time time;
	isx::Time newTime = time.addSecs(0);
	ISX_COMPARE_STRINGS(newTime.toString(), "19700101-000000.000");
}

void TimeTest::testAddSecsInt() {
	isx::Time time;
	isx::Time newTime = time.addSecs(7);
	ISX_COMPARE_STRINGS(newTime.toString(), "19700101-000007.000");
}

void TimeTest::testAddSecsFloat() {
	isx::Time time;
	isx::Time newTime = time.addSecs(0.07543);
	ISX_COMPARE_STRINGS(newTime.toString(5), "19700101-000000.07543");
}

QTEST_APPLESS_MAIN(TimeTest)

