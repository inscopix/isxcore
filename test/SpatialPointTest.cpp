#include "isxTest.h"
#include "isxSpatialPoint.h"
#include "isxRatio.h"
#include "catch.hpp"

TEST_CASE("SpatialPointTest", "[core]") {

    SECTION("default constructor with rationals")
    {
        isx::SpatialPoint<isx::Ratio> actual;
        isx::SpatialPoint<isx::Ratio> expected(0, 0);
        REQUIRE(actual == expected);
    }

    SECTION("default constructor with sizes")
    {
        isx::SpatialPoint<isx::isize_t> actual;
        isx::SpatialPoint<isx::isize_t> expected(0, 0);
        REQUIRE(actual == expected);
    }

    SECTION("add a rational vector to a rational point")
    {
        isx::SpatialPoint<isx::Ratio> point(isx::Ratio(88, 10), isx::Ratio(22, 10));
        isx::SpatialVector<isx::Ratio> vector(isx::Ratio(22, 10), isx::Ratio(44, 10));
        isx::SpatialPoint<isx::Ratio> actual = point + vector;
        isx::SpatialPoint<isx::Ratio> expected(11, isx::Ratio(66, 10));
        REQUIRE(actual == expected);
    }

    SECTION("subtract one ration point from another to get a rational vector")
    {
        isx::SpatialPoint<isx::Ratio> point1(isx::Ratio(88, 10), isx::Ratio(22, 10));
        isx::SpatialPoint<isx::Ratio> point2(isx::Ratio(22, 10), isx::Ratio(66, 10));
        isx::SpatialVector<isx::Ratio> actual = point1 - point2;
        isx::SpatialVector<isx::Ratio> expected(isx::Ratio(66, 10), isx::Ratio(-44, 10));
        REQUIRE(actual == expected);
    }

}
