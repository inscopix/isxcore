#include "isxRatio.h"
#include "isxTest.h"
#include "isxTimingInfo.h"
#include "catch.hpp"

TEST_CASE("TimingInfoTest", "[core]")
{
    SECTION("Default constructor")
    {
        isx::TimingInfo timingInfo;
        REQUIRE(timingInfo.getStart() == isx::Time());
        REQUIRE(timingInfo.getStep() == isx::Ratio(50, 1000));
        REQUIRE(timingInfo.getNumTimes() == 100);
    }

    SECTION("Valid usage of constructor")
    {
        isx::Time start;
        isx::Ratio step(50, 1000);
        uint32_t numTimes = 20;
        isx::TimingInfo timingInfo(start, step, numTimes);
        REQUIRE(timingInfo.getStart() == start);
        REQUIRE(timingInfo.getStep() == step);
        REQUIRE(timingInfo.getNumTimes() == numTimes);
    }

    SECTION("Get the end time of the samples")
    {
        isx::Time start;
        isx::Ratio step(50, 1000);
        uint32_t numTimes = 20;
        isx::TimingInfo timingInfo(start, step, numTimes);
        isx::Time expected = start.addSecs(1);
        REQUIRE(timingInfo.getEnd() == expected);
    }

    SECTION("Get the duration of all samples")
    {
        isx::Time start;
        isx::Ratio step(50, 1000);
        uint32_t numTimes = 20;
        isx::TimingInfo timingInfo(start, step, numTimes);
        REQUIRE(timingInfo.getDuration() == 1);
    }

    SECTION("Convert start time to index")
    {
        isx::Time start;
        isx::Ratio step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        uint32_t index = timingInfo.convertTimeToIndex(start);
        REQUIRE(index == 0);
    }

    SECTION("Convert time before the start to index")
    {
        isx::Time start;
        isx::Ratio step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::Time time = start.addSecs(-2);
        uint32_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 0);
    }

    SECTION("Convert time half a sample after start to an index")
    {
        isx::Time start;
        isx::Ratio step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::Time time = start.addSecs(isx::Ratio(25, 1000));
        uint32_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 0);
    }

    SECTION("Convert time one sample after the start to an index")
    {
        isx::Time start;
        isx::Ratio step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::Time time = start.addSecs(isx::Ratio(50, 1000));
        uint32_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 1);
    }

    SECTION("Convert time almost one and a half samples after the start to an index")
    {
        isx::Time start;
        isx::Ratio step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::Time time = start.addSecs(isx::Ratio(74, 1000));
        uint32_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 1);
    }

    SECTION("Convert end time to index")
    {
        isx::Time start;
        isx::Ratio step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::Time time = timingInfo.getEnd();
        uint32_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 99);
    }

    SECTION("Convert time after end to index")
    {
        isx::TimingInfo timingInfo;
        isx::Time time = timingInfo.getEnd().addSecs(2);
        uint32_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 99);
    }

}
