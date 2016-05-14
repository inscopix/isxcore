#include "isxTime.h"
#include "isxTest.h"
#include "catch.hpp"

TEST_CASE("TimeTest", "[core]") {

    SECTION("default constructor") {
        isx::Time actual;
        isx::Time expected(1970, 1, 1, 0, 0, 0, 0);
        REQUIRE(actual == expected);
    }

    SECTION("add zero seconds to a time") {
        isx::Time expected(1970, 1, 1, 0, 0, 0, 0);
        isx::Time actual = expected.addSecs(0);
        REQUIRE(actual == expected);
    }

    SECTION("add integral seconds to a time") {
        isx::Time time(1970, 1, 1, 0, 0, 0, 0);
        isx::Time expected(1970, 1, 1, 0, 0, 7, 0);
        isx::Time actual = time.addSecs(7);
        REQUIRE(actual == expected);
    }

    SECTION("add floating point seconds to a time") {
        isx::Time time;
        isx::Time expected(1970, 1, 1, 0, 0, 0, 0.531);
        isx::Time actual = time.addSecs(0.531);
        REQUIRE(actual == expected);
    }

    SECTION("copy constructor") {
        isx::Time time;
        isx::Time otherTime(time);
        REQUIRE(time == otherTime);
    }

    SECTION("copy assignment") {
        isx::Time time;
        isx::Time otherTime = time;
        REQUIRE(time == otherTime);
    }

    SECTION("equals operator") {
        isx::Time time(1970, 1, 1, 0, 0, 0, 0);
        isx::Time otherTime(1970, 1, 1, 0, 0, 0, 0);
        REQUIRE(time == otherTime);
    }

}
