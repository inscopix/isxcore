#include "catch.hpp"

#include "isxTest.h"
#include "isxMovieFactory.h"
#include "isxMovieCompressedAviExporter.h"
#include "isxTiffMovie.h"
#include "isxPathUtils.h"
#include "isxCore.h"
#include "isxBehavMovieFile.h"
#include "isxArmaUtils.h"

#if ISX_ARCH_ARM == 0
#include "isxTemporalCrop.h"
#endif

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

#if ISX_ARCH_ARM == 0
    void
    trimAndCompressMovie(
            const std::string & inInputFile,
            const std::string & inOutputDir,
            const isx::isize_t inNumFramesToKeep,
            const std::vector<double> & inBitRateFractions)
    {
        const std::string outputBase = inOutputDir + "/" + isx::getBaseName(inInputFile) + "-trimmed";

        const std::string trimmedFile = outputBase + ".isxd";
        std::remove(trimmedFile.c_str());
        const isx::SpMovie_t movie = isx::readMovie(inInputFile);
        const isx::isize_t numFrames = movie->getTimingInfo().getNumTimes();
        const isx::TemporalCropParams trimInputParams(movie, trimmedFile,
                {isx::IndexRange(inNumFramesToKeep, numFrames - 1)});
        auto trimOutputParams = std::make_shared<isx::TemporalCropOutputParams>();
        isx::temporalCrop(trimInputParams, trimOutputParams, [](float){return false;});

        const isx::SpMovie_t trimmedMovie = isx::readMovie(trimmedFile);
        isx::MovieCompressedAviExporterParams params({trimmedMovie}, "", 1.0);
        for (const double bitRateFraction : inBitRateFractions)
        {
            ISX_LOG_INFO("Compressing ", trimmedFile, " with bit-rate fraction ", bitRateFraction);
            params.m_filename = outputBase + "-brf_" + std::to_string(size_t(bitRateFraction * 1000)) + ".mp4";
            std::remove(params.m_filename.c_str());
            params.m_bitRateFraction = bitRateFraction;
            isx::runMovieCompressedAviExporter(params);
        }
    }
#endif

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

TEST_CASE("MovieCompressedAviExportU8Test", "[core][export_mp4]")
{
    const std::vector<std::string> inputFileNames = {
        g_resources["unitTestDataPath"] + "/nVision/20220401-022845-KTM-RQEHB_10_secs.isxb"
    };

    std::string exportedCompressedAviFileName = g_resources["unitTestDataPath"] + "/exportedMovie.mp4";

    isx::CoreInitialize();

    isx::SpMovie_t inputMovie = isx::readMovie(inputFileNames[0]);

    const double bitRateFraction = 0.1;
    isx::MovieCompressedAviExporterParams params(
        {inputMovie},
        exportedCompressedAviFileName,
        bitRateFraction);
    isx::runMovieCompressedAviExporter(params);
    
    SECTION("Verify exported data")
    {
        isx::DataSet::Properties props = {};
        props[isx::DataSet::PROP_MOVIE_FRAME_RATE] = isx::Variant(20.f);
        props[isx::DataSet::PROP_MOVIE_START_TIME] = isx::Variant(isx::Time());
        isx::BehavMovieFile::getBehavMovieProperties(exportedCompressedAviFileName, props);
        isx::SpMovie_t exportedMovie = isx::readBehavioralMovie(exportedCompressedAviFileName, props);

        REQUIRE(exportedMovie->getTimingInfo().getNumTimes() == inputMovie->getTimingInfo().getNumTimes());

        // Verify some movie data by computing sum of entire movie
        const size_t numFrames = inputMovie->getTimingInfo().getNumTimes();
        // Results of codec are slightly different between windows and linux/mac, but images look similar
#if ISX_OS_WIN32
        const size_t expSum = 9720275;
#else
        const size_t expSum = 9752141;
#endif
        size_t sum = 0;
        for (size_t i = 0; i < numFrames; i++)
        {
            const auto frame = exportedMovie->getFrame(i);
            isx::ColumnUInt16_t frameCol;
            isx::copyFrameToColumn(frame, frameCol);
            sum += arma::sum(frameCol);
        }
        REQUIRE(sum == expSum);
    }

    isx::CoreShutdown();

    std::remove(exportedCompressedAviFileName.c_str());
}

TEST_CASE("MovieCompressedAviExportBitrateTest", "[core][export_mp4]")
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
                REQUIRE(compressedFileSize <= uncompressedFileSize); // file size has not increased relative to original movie
            }
            else
            {
                REQUIRE(compressedFileSize <= compressedFileSizeLast); // file size does not increase as bit-rate decreases
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
        isx::MovieCompressedAviExporterParams params({movie}, outputFile, 0.25);

        REQUIRE(isx::runMovieCompressedAviExporter(params) == isx::AsyncTaskStatus::COMPLETE);
    }

    isx::removeDirectory(outputDir);
    isx::CoreShutdown();
}

#if ISX_ARCH_ARM == 0

TEST_CASE("MOS-1408", "[!hide]")
{
    isx::CoreInitialize();

    // You will likely need to change these when running on your own machine.
    const std::string data2Dir = "/Volumes/Data2";
    const std::string outputDir = "/Users/sweet/scratch/mos1408";

    isx::makeDirectory(outputDir);

    const std::string inputDir = data2Dir + "/standard_datasets";
    const std::vector<std::pair<std::string, std::string>> dataPaths =
    {
        {"hippocampus/inscopix/BO3/20170818", "recording_20170818_125053"},
        {"hippocampus/inscopix/BO4/20170818", "recording_20170818_133231"},
        {"hippocampus/inscopix/DD1/20171013", "recording_20171013_133816"},
        {"hippocampus/inscopix/SAAV1-H-IM10/20170202", "recording_20170202_135509"},
        {"hippocampus/inscopix/SAAV1-H-IM7/20170202", "recording_20170202_100149"},
        {"hippocampus/inscopix/SAAV1-H-IM9/20170202", "recording_20170202_114629"},
        {"pfc/inscopix/IM2/20171130", "recording_20171130_084154"},
        {"pfc/inscopix/V3_38/20170825", "recording_20170825_143312"},
        {"pfc/inscopix/V3_40/20170830", "recording_20170830_100336"},
    };

    std::vector<std::string> ppFiles;
    std::vector<std::string> bpFiles;
    std::vector<std::string> dffFiles;
    for (const auto & p : dataPaths)
    {
        const std::string dataDir = inputDir + "/" + p.first;
        ppFiles.push_back(dataDir + "/" + p.second + "-PP.isxd");
        bpFiles.push_back(dataDir + "/pipeline/" + p.second + "-PP-bp.isxd");
        dffFiles.push_back(dataDir + "/pipeline/" + p.second + "-PP-bp-mc-dff.isxd");
    }
    const isx::isize_t numFramesToKeep = 1000;
    const std::vector<double> bitRateFractions({0.001, 0.01, 0.1, 1.0});

    SECTION("PP")
    {
        for (const auto & ppFile : ppFiles)
        {
            trimAndCompressMovie(ppFile, outputDir, numFramesToKeep, bitRateFractions);
        }
    }

    SECTION("BP")
    {
        for (const auto & bpFile : bpFiles)
        {
            trimAndCompressMovie(bpFile, outputDir, numFramesToKeep, bitRateFractions);
        }
    }

    SECTION("DFF")
    {
        for (const auto & dffFile : dffFiles)
        {
            trimAndCompressMovie(dffFile, outputDir, numFramesToKeep, bitRateFractions);
        }
    }

    isx::CoreShutdown();
}

#endif
