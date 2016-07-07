#include "isxTest.h"
#include "isxSpatialSize.h"
#include "isxRatio.h"
#include "catch.hpp"

TEST_CASE("SpatialSizeTest", "[core]") {

    SECTION("default constructor with rationals")
    {
        isx::SpatialSize<isx::Ratio> actual;
        isx::SpatialSize<isx::Ratio> expected(1, 1);
        REQUIRE(actual == expected);
    }

    SECTION("default constructor with sizes")
    {
        isx::SpatialSize<isx::isize_t> actual;
        isx::SpatialSize<isx::isize_t> expected(1, 1);
        REQUIRE(actual == expected);
    }

}
