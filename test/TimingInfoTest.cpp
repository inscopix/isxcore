#include "isxTest.h"
#include "isxTimingInfo.h"
#include "catch.hpp"

TEST_CASE("TimingInfoTest", "[core]")
{
    SECTION("Default constructor")
    {
        isx::TimingInfo timingInfo;
        REQUIRE(timingInfo.isValid() == false);
    }

    SECTION("Valid usage of constructor")
    {
        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::isize_t numTimes = 20;
        std::vector<isx::isize_t> droppedFrames{4, 7};
        isx::TimingInfo timingInfo(start, step, numTimes, droppedFrames);
        REQUIRE(timingInfo.getStart() == start);
        REQUIRE(timingInfo.getStep() == step);
        REQUIRE(timingInfo.getNumTimes() == numTimes);
        std::vector<isx::isize_t> retrievedDroppedFrames = timingInfo.getDroppedFrames();
        REQUIRE(retrievedDroppedFrames == droppedFrames);
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

}

TEST_CASE("TimingInfoConversionTest", "[core]")
{

    isx::Time start(1970, 1, 1, 0, 0, 0);
    isx::DurationInSeconds step(50, 1000);
    isx::isize_t numTimes = 100;
    isx::TimingInfo timingInfo(start, step, numTimes);

    // Index to time
    SECTION("Convert the first index to a time")
    {
        isx::Time actual = timingInfo.convertIndexToMidTime(0);
        isx::Time expected(1970, 1, 1, 0, 0, 0, isx::DurationInSeconds(25, 1000));
        REQUIRE(actual == expected);
    }

    SECTION("Convert a valid index to a time")
    {
        isx::Time actual = timingInfo.convertIndexToMidTime(2);
        isx::Time expected(1970, 1, 1, 0, 0, 0, isx::DurationInSeconds(125, 1000));
        REQUIRE(actual == expected);
    }

    SECTION("Convert an index that exceeds the number of samples to a time")
    {
        isx::Time actual = timingInfo.convertIndexToMidTime(100);
        isx::Time expected(1970, 1, 1, 0, 0, 4, isx::DurationInSeconds(975, 1000));
        REQUIRE(actual == expected);
    }

    SECTION("Convert an index to a time when there are no samples")
    {
        numTimes = 0;
        timingInfo = isx::TimingInfo(start, step, numTimes);
        isx::Time actual = timingInfo.convertIndexToMidTime(2);
        REQUIRE(actual == start);
    }

    // Index to start time of window
    SECTION("Convert the first index to a start time")
    {
        isx::Time actual = timingInfo.convertIndexToStartTime(0);
        REQUIRE(actual == start);
    }

    SECTION("Convert a valid index to a start time")
    {
        isx::Time actual = timingInfo.convertIndexToStartTime(2);
        isx::Time expected(1970, 1, 1, 0, 0, 0, isx::DurationInSeconds(100, 1000));
        REQUIRE(actual == expected);
    }

    SECTION("Convert an index that exceeds the number of samples to a start time")
    {
        isx::Time actual = timingInfo.convertIndexToStartTime(100);
        isx::Time expected(1970, 1, 1, 0, 0, 4, isx::DurationInSeconds(950, 1000));
        REQUIRE(actual == expected);
    }

    SECTION("Convert an index to a start time when there are no samples")
    {
        numTimes = 0;
        timingInfo = isx::TimingInfo(start, step, numTimes);
        isx::Time actual = timingInfo.convertIndexToStartTime(2);
        REQUIRE(actual == start);
    }

    // Time to index
    SECTION("Convert start time to index")
    {
        isx::isize_t index = timingInfo.convertTimeToIndex(start);
        REQUIRE(index == 0);
    }

    SECTION("Convert time before the start to index")
    {
        start += isx::DurationInSeconds(10);
        isx::DurationInSeconds step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        isx::Time time = start - 2;
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 0);
    }

    SECTION("Convert time half a sample after start to an index")
    {
        isx::Time time = start + isx::DurationInSeconds(25, 1000);
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 0);
    }

    SECTION("Convert time one sample after the start to an index")
    {
        isx::Time time = start + isx::DurationInSeconds(50, 1000);
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 1);
    }

    SECTION("Convert time almost one and a half samples after the start to an index")
    {
        isx::Time time = start + isx::DurationInSeconds(74, 1000);
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 1);
    }

    SECTION("Convert end time to index")
    {
        isx::Time time = timingInfo.getEnd();
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 99);
    }

    SECTION("Convert time after end to index")
    {
        isx::Time time = timingInfo.getEnd() + 2;
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 99);
    }

    SECTION("Convert largest time value in a frame to an index")
    {
        isx::Time time = start + isx::DurationInSeconds(199, 1000);
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 3);
    }

    SECTION("Convert smallest time value in a frame to an index")
    {
        isx::Time time = start + isx::DurationInSeconds(200, 1000);
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 4);
    }

    SECTION("Convert time to index when there are no samples")
    {
        numTimes = 0;
        timingInfo = isx::TimingInfo(start, step, numTimes);
        isx::Time time = start + isx::DurationInSeconds(74, 1000);
        isx::isize_t index = timingInfo.convertTimeToIndex(time);
        REQUIRE(index == 0);
    }

    SECTION("Convert to string")
    {
        isx::Time start(2016, 6, 20, 11, 10);
        isx::Ratio step(50, 1000);
        isx::TimingInfo timingInfo(start, step, 100);
        std::string expected = "TimingInfo(Start=20160620-111000 0 / 1 UTC, "
            "Step=50 / 1000, NumTimes=100)";
        REQUIRE(timingInfo.toString() == expected);
    }

    SECTION("Adjust indices for dropped frames")
    {
        isx::Time start(2016, 6, 20, 11, 10);
        isx::Ratio step(50, 1000);
        std::vector<isx::isize_t> dropped{2, 5};
        isx::isize_t numTimes = 10;
        isx::TimingInfo ti(start, step, numTimes, dropped);

        REQUIRE(ti.getDroppedCount() == 2);
        
        for(isx::isize_t i(0); i < numTimes; ++i)
        {
            if(i == 2 || i == 5)
            {
                REQUIRE(true == ti.isDropped(i));
            }
            else
            {
                REQUIRE(false == ti.isDropped(i));

                if(i < 2)
                {
                    REQUIRE(i == ti.timeIdxToRecordedIdx(i));
                }
                else if(i > 2 && i < 5)
                {
                    REQUIRE((i-1) == ti.timeIdxToRecordedIdx(i));                    
                }
                else
                {
                    REQUIRE((i-2) == ti.timeIdxToRecordedIdx(i));
                }
            }
        }

    }

}
