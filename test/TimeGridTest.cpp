#include "isxTest.h"
#include "isxTimeGrid.h"
#include "catch.hpp"

TEST_CASE("TimeGridTest", "[core]") {

    SECTION("valid usage of constructor") {
        //! Tests valid usage of constructor.
        isx::Time start;
        uint32_t numTimes = 20;
        double step = 50;
        isx::TimeGrid timeGrid(start, numTimes, step);
        REQUIRE(timeGrid.getStart().toString() == start.toString());
        REQUIRE(timeGrid.getNumTimes() == numTimes);
        REQUIRE(timeGrid.getStep() == step);
    }

    SECTION("getting the start time") {
        //! Tests getting the start time.
        isx::Time start;
        isx::TimeGrid timeGrid(start, 20, 50);
        REQUIRE(timeGrid.getStart().toString() == start.toString());
    }

    SECTION("getting the length of the samples") {
        //! Test getting the length of the samples.
        isx::Time start;
        uint32_t numTimes = 20;
        double step = 50;
        isx::TimeGrid timeGrid(start, numTimes, step);
        double length = numTimes * step;
        REQUIRE(timeGrid.getLength() == length);
    }
}
