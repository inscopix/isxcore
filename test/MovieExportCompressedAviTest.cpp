#include "catch.hpp"

#include "isxTest.h"
#include "isxMovieFactory.h"
#include "isxMovieCompressedAviExporter.h"
#include "isxTiffMovie.h"
#include "isxPathUtils.h"
#include "isxCore.h"

#include <array>
#include <sys/stat.h>

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

    void
    createFrameDataCheckerboard(float * outBuf, int32_t inNumFrames, int32_t inPixelsPerFrame, int32_t inWidth, int32_t szSquare, int32_t inVelX, int32_t inVelY)
    {
        for (int32_t f = 0; f < inNumFrames; ++f)
        {
            for (int32_t p = 0; p < inPixelsPerFrame; ++p)
            {
                int32_t x = p % inWidth + f * inVelX;
                int32_t y = p / inWidth + f * inVelY;

                int32_t xLowRes;
                if (x >= 0)
                {
                    xLowRes = x / szSquare;
                }
                else // this implements floor for negative number
                {
                    xLowRes = -((-x - 1) / szSquare) - 1;
                }

                int32_t yLowRes;
                if (y >= 0)
                {
                    yLowRes = y / szSquare;
                }
                else // this implements floor for negative number
                {
                    yLowRes = -((-y - 1) / szSquare) - 1;
                }

                // abs is used to avoid naive floor of a negative number: abs does not change result for "% 2"
                float intensityValue = float((abs(xLowRes + yLowRes)) % 2);
                outBuf[f * inPixelsPerFrame + p] = intensityValue;
            }
        }
    }

    int64_t GetFileSize(std::string filename)
    {
        struct stat stat_buf;
        int rc = stat(filename.c_str(), &stat_buf);
        return rc == 0 ? stat_buf.st_size : -1;
    }

} // namespace

    
TEST_CASE("MovieCompressedAviExportF32Test", "[core]")
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
            exportedCompressedAviFileName,
            isx::isize_t(400000));
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

TEST_CASE("MovieCompressedAviExportU16Test", "[core]")
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
            exportedCompressedAviFileName,
            isx::isize_t(400000));
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

TEST_CASE("MovieCompressedAviExportBitrateTest", "[core]")
{
    std::array<const char *, 1> names =
    { {
            "seriesMovie0_avi.isxd"
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

    //std::vector<isx::isize_t> dropped = { 2 };
    std::array<isx::TimingInfo, 1> timingInfos =
    { {
        { isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 200 }
    } };

    isx::SizeInPixels_t sizePixels(240, 160);
    isx::SpacingInfo spacingInfo(sizePixels);

    isx::CoreInitialize();

    SECTION("Export F32 movie series")
    {
        // write isxds
        const auto pixelsPerFrame = int32_t(sizePixels.getWidth() * sizePixels.getHeight());
        std::vector<float> buf(pixelsPerFrame * 200);   // longest movie segment has 200 frames

        int32_t i = 0;
        for (const auto fn : filenames)
        {
            int32_t width = int32_t(sizePixels.getWidth());
            int32_t szSquare = 15; // size of each square on checkerboard pattern
            int32_t velX = -3; // motion of checkerboard
            int32_t velY = 2;
            createFrameDataCheckerboard(buf.data(), int32_t(timingInfos[i].getNumTimes()), pixelsPerFrame, width, szSquare, velX, velY);
            writeTestF32MovieGeneric(fn, timingInfos[i], spacingInfo, buf.data());
            ++i;
        }

        std::vector<isx::SpMovie_t> movies;
        for (const auto fn : filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }

        double startBitRateFraction = .1;
        double endBitRateFraction = .0001;
        double incBitRateFraction = .1;

        int64_t uncompressedFileSize = GetFileSize(filenames[0]);
        int64_t compressedFileSizeLast = 0;
        for (double bitRateFraction = startBitRateFraction; bitRateFraction >= endBitRateFraction; bitRateFraction *= incBitRateFraction)
        {
            std::remove(exportedCompressedAviFileName.c_str());
            isx::MovieCompressedAviExporterParams params(
                movies,
                exportedCompressedAviFileName,
                bitRateFraction);
            isx::runMovieCompressedAviExporter(params);
            int64_t compressedFileSize = GetFileSize(exportedCompressedAviFileName);
            if (bitRateFraction == startBitRateFraction)
            {
                REQUIRE(compressedFileSize <= uncompressedFileSize); // file size is not greater than for original movie
            }
            else
            {
                REQUIRE(compressedFileSize <= compressedFileSizeLast); // file size is not greater than when bit-rate was larger
            }
            compressedFileSizeLast = compressedFileSize;
        }
    }

    for (const auto & fn : filenames)
    {
        std::remove(fn.c_str());
    }
    std::remove(exportedCompressedAviFileName.c_str());

}
