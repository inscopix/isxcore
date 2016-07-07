#include "isxTest.h"
#include "isxSpatialVector.h"
#include "isxRatio.h"
#include "catch.hpp"

TEST_CASE("SpatialVectorTest", "[core]") {

    SECTION("default constructor with rationals")
    {
        isx::SpatialVector<isx::Ratio> actual;
        isx::SpatialVector<isx::Ratio> expected(0, 0);
        REQUIRE(actual == expected);
    }

    SECTION("default constructor with sizes")
    {
        isx::SpatialVector<isx::isize_t> actual;
        isx::SpatialVector<isx::isize_t> expected(0, 0);
        REQUIRE(actual == expected);
    }

    SECTION("empty constructor with signed integers")
    {
        isx::SpatialVector<int64_t> actual;
        isx::SpatialVector<int64_t> expected(0, 0);
        REQUIRE(actual == expected);
    }

    SECTION("addition of two rational vectors")
    {
        isx::SpatialVector<isx::Ratio> point1(isx::Ratio(88, 10), isx::Ratio(22, 10));
        isx::SpatialVector<isx::Ratio> point2(isx::Ratio(22, 10), isx::Ratio(44, 10));
        isx::SpatialVector<isx::Ratio> actual = point1 + point2;
        isx::SpatialVector<isx::Ratio> expected(11, isx::Ratio(66, 10));
        REQUIRE(actual == expected);
    }

    SECTION("multiplication of a rational vector and an integral vector")
    {
        isx::SpatialVector<isx::Ratio> point1(isx::Ratio(22, 10), isx::Ratio(44, 10));
        isx::SpatialVector<isx::isize_t> point2(1440, 1080);
        isx::SpatialVector<isx::Ratio> actual = point1 * point2;
        isx::SpatialVector<isx::Ratio> expected(3168, 4752);
        REQUIRE(actual == expected);
    }

}
