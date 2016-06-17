#include "isxTest.h"
#include "isxPoint.h"
#include "isxRatio.h"
#include "catch.hpp"

TEST_CASE("PointTest", "[core]") {

    SECTION("default constructor with doubles")
    {
        isx::Point<double> actual;
        isx::Point<double> expected(0, 0);
        REQUIRE(actual == expected);
    }

    SECTION("default constructor with rationals")
    {
        isx::Point<isx::Ratio> actual;
        isx::Point<isx::Ratio> expected(0, 0);
        REQUIRE(actual == expected);
    }

    SECTION("default constructor with sizes")
    {
        isx::Point<size_t> actual;
        isx::Point<size_t> expected(0, 0);
        REQUIRE(actual == expected);
    }

    //SECTION("addition of two double points")
    //{
    //    isx::Point<double> point1(8.8, 2.2);
    //    isx::Point<double> point2(2.2, 4.4);
    //    isx::Point<double> actual = point1 + point2;
    //    isx::Point<double> expected(11, 6.6);
    //    REQUIRE(actual == expected);
    //}

    SECTION("addition of two rational points")
    {
        isx::Point<isx::Ratio> point1(isx::Ratio(88, 10), isx::Ratio(22, 10));
        isx::Point<isx::Ratio> point2(isx::Ratio(22, 10), isx::Ratio(44, 10));
        isx::Point<isx::Ratio> actual = point1 + point2;
        isx::Point<isx::Ratio> expected(11, isx::Ratio(66, 10));
        REQUIRE(actual == expected);
    }

    //SECTION("multiplication of a double point and an integral point")
    //{
    //    isx::Point<double> point1(2.2, 4.4);
    //    isx::Point<size_t> point2(1440, 1080);
    //    isx::Point<double> actual = point1 * point2;
    //    isx::Point<double> expected(3168, 4752);
    //    REQUIRE(actual == expected);
    //}

    SECTION("multiplication of a rational point and an integral point")
    {
        isx::Point<isx::Ratio> point1(isx::Ratio(22, 10), isx::Ratio(44, 10));
        isx::Point<size_t> point2(1440, 1080);
        isx::Point<isx::Ratio> actual = point1 * point2;
        isx::Point<isx::Ratio> expected(3168, 4752);
        REQUIRE(actual == expected);
    }

}
