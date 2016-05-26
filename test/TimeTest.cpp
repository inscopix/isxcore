#include "isxTime.h"
#include "isxTest.h"
#include "catch.hpp"

TEST_CASE("TimeTest", "[core]") {

    SECTION("default constructor")
    {
        isx::Time actual;
        isx::Time expected(1970, 1, 1, 0, 0, 0, 0);
        REQUIRE(actual == expected);
    }

    SECTION("add zero seconds to a time")
    {
        isx::Time expected(1970, 1, 1, 0, 0, 0, 0);
        isx::Time actual = expected.addSecs(0);
        REQUIRE(actual == expected);
    }

    SECTION("add integral seconds to a time")
    {
        isx::Time time(1970, 1, 1, 0, 0, 0, 0);
        isx::Time expected(1970, 1, 1, 0, 0, 7, 0);
        isx::Time actual = time.addSecs(7);
        REQUIRE(actual == expected);
    }

    SECTION("add rational number seconds to a time")
    {
        isx::Time time;
        isx::Ratio offset(531, 1000);
        isx::Time expected(1970, 1, 1, 0, 0, 0, offset);
        isx::Time actual = time.addSecs(offset);
        REQUIRE(actual == expected);
    }

    SECTION("accumulate rational number seconds")
    {
        isx::Ratio stepTime(1, 30);
        uint32_t numTimes = 32;
        isx::Time time;
        isx::Time expected = time.addSecs(stepTime * numTimes);

        isx::Time actual = time;
        for (int t = 0; t < numTimes; ++t)
        {
            actual = actual.addSecs(stepTime);
        }

        REQUIRE(actual == expected);
    }

    SECTION("copy constructor")
    {
        isx::Time time;
        isx::Time otherTime(time);
        REQUIRE(time == otherTime);
    }

    SECTION("copy assignment")
    {
        isx::Time time;
        isx::Time otherTime = time;
        REQUIRE(time == otherTime);
    }

    SECTION("equals operator")
    {
        isx::Time time(1970, 1, 1, 0, 0, 0, 0);
        isx::Time otherTime(1970, 1, 1, 0, 0, 0, 0);
        REQUIRE(time == otherTime);
    }

    SECTION("String conversion")
    {
        isx::Time time(1970, 1, 1, 0, 0, 0, 0);
        std::string expected = "19700101-000000 0 / 1 UTC";
        std::string actual = time.toString();
        REQUIRE(actual == expected);
    }

}
