#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxNVisionMovie.h"
#include "isxTest.h"
#include "isxArmaUtils.h"

#include "catch.hpp"
#include <atomic>
#include <thread>
#include <chrono>

TEST_CASE("NVisionMovie", "[core]") 
{
    const std::string testFileName = g_resources["unitTestDataPath"] + "/nVision/20220401-022845-KTM-RQEHB_10_secs.isxb";

    isx::CoreInitialize();

    const std::shared_ptr<isx::NVisionMovie> movie = std::make_shared<isx::NVisionMovie>(testFileName);

    SECTION("Is valid")
    {
        REQUIRE(movie->isValid());
    }

    SECTION("File name")
    {
        REQUIRE(movie->getFileName() == testFileName);
    }

    SECTION("Timing info")
    {
        const isx::Time start(2022, 4, 1, 7, 27, 06, isx::DurationInSeconds(332, 1000));
        const isx::DurationInSeconds step(9968014, 299000000);
        const size_t numSamples = 300;
        const isx::TimingInfo expTimingInfo(start, step, numSamples);

        REQUIRE(movie->getTimingInfo() == expTimingInfo);
    }

    SECTION("Spacing info")
    {
        const isx::SizeInPixels_t numPixels(1280, 720);
        const isx::SpacingInfo expSpacingInfo(numPixels);

        REQUIRE(movie->getSpacingInfo() == expSpacingInfo);
    }

    SECTION("Data type")
    {
        REQUIRE(movie->getDataType() == isx::DataType::U8);
    }

    SECTION("Frames")
    {
        // Verify some movie data by computing sum of first ten frames
        const size_t numFrames = 10;
        // Results of codec are slightly different between windows and linux/mac,
        // but image look similar
#if ISX_OS_WIN32
        const size_t expSum = 403115;
#else
        const size_t expSum = 402551;
#endif
        size_t sum = 0;
        for (size_t i = 0; i < numFrames; i++)
        {
            const auto frame = movie->getFrame(i);
            isx::ColumnUInt16_t frameCol;
            isx::copyFrameToColumn(frame, frameCol);
            sum += arma::sum(frameCol);
        }
        REQUIRE(sum == expSum);
    }

    SECTION("Async frames")
    {
        std::atomic_int doneCount(0);
        std::atomic_int sum(0);

        isx::MovieGetFrameCB_t cb = [&](isx::AsyncTaskResult<isx::SpVideoFrame_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            const isx::SpVideoFrame_t frame = inAsyncTaskResult.get();
            isx::ColumnUInt16_t frameCol;
            isx::copyFrameToColumn(frame, frameCol);
            sum += arma::sum(frameCol);
            ++doneCount;
        };

        const size_t numFrames = 10;
#if ISX_OS_WIN32
        const size_t expSum = 403115;
#else
        const size_t expSum = 402551;
#endif
        for (size_t i = 0; i < numFrames; i++)
        {
            movie->getFrameAsync(i, cb);
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numFrames))
            {
                break;
            }
            const std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == numFrames);
        REQUIRE(sum == expSum);
    }

    isx::CoreShutdown();
}

// // TODO: handle series
// TEST_CASE("NVisionMovie-Series", "[core]")
// {
//     isx::CoreInitialize();

//     ...

//     isx::CoreShutdown();   
// }
