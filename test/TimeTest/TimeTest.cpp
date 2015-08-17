#include "isxTime.h"
#include "isxTest.h"

class TimeTest : public isx::Test {
    Q_OBJECT

private slots:

    //! Tests valid usage of constructor.
    void testConstructor() {
        isx::Time time;
        ISX_COMPARE_STRINGS(time.toString(), "19700101-000000.000");
    }

    //! Tests constructing a time with a string.
    void testConstructorWithString() {
        std::string timeStr = "20151022-110159.293";
        isx::Time time(2015, 10, 22, 11, 1, 59, 293);
        ISX_COMPARE_STRINGS(time.toString(), timeStr);
    }

    //! Tests adding zero seconds to a time.
    void testAddSecsZero() {
        isx::Time time;
        isx::Time newTime = time.addSecs(0);
        ISX_COMPARE_STRINGS(newTime.toString(), "19700101-000000.000");
    }

    //! Tests adding integral seconds to a time.
    void testAddSecsInt() {
        isx::Time time;
        isx::Time newTime = time.addSecs(7);
        ISX_COMPARE_STRINGS(newTime.toString(), "19700101-000007.000");
    }

    //! Tests adding floating point seconds to a time.
    void testAddSecsFloat() {
        isx::Time time;
        isx::Time newTime = time.addSecs(0.07543);
        ISX_COMPARE_STRINGS(newTime.toString(5), "19700101-000000.07543");
    }

};

QTEST_APPLESS_MAIN(TimeTest)
#include "TimeTest.moc"

