#include "isxTest.h"
#include "isxPoint.h"
#include "catch.hpp"

TEST_CASE("PointTest", "[core]") {

    SECTION("valid usage of constructor") {
        //! Tests valid usage of constructor.
        isx::Point point;
        REQUIRE(point.toString() == "(0.00, 0.00)");
    }

    SECTION("constructing a time with a string") {
        //! Tests constructing a time with a string.
        isx::Point point(9.342, -25);
        REQUIRE(point.toString(3) == "(9.342, -25.000)");
    }

    SECTION("adding another point") {    
        //! Tests adding another point.
        isx::Point point1(5.2, 91);
        isx::Point point2(16, -6.3);
        isx::Point newPoint = point1.plus(point2);
        REQUIRE(newPoint.toString() == "(21.20, 84.70)");
    }

    SECTION("subtracting another point") {
        //! Tests subtracting another point.
        isx::Point point1(5.2, 91);
        isx::Point point2(16, -6.3);
        isx::Point newPoint = point1.minus(point2);
        REQUIRE(newPoint.toString() == "(-10.80, 97.30)");
    }
}
