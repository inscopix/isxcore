#include "isxTime.h"
#include "isxTest.h"
#include "catch.hpp"

TEST_CASE("TimeTest", "[core]") {

    SECTION("valid usage of constructor") {
        //! Tests valid usage of constructor.
        isx::Time time;
        REQUIRE(time.toString() == "19700101-000000.000");
    }

    SECTION("constructing a time with a string") {
        //! Tests constructing a time with a string.
        std::string timeStr = "20151022-110159.293";
        isx::Time time(2015, 10, 22, 11, 1, 59, 293);
        REQUIRE(time.toString() == timeStr);
    }

    SECTION("adding zero seconds to a time") {
        //! Tests adding zero seconds to a time.
        isx::Time time;
        isx::Time newTime = time.addSecs(0);
        REQUIRE(newTime.toString() == "19700101-000000.000");
    }

    SECTION("adding integral seconds to a time") {
        //! Tests adding integral seconds to a time.
        isx::Time time;
        isx::Time newTime = time.addSecs(7);
        REQUIRE(newTime.toString() == "19700101-000007.000");
    }

    SECTION("adding floating point seconds to a time") {
        //! Tests adding floating point seconds to a time.
        isx::Time time;
        isx::Time newTime = time.addSecs(0.07543);
        REQUIRE(newTime.toString(5) == "19700101-000000.07543");
    }

}
