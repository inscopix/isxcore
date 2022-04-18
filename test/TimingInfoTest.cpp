#include "isxTest.h"
#include "isxTimingInfo.h"
#include "isxStopWatch.h"

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
        std::string expected = "TimingInfo(Start=2016/06/20-11:10:00.000, "
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

TEST_CASE("TimingInfo-cropped", "[core]")
{
    const isx::Time start(1970, 1, 1, 0, 0, 0);
    const isx::DurationInSeconds step(50, 1000);
    const isx::isize_t numTimes = 10;

    SECTION("Empty cropped indices")
    {
        const isx::IndexRanges_t ranges;
        const isx::TimingInfo ti(start, step, numTimes, {}, ranges);

        REQUIRE(ti.getCropped().empty());
        REQUIRE(ti.getCroppedCount() == 0);

        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            const isx::isize_t storedIndex = ti.timeIdxToRecordedIdx(t);
            if (storedIndex != t)
            {
                FAIL("Converted frame " << t << " to stored index " << storedIndex);
            }
        }
    }

    SECTION("Single cropped index")
    {
        const isx::IndexRanges_t ranges = {isx::IndexRange(2)};
        const isx::TimingInfo ti(start, step, numTimes, {}, ranges);

        REQUIRE(ranges == ti.getCropped());
        REQUIRE(ti.getCroppedCount() == 1);
        isx::isize_t offset = 0;
        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            if (t == 2)
            {
                if (!ti.isCropped(t))
                {
                    FAIL("Frame " << t << " should be cropped, but isCropped returns false.");
                }
                ++offset;
            }
            else
            {
                if (ti.isCropped(t))
                {
                    FAIL("Frame " << t << " should be not cropped, but isCropped returns true.");
                }
                const isx::isize_t storedIndex = ti.timeIdxToRecordedIdx(t);
                if (storedIndex != (t - offset))
                {
                    FAIL("Converted frame " << t << " to stored index " << storedIndex);
                }
            }
        }
    }

    SECTION("Single and range of cropped indices")
    {
        const isx::IndexRanges_t ranges = {isx::IndexRange(2), isx::IndexRange(5, 8)};
        const isx::TimingInfo ti(start, step, numTimes, {}, ranges);

        REQUIRE(ranges == ti.getCropped());
        REQUIRE(ti.getCroppedCount() == 5);
        isx::isize_t offset = 0;
        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            if (t == 2 || (t >= 5 && t <= 8))
            {
                if (!ti.isCropped(t))
                {
                    FAIL("Frame " << t << " should be cropped, but isCropped returns false.");
                }
                ++offset;
            }
            else
            {
                if (ti.isCropped(t))
                {
                    FAIL("Frame " << t << " should not be cropped, but isCropped returns true.");
                }
                const isx::isize_t storedIndex = ti.timeIdxToRecordedIdx(t);
                if (storedIndex != (t - offset))
                {
                    FAIL("Converted frame " << t << " to stored index " << storedIndex);
                }
            }
        }
    }

    SECTION("Two ranges of cropped indices")
    {
        const isx::IndexRanges_t ranges = {isx::IndexRange(2, 3), isx::IndexRange(5, 8)};
        const isx::TimingInfo ti(start, step, numTimes, {}, ranges);

        REQUIRE(ranges == ti.getCropped());
        REQUIRE(ti.getCroppedCount() == 6);
        isx::isize_t offset = 0;
        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            if ((t >= 2 && t <= 3) || (t >= 5 && t <= 8))
            {
                if (!ti.isCropped(t))
                {
                    FAIL("Frame " << t << " should be cropped, but isCropped returns false.");
                }
                ++offset;
            }
            else
            {
                if (ti.isCropped(t))
                {
                    FAIL("Frame " << t << " should not be cropped, but isCropped returns true.");
                }
                const isx::isize_t storedIndex = ti.timeIdxToRecordedIdx(t);
                if (storedIndex != (t - offset))
                {
                    FAIL("Converted frame " << t << " to stored index " << storedIndex);
                }
            }
        }
    }

    SECTION("Two overlapping ranges of cropped indices get simplified to one")
    {
        const isx::IndexRanges_t ranges = {isx::IndexRange(2, 4), isx::IndexRange(4, 6)};
        const isx::TimingInfo ti(start, step, numTimes, {}, ranges);

        const isx::IndexRanges_t expectedRanges = {isx::IndexRange(2, 6)};
        REQUIRE(ti.getCropped() == expectedRanges);
    }

    SECTION("One range absorbs another range of cropped indices")
    {
        const isx::IndexRanges_t ranges = {isx::IndexRange(2, 8), isx::IndexRange(4, 6)};
        const isx::TimingInfo ti(start, step, numTimes, {}, ranges);

        const isx::IndexRanges_t expectedRanges = {isx::IndexRange(2, 8)};
        REQUIRE(ti.getCropped() == expectedRanges);
    }

    SECTION("Two equal ranges of cropped indices")
    {
        const isx::IndexRanges_t ranges = {isx::IndexRange(2, 6), isx::IndexRange(2, 6)};
        const isx::TimingInfo ti(start, step, numTimes, {}, ranges);

        const isx::IndexRanges_t expectedRanges = {isx::IndexRange(2, 6)};
        REQUIRE(ti.getCropped() == expectedRanges);
    }

    SECTION("Three adjacent ranges followed by separated range")
    {
        const isx::IndexRanges_t ranges = {
            isx::IndexRange(2, 6), 
            isx::IndexRange(7, 7),
            isx::IndexRange(8, 12),
            isx::IndexRange(24, 42)
        };
        const isx::TimingInfo ti(start, step, numTimes, {}, ranges);

        const isx::IndexRanges_t expectedRanges = {
            isx::IndexRange(2, 12),
            isx::IndexRange(24, 42)
        };
        REQUIRE(ti.getCropped() == expectedRanges);
    }

}

TEST_CASE("TimingInfo-blank", "[core]")
{
    const isx::Time start(1970, 1, 1, 0, 0, 0);
    const isx::DurationInSeconds step(50, 1000);
    const isx::isize_t numTimes = 10;

    SECTION("Empty blank indices")
    {
        const std::vector<isx::isize_t> indices;
        const isx::TimingInfo ti(start, step, numTimes, {}, {}, indices);

        REQUIRE(ti.getBlankFrames().empty());
        REQUIRE(ti.getBlankCount() == 0);

        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            const isx::isize_t storedIndex = ti.timeIdxToRecordedIdx(t);
            if (storedIndex != t)
            {
                FAIL("Converted frame " << t << " to stored index " << storedIndex);
            }
        }
    }

    SECTION("Single blank index")
    {
        const std::vector<isx::isize_t> indices = {2};
        const isx::TimingInfo ti(start, step, numTimes, {}, {}, indices);

        REQUIRE(indices == ti.getBlankFrames());
        REQUIRE(ti.getBlankCount() == 1);
        isx::isize_t offset = 0;
        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            if (t == 2)
            {
                if (!ti.isBlank(t))
                {
                    FAIL("Frame " << t << " should be blank, but isBlank returns false.");
                }
                ++offset;
            }
            else
            {
                if (ti.isBlank(t))
                {
                    FAIL("Frame " << t << " should be not blank, but isBlank returns true.");
                }
                const isx::isize_t storedIndex = ti.timeIdxToRecordedIdx(t);
                if (storedIndex != (t - offset))
                {
                    FAIL("Converted frame " << t << " to stored index " << storedIndex);
                }
            }
        }
    }

    SECTION("Three blank indices")
    {
        const std::vector<isx::isize_t> indices = {2, 7, 9};
        const isx::TimingInfo ti(start, step, numTimes, {}, isx::IndexRanges_t(), indices);

        REQUIRE(indices == ti.getBlankFrames());
        REQUIRE(ti.getBlankCount() == 3);
        isx::isize_t offset = 0;
        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            if (t == 2 || t == 7 || t == 9)
            {
                if (!ti.isBlank(t))
                {
                    FAIL("Frame " << t << " should be blank, but isBlank returns false.");
                }
                ++offset;
            }
            else
            {
                if (ti.isBlank(t))
                {
                    FAIL("Frame " << t << " should be not blank, but isBlank returns true.");
                }
                const isx::isize_t storedIndex = ti.timeIdxToRecordedIdx(t);
                if (storedIndex != (t - offset))
                {
                    FAIL("Converted frame " << t << " to stored index " << storedIndex);
                }
            }
        }
    }
}

TEST_CASE("TimingInfo-validIndices", "[core]")
{
    const isx::Time start(1970, 1, 1, 0, 0, 0);
    const isx::DurationInSeconds step(50, 1000);
    const isx::isize_t numTimes = 10;

    SECTION("No dropped, cropped, or blank frames")
    {
        const isx::TimingInfo ti(start, step, numTimes, {}, {}, {});

        REQUIRE(ti.getNumValidTimes() == 10);
        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            if (!ti.isIndexValid(t))
            {
                FAIL("Index " << t << " should be valid.");
            }
        }
    }

    SECTION("Some dropped frames")
    {
        const isx::TimingInfo ti(start, step, numTimes, {4, 8}, {});

        REQUIRE(ti.getNumValidTimes() == 8);
        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            if (ti.isDropped(t))
            {
                if (ti.isIndexValid(t))
                {
                    FAIL("Index " << t << " should not be valid.");
                }
            }
            else
            {
                if (!ti.isIndexValid(t))
                {
                    FAIL("Index " << t << " should be valid.");
                }
            }
        }
    }

    SECTION("Some cropped frames")
    {
        const isx::TimingInfo ti(start, step, numTimes, {}, {3, isx::IndexRange(5, 7)});

        REQUIRE(ti.getNumValidTimes() == 6);
        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            if (ti.isCropped(t))
            {
                if (ti.isIndexValid(t))
                {
                    FAIL("Index " << t << " should not be valid.");
                }
            }
            else
            {
                if (!ti.isIndexValid(t))
                {
                    FAIL("Index " << t << " should be valid.");
                }
            }
        }
    }

    SECTION("Some blank frames")
    {
        const isx::TimingInfo ti(start, step, numTimes, {}, {}, {4, 8});

        REQUIRE(ti.getNumValidTimes() == 8);
        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            if (ti.isBlank(t))
            {
                if (ti.isIndexValid(t))
                {
                    FAIL("Index " << t << " should not be valid.");
                }
            }
            else
            {
                if (!ti.isIndexValid(t))
                {
                    FAIL("Index " << t << " should be valid.");
                }
            }
        }
    }

    SECTION("Some dropped, cropped, and blank frames")
    {
        const isx::TimingInfo ti(start, step, numTimes, {2}, {isx::IndexRange(4, 6), {8}});

        REQUIRE(ti.getNumValidTimes() == 5);
        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            if (ti.isDropped(t) || ti.isCropped(t))
            {
                if (ti.isIndexValid(t))
                {
                    FAIL("Index " << t << " should not be valid.");
                }
            }
            else
            {
                if (!ti.isIndexValid(t))
                {
                    FAIL("Index " << t << " should be valid.");
                }
            }
        }
    }

}

TEST_CASE("TimingInfo-droppedAndCroppedBench", "[core][!hide]")
{
    const isx::Time start;
    const isx::DurationInSeconds step(50, 1000);

    SECTION("100000 frames, 1000 ranges of 5 cropped indices and 1000 isolated dropped indices")
    {
        const isx::isize_t numTimes = 100000;
        std::vector<isx::isize_t> droppedFrames;
        isx::IndexRanges_t cropped;
        for (isx::isize_t t = 0; t < numTimes; t += 100)
        {
            cropped.push_back(isx::IndexRange(t, t + 4));
            droppedFrames.push_back(t + 6);
        }
        const isx::TimingInfo ti(start, step, numTimes, droppedFrames, cropped);

        isx::StopWatch globalTimer;
        globalTimer.start();
        float maxDurationInMs = 0.f;
        float maxIndex = 0;
        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            isx::StopWatch localTimer;
            localTimer.start();
            if (ti.isIndexValid(t))
            {
                const isx::isize_t storedIndex = ti.timeIdxToRecordedIdx(t);
                (void)(storedIndex);
            }
            localTimer.stop();

            const float localDurationInMs = localTimer.getElapsedMs();
            if (localDurationInMs > maxDurationInMs)
            {
                maxDurationInMs = localDurationInMs;
                maxIndex = float(t);
            }
        }
        globalTimer.stop();
        const float durationInMs = globalTimer.getElapsedMs();
        const float durationInMsPerTime = durationInMs / numTimes;
        ISX_LOG_INFO(
                "Scanning 100000 time points with 1000 ranges of 5 cropped indices "
                "and 1000 dropped indices took ", durationInMs, " ms. ",
                "That's ", durationInMsPerTime, " ms per frame. ",
                "The maximum duration was ", maxDurationInMs, " ms for frame ", maxIndex);
    }
}

// IDPS-857: The purpose of this test case is to verify the functionality of the timing info
// when the step size is composed of large numbers in order to maintain full precision.
// Previously, using large numbers in the step size led to issues with integer overflow in downstream calculations.
// This problem was solved by increasing the number of bits used to represent the numerator & denominator of a Ratio.
TEST_CASE("TimingInfo-largeNumStep", "[core]")
{
    SECTION("calculate end time without int overflow")
    {
        const isx::Time start(2022, 3, 22, 14, 35, 41, isx::DurationInSeconds::fromMilliseconds(300));
        const isx::DurationInSeconds step(isx::Ratio(3547428992, 356173000000));
        const uint64_t numSamples = 356174;
        const isx::TimingInfo ti(start, step, numSamples);

        REQUIRE(ti.getEnd() > ti.getStart());
    }

    SECTION("index -> time -> index")
    {
        const isx::Time start(2022, 3, 22, 14, 35, 41, isx::DurationInSeconds::fromMilliseconds(300));
        const isx::DurationInSeconds step(isx::Ratio(3547428992, 356173000000));
        const uint64_t numSamples = 356174;
        const isx::TimingInfo ti(start, step, numSamples);

        for (size_t i = 0; i < 10; i++)
        {
            const auto time = ti.convertIndexToStartTime(i);
            const auto i_prime = ti.convertTimeToIndex(time);
            REQUIRE(i == i_prime);
        }
    }
}
