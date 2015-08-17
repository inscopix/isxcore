#include "isxTest.h"
#include "isxPoint.h"

class PointTest : public isx::Test {
    Q_OBJECT

private slots:

    //! Tests valid usage of constructor.
    void testConstructor() {
        isx::Point point;
        ISX_COMPARE_STRINGS(point.toString(), "(0.00, 0.00)");
    }

    //! Tests constructing a time with a string.
    void testConstructorWithCoords() {
        isx::Point point(9.342, -25);
        ISX_COMPARE_STRINGS(point.toString(3), "(9.342, -25.000)");
    }

    //! Tests adding another point.
    void testPlusPoint() {
        isx::Point point1(5.2, 91);
        isx::Point point2(16, -6.3);
        isx::Point newPoint = point1.plus(point2);
        ISX_COMPARE_STRINGS(newPoint.toString(), "(21.20, 84.70)");
    }

    //! Tests subtracting another point.
    void testMinusPoint() {
        isx::Point point1(5.2, 91);
        isx::Point point2(16, -6.3);
        isx::Point newPoint = point1.minus(point2);
        ISX_COMPARE_STRINGS(newPoint.toString(), "(-10.80, 97.30)");
    }

};

QTEST_APPLESS_MAIN(PointTest)
#include "PointTest.moc"

