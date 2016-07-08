#include "isxTest.h"
#include "isxTimingInfo.h"
#include "catch.hpp"

TEST_CASE("TimingInfoTest", "[core]")
{
    SECTION("Default constructor")
    {
        isx::TimingInfo timingInfo;
        REQUIRE(timingInfo.getStart() == isx::Time());
        REQUIRE(timingInfo.getStep() == isx::DurationInSeconds(50, 1000));
        REQUIRE(timingInfo.getNumTimes() == 100);
    }

    SECTION("Valid usage of constructor")
    {
        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::isize_t numTimes = 20;
        isx::TimingInfo timingInfo(start, step, numTimes);
        REQUIRE(timingInfo.getStart() == start);
        REQUIRE(timingInfo.getStep() == step);
        REQUIRE(timingInfo.getNumTimes() == numTimes);
    }

    SECTION("Get the end time of the samples")
    {
        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::isize_t numTimes = 20;
        isx::TimingInfo timingInfo(start, step, numTimes);
        isx::Time expected = start + 1;
        REQUIRE(timingInfo.getEnd() == expected);
    }

    SECTION("Get the duration of all samples")
    {
        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::isize_t numTimes = 20;
        isx::TimingInfo timingInfo(start, step, numTimes);
        REQUIRE(timingInfo.getDuration() == 1);
    }

    SECTION("Convert start time to index")
    {
        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::isize_t index = timingInfo.convertTimeToIndex(start);
        REQUIRE(index == 0);
    }

    SECTION("Convert time before the start to index")
    {
        isx::Time start(1970, 1, 1, 0, 0, 10);
        isx::DurationInSeconds step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::Time time = start - 2;
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 0);
    }

    SECTION("Convert time half a sample after start to an index")
    {
        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::Time time = start + isx::DurationInSeconds(25, 1000);
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 0);
    }

    SECTION("Convert time one sample after the start to an index")
    {
        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::Time time = start + isx::DurationInSeconds(50, 1000);
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 1);
    }

    SECTION("Convert time almost one and a half samples after the start to an index")
    {
        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::Time time = start + isx::DurationInSeconds(74, 1000);
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 1);
    }

    SECTION("Convert end time to index")
    {
        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::Time time = timingInfo.getEnd();
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 99);
    }

    SECTION("Convert time after end to index")
    {
        isx::TimingInfo timingInfo;
        isx::Time time = timingInfo.getEnd() + 2;
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 99);
    }

    SECTION("Convert largest time value in a frame to an index")
    {
        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::Time time = start + isx::DurationInSeconds(199, 1000);
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 3);
    }

    SECTION("Convert smallest time value in a frame to an index")
    {
        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::Time time = start + isx::DurationInSeconds(200, 1000);
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 4);
    }

}
