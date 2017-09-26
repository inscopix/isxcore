#include "catch.hpp"

#include "isxTest.h"
#include "isxMovieFactory.h"
#include "isxMovieTiffExporter.h"
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

    
TEST_CASE("MovieTiffExportF32Test", "[core]")
{
    std::array<const char *, 3> names =
    { {
        "seriesMovie0.isxd",
        "seriesMovie1.isxd",
        "seriesMovie2.isxd"
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

    std::string exportedTiffFileName = g_resources["unitTestDataPath"] + "/exportedMovie.tiff";
    std::remove(exportedTiffFileName.c_str());

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

        // export to nwb
        std::vector<isx::SpMovie_t> movies;
        for (const auto fn: filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieTiffExporterParams params(
            movies,
            exportedTiffFileName);
        isx::runMovieTiffExporter(params, nullptr, [](float){return false;});
        
        
        // verify exported data
        {
            //isx::TiffMovie tiffMovie(exportedTiffFileName); // TODO: import is not implemented yet

        }

    }

    for (const auto & fn: filenames)
    {
        std::remove(fn.c_str());
    }
}

TEST_CASE("MovieTiffExportU16Test", "[core]")
{
    std::array<const char *, 3> names =
    { {
            "seriesMovie0.isxd",
            "seriesMovie1.isxd",
            "seriesMovie2.isxd"
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

    std::string exportedTiffFileName = g_resources["unitTestDataPath"] + "/exportedMovie.tiff";
    std::remove(exportedTiffFileName.c_str());

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

        // export to nwb
        std::vector<isx::SpMovie_t> movies;
        for (const auto fn : filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieTiffExporterParams params(
            movies,
            exportedTiffFileName);
        isx::runMovieTiffExporter(params, nullptr, [](float) {return false; });


        // verify exported data
        {
            isx::TiffMovie tiffMovie(exportedTiffFileName);
            REQUIRE(tiffMovie.getFrameHeight() == sizePixels.getHeight());
            REQUIRE(tiffMovie.getFrameWidth() == sizePixels.getWidth());
            REQUIRE(tiffMovie.getDataType() == isx::DataType::U16);
            REQUIRE(tiffMovie.getNumFrames() == 12);
        }

    }

    for (const auto & fn : filenames)
    {
        std::remove(fn.c_str());
    }
}

TEST_CASE("MovieTiffExportSplittedTest", "[core]")
{
    const int numInputMovies = 1;
    std::array<const char *, numInputMovies> names =
    { {
            "seriesMovieSplitted0.isxd"
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

    std::string exportedTiffFileName = g_resources["unitTestDataPath"] + "/exportedSplittedMovie.tiff";
    std::remove(exportedTiffFileName.c_str());

    std::array<isx::TimingInfo, numInputMovies> timingInfos =
    { {
        { isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 5 }
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

        // export to nwb
        std::vector<isx::SpMovie_t> movies;
        for (const auto fn : filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieTiffExporterParams params(
            movies,
            exportedTiffFileName,
            3);
        isx::runMovieTiffExporter(params, nullptr, [](float) {return false; });


        // verify exported data
        {
            isx::TiffMovie tiffMovie(exportedTiffFileName);
            REQUIRE(tiffMovie.getFrameHeight() == sizePixels.getHeight());
            REQUIRE(tiffMovie.getFrameWidth() == sizePixels.getWidth());
            REQUIRE(tiffMovie.getDataType() == isx::DataType::U16);
            REQUIRE(tiffMovie.getNumFrames() == 3);
        }

        {
            const std::string dirname = isx::getDirName(exportedTiffFileName);
            const std::string basename = isx::getBaseName(exportedTiffFileName);
            const std::string extension = isx::getExtension(exportedTiffFileName);
            std::string exportedSecondTiffFileName = dirname + "/" + basename + "_" + isx::convertNumberToPaddedString(1, 2) + "." + extension;

            isx::TiffMovie tiffMovie(exportedSecondTiffFileName);
            REQUIRE(tiffMovie.getFrameHeight() == sizePixels.getHeight());
            REQUIRE(tiffMovie.getFrameWidth() == sizePixels.getWidth());
            REQUIRE(tiffMovie.getDataType() == isx::DataType::U16);
            REQUIRE(tiffMovie.getNumFrames() == 2);

            std::remove(exportedSecondTiffFileName.c_str());
        }
    }

    for (const auto & fn : filenames)
    {
        std::remove(fn.c_str());
    }
}
