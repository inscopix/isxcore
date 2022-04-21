#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxNVisionMovieFile.h"
#include "isxTest.h"
#include "isxArmaUtils.h"

#include "catch.hpp"
// #include <vector>

TEST_CASE("NVisionMovieFile", "[core]") 
{
    const std::string testFileName = g_resources["unitTestDataPath"] + "/nVision/20220401-022845-KTM-RQEHB_10_secs.isxb";

    isx::CoreInitialize();

    isx::NVisionMovieFile file(testFileName);

    SECTION("Is valid")
    {
        REQUIRE(file.isValid());
    }

    SECTION("File name")
    {
        REQUIRE(file.getFileName() == testFileName);
    }

    SECTION("Timing info")
    {
        const isx::Time start(2022, 4, 1, 7, 27, 06, isx::DurationInSeconds(332, 1000));
        const isx::DurationInSeconds step(9968014, 299000000);
        const size_t numSamples = 300;
        const isx::TimingInfo expTimingInfo(start, step, numSamples);

        REQUIRE(file.getTimingInfo() == expTimingInfo);
        REQUIRE(file.getTimingInfosForSeries() == std::vector<isx::TimingInfo>{expTimingInfo});
    }

    SECTION("Spacing info")
    {
        const isx::SizeInPixels_t numPixels(1280, 720);
        const isx::SpacingInfo expSpacingInfo(numPixels);

        REQUIRE(file.getSpacingInfo() == expSpacingInfo);
    }

    SECTION("Data type")
    {
        REQUIRE(file.getDataType() == isx::DataType::U8);
    }

    SECTION("Frames")
    {
        // Verify movie data by computing sum of entire movie
        const size_t numFrames = file.getTimingInfo().getNumTimes();
        // Results of codec are slightly different between windows and linux/mac,
        // but image look similar
#if ISX_OS_WIN32
        const size_t expSum = 9250722;
#else
        const size_t expSum = 9235061;
#endif
        size_t sum = 0;
        for (size_t i = 0; i < numFrames; i++)
        {
            const auto frame = file.readFrame(i);
            isx::ColumnUInt16_t frameCol;
            isx::copyFrameToColumn(frame, frameCol);
            sum += arma::sum(frameCol);
        }
        REQUIRE(sum == expSum);   
    }

    // SECTION("Acquisition info")
    // {
    //     // TODO: expose json session metadata from IDAS
    // }

    isx::CoreShutdown();
}

// // TODO: handle dropped frames
// TEST_CASE("NVisionMovieFile-Dropped", "[core]")
// {
//     isx::CoreInitialize();

//     ...

//     isx::CoreShutdown();   
// }
