#include "catch.hpp"

#include "isxTest.h"
#include "isxMovieFactory.h"
#include "isxMovieCompressedAviExporter.h"
#include "isxTiffMovie.h"
#include "isxPathUtils.h"
#include "isxCore.h"

#include <array>

namespace
{
    void
    createFrameData(float * outBuf, int32_t inNumFrames, int32_t inPixelsPerFrame, int32_t inBaseValueForFrame)
    {
        for (int32_t f = 0; f < inNumFrames; ++f)
        {
            for (int32_t p = 0; p < inPixelsPerFrame; ++p)
            {
                outBuf[f * inPixelsPerFrame + p] = float((inBaseValueForFrame + f) * inPixelsPerFrame + p);
            }
        }
    }

    void
    createFrameData(uint16_t * outBuf, int32_t inNumFrames, int32_t inPixelsPerFrame, int32_t inBaseValueForFrame)
    {
        for (int32_t f = 0; f < inNumFrames; ++f)
        {
            for (int32_t p = 0; p < inPixelsPerFrame; ++p)
            {
                outBuf[f * inPixelsPerFrame + p] = uint16_t((inBaseValueForFrame + f) * inPixelsPerFrame + p);
            }
        }
    }

} // namespace

    
TEST_CASE("MovieCompressedAviExportF32Test", "[core][export_mp4]")
{
    std::array<const char *, 3> names =
    { {
        "seriesMovie0_avi.isxd",
        "seriesMovie1_avi.isxd",
        "seriesMovie2_avi.isxd"
    } };
    std::vector<std::string> filenames;

    for (auto n: names)
    {
        filenames.push_back(g_resources["unitTestDataPath"] + "/" + n);
    }
    
    for (const auto & fn: filenames)
    {
        std::remove(fn.c_str());
    }

    std::string exportedCompressedAviFileName = g_resources["unitTestDataPath"] + "/exportedMovie.avi";
    std::remove(exportedCompressedAviFileName.c_str());

    std::vector<isx::isize_t> dropped = { 2 };
    std::array<isx::TimingInfo, 3> timingInfos =
    { {
        { isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 3 },
        { isx::Time(2222, 4, 1, 3, 1, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 4 },
        { isx::Time(2222, 4, 1, 3, 2, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 5, dropped }
    } };

    isx::SizeInPixels_t sizePixels(4, 3);
    isx::SpacingInfo spacingInfo(sizePixels);

    isx::DataType dataType = isx::DataType::U16;

    isx::CoreInitialize();

    SECTION("Export F32 movie series")
    {
        // write isxds
        const auto pixelsPerFrame = int32_t(sizePixels.getWidth() * sizePixels.getHeight());
        std::vector<float> buf(pixelsPerFrame * 5);   // longest movie segment has 5 frames

        int32_t i = 0;
        for (const auto fn: filenames)
        {
            createFrameData(buf.data(), int32_t(timingInfos[i].getNumTimes()), pixelsPerFrame, i * 5);
            writeTestF32MovieGeneric(fn, timingInfos[i], spacingInfo, buf.data());
            ++i;
        }

        std::vector<isx::SpMovie_t> movies;
        for (const auto fn: filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieCompressedAviExporterParams params(
            movies,
			exportedCompressedAviFileName);
        isx::runMovieCompressedAviExporter(params);
        
        
        // verify exported data
        {
			// TODO: CompressedAviMovie not implemented yet
            //isx::CompressedAviMovie compressedAviMovie(exportedCompressedAviFileName); // TODO: import is not implemented yet

        }

    }

    for (const auto & fn: filenames)
    {
        std::remove(fn.c_str());
    }

}

TEST_CASE("MovieCompressedAviExportU16Test", "[core][export_mp4]")
{
    std::array<const char *, 3> names =
    { {
            "seriesMovie0_avi.isxd",
            "seriesMovie1_avi.isxd",
            "seriesMovie2_avi.isxd"
        } };
    std::vector<std::string> filenames;

    for (auto n : names)
    {
        filenames.push_back(g_resources["unitTestDataPath"] + "/" + n);
    }

    for (const auto & fn : filenames)
    {
        std::remove(fn.c_str());
    }

    std::string exportedCompressedAviFileName = g_resources["unitTestDataPath"] + "/exportedMovie.avi";
    std::remove(exportedCompressedAviFileName.c_str());

    std::vector<isx::isize_t> dropped = { 2 };
    std::array<isx::TimingInfo, 3> timingInfos =
    { {
        { isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 3 },
        { isx::Time(2222, 4, 1, 3, 1, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 4 },
        { isx::Time(2222, 4, 1, 3, 2, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 5, dropped }
        } };

    isx::SizeInPixels_t sizePixels(4, 3);
    isx::SpacingInfo spacingInfo(sizePixels);

    isx::CoreInitialize();

    SECTION("Export movie series")
    {
        // write isxds
        const auto pixelsPerFrame = int32_t(sizePixels.getWidth() * sizePixels.getHeight());
        std::vector<uint16_t> buf(pixelsPerFrame * 5);   // longest movie segment has 5 frames

        int32_t i = 0;
        for (const auto fn : filenames)
        {
            createFrameData(buf.data(), int32_t(timingInfos[i].getNumTimes()), pixelsPerFrame, i * 5);
            writeTestU16MovieGeneric(fn, timingInfos[i], spacingInfo, buf.data());
            ++i;
        }

        std::vector<isx::SpMovie_t> movies;
        for (const auto fn : filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieCompressedAviExporterParams params(
            movies,
			exportedCompressedAviFileName);
        isx::runMovieCompressedAviExporter(params);


        // verify exported data
        {
			// TODO: CompressedAviMovie not implemented yet
            // isx::CompressedAviMovie compressedAviMovie(exportedCompressedAviFileName);
            // REQUIRE(compressedAviMovie.getFrameHeight() == sizePixels.getHeight());
            // REQUIRE(compressedAviMovie.getFrameWidth() == sizePixels.getWidth());
            // REQUIRE(compressedAviMovie.getDataType() == isx::DataType::U16);
            // REQUIRE(compressedAviMovie.getNumFrames() == 11);
        }

    }

    for (const auto & fn : filenames)
    {
        std::remove(fn.c_str());
    }
}

TEST_CASE("MOS-1675", "[core][export_mp4]")
{
    isx::CoreInitialize();

    const std::string inputDir = g_resources["unitTestDataPath"] + "/export_mp4";
    const std::string outputDir = inputDir + "/output";

    isx::removeDirectory(outputDir);
    isx::makeDirectory(outputDir);

    SECTION("Data with a frame period of greater than 2^16 - 1")
    {
        const std::string inputFile = inputDir + "/2018-08-09-17-05-09_video-PP-TPC.isxd";
        const isx::SpMovie_t movie = isx::readMovie(inputFile);
        const std::string outputFile = outputDir + "/" + isx::getBaseName(inputFile) + ".mp4";
        isx::MovieCompressedAviExporterParams params({movie}, outputFile);

        REQUIRE(isx::runMovieCompressedAviExporter(params) == isx::AsyncTaskStatus::COMPLETE);
    }

    isx::removeDirectory(outputDir);
    isx::CoreShutdown();
}
