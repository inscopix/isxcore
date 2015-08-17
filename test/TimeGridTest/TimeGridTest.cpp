#include "isxTest.h"
#include "isxTimeGrid.h"

class TimeGridTest : public isx::Test {
    Q_OBJECT

private slots:

    //! Tests valid usage of constructor.
    void testConstructor() {
        isx::Time start;
        uint32_t numTimes = 20;
        double step = 50;
        isx::TimeGrid timeGrid(start, numTimes, step);
        ISX_COMPARE_STRINGS(timeGrid.getStart().toString(), start.toString());
        QCOMPARE(timeGrid.getNumTimes(), numTimes);
        QCOMPARE(timeGrid.getStep(), step);
    }

    //! Tests getting the start time.
    void testGetStart() {
        isx::Time start;
        isx::TimeGrid timeGrid(start, 20, 50);
        ISX_COMPARE_STRINGS(timeGrid.getStart().toString(), start.toString());
    }

    //! Test getting the length of the samples.
    void testGetLength() {
        isx::Time start;
        uint32_t numTimes = 20;
        double step = 50;
        isx::TimeGrid timeGrid(start, numTimes, step);
        double length = numTimes * step;
        QCOMPARE(timeGrid.getLength(), length);
    }

};

QTEST_APPLESS_MAIN(TimeGridTest)
#include "TimeGridTest.moc"

