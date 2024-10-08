#include "catch.hpp"

#include "isxTest.h"
#include "isxMovieFactory.h"
#include "isxMovieTiffExporter.h"
#include "isxTiffMovie.h"
#include "isxPathUtils.h"
#include "isxCore.h"
#include "isxStopWatch.h"

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

    void
    exportImageAndCheckOutput(const std::string & inInputFile, const std::string & inOutputDir)
    {
        const std::string outputFile = inOutputDir + "/" + isx::getBaseName(inInputFile) + ".tif";
        const isx::SpMovie_t inputMovie = isx::readMovie(inInputFile);
        const isx::MovieTiffExporterParams params({inputMovie}, {outputFile});
        REQUIRE(isx::runMovieTiffExporter(params) == isx::AsyncTaskStatus::COMPLETE);

        const isx::SpVideoFrame_t inputFrame = isx::readImage(inInputFile);
        const isx::SpVideoFrame_t outputFrame = isx::readImage(outputFile);
        requireEqualImages(outputFrame->getImage(), inputFrame->getImage());
    }

} // namespace

TEST_CASE("MovieTiffExportF32Test", "[core][export_tiff]")
{
    std::array<const char *, 1> names =
    { {
        "seriesMovie0.isxd"
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

    std::array<isx::TimingInfo, 3> timingInfos =
    { {
        { isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 1 }
    } };

    isx::SizeInPixels_t sizePixels(4, 3);
    isx::SpacingInfo spacingInfo(sizePixels);

    isx::CoreInitialize();

    SECTION("Export F32 movie series")
    {
        // write isxds
        const auto pixelsPerFrame = int32_t(sizePixels.getWidth() * sizePixels.getHeight());
        std::vector<float> buf(pixelsPerFrame);

        int32_t i = 0;
        for (const auto & fn: filenames)
        {
            createFrameData(buf.data(), int32_t(timingInfos[i].getNumTimes()), pixelsPerFrame, i * 5);
            writeTestF32MovieGeneric(fn, timingInfos[i], spacingInfo, buf.data());
            ++i;
        }

        // export to nwb
        std::vector<isx::SpMovie_t> movies;
        for (const auto & fn: filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieTiffExporterParams params(
            movies,
            exportedTiffFileName);
        isx::runMovieTiffExporter(params);

        // verify exported data
        {
            isx::TiffMovie tiffMovie(exportedTiffFileName);
            REQUIRE(tiffMovie.getFrameHeight() == sizePixels.getHeight());
            REQUIRE(tiffMovie.getFrameWidth() == sizePixels.getWidth());
            REQUIRE(tiffMovie.getDataType() == isx::DataType::F32);
            REQUIRE(tiffMovie.getNumFrames() == 1);

            auto originImage = movies[0]->getFrame(0);
            auto importedImage = tiffMovie.getVideoFrame(0, spacingInfo, timingInfos[0].convertIndexToStartTime(1));
            requireEqualImages(importedImage->getImage(), originImage->getImage());
        }

    }

    isx::CoreShutdown();

    for (const auto & fn: filenames)
    {
        std::remove(fn.c_str());
    }
}

TEST_CASE("MovieTiffExportU16Test", "[core][export_tiff]")
{
    std::array<const char *, 1> names =
    { {
            "seriesMovie0.isxd"
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

    std::array<isx::TimingInfo, 3> timingInfos =
    { {
        { isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 1 }
        } };

    isx::SizeInPixels_t sizePixels(4, 3);
    isx::SpacingInfo spacingInfo(sizePixels);

    isx::CoreInitialize();

    SECTION("Export U16 movie series")
    {
        // write isxds
        const auto pixelsPerFrame = int32_t(sizePixels.getWidth() * sizePixels.getHeight());
        std::vector<uint16_t> buf(pixelsPerFrame);

        int32_t i = 0;
        for (const auto & fn : filenames)
        {
            createFrameData(buf.data(), int32_t(timingInfos[i].getNumTimes()), pixelsPerFrame, i * 5);
            writeTestU16MovieGeneric(fn, timingInfos[i], spacingInfo, buf.data());
            ++i;
        }

        // export to nwb
        std::vector<isx::SpMovie_t> movies;
        for (const auto & fn : filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieTiffExporterParams params(
            movies,
            exportedTiffFileName);
        isx::runMovieTiffExporter(params);


        // verify exported data
        {
            isx::TiffMovie tiffMovie(exportedTiffFileName);
            REQUIRE(tiffMovie.getFrameHeight() == sizePixels.getHeight());
            REQUIRE(tiffMovie.getFrameWidth() == sizePixels.getWidth());
            REQUIRE(tiffMovie.getDataType() == isx::DataType::U16);
            REQUIRE(tiffMovie.getNumFrames() == 1);

            auto originImage = movies[0]->getFrame(0);
            auto importedImage = tiffMovie.getVideoFrame(0, spacingInfo, timingInfos[0].convertIndexToStartTime(1));
            requireEqualImages(importedImage->getImage(), originImage->getImage());
        }

    }

    for (const auto & fn : filenames)
    {
        std::remove(fn.c_str());
    }
}

TEST_CASE("MovieTiffExportU16ComplexTest", "[core][export_tiff]")
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
        for (const auto & fn : filenames)
        {
            createFrameData(buf.data(), int32_t(timingInfos[i].getNumTimes()), pixelsPerFrame, i * 5);
            writeTestU16MovieGeneric(fn, timingInfos[i], spacingInfo, buf.data());
            ++i;
        }

        // export to nwb
        std::vector<isx::SpMovie_t> movies;
        for (const auto & fn : filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieTiffExporterParams params(
            movies,
            exportedTiffFileName);
        isx::runMovieTiffExporter(params);


        // verify exported data
        {
            isx::TiffMovie tiffMovie(exportedTiffFileName);
            REQUIRE(tiffMovie.getFrameHeight() == sizePixels.getHeight());
            REQUIRE(tiffMovie.getFrameWidth() == sizePixels.getWidth());
            REQUIRE(tiffMovie.getDataType() == isx::DataType::U16);
            REQUIRE(tiffMovie.getNumFrames() == 11);

            isx::isize_t frameIndex = 0;
            isx::isize_t movieIndex = 0;

            for (isx::isize_t globalFrameIndex = 0; globalFrameIndex < tiffMovie.getNumFrames(); globalFrameIndex++)
            {
                auto originImage = movies[movieIndex]->getFrame(frameIndex);
                if (originImage->getFrameType() != isx::VideoFrame::Type::VALID)
                    continue;

                auto importedImage = tiffMovie.getVideoFrame(globalFrameIndex, spacingInfo, timingInfos[movieIndex].convertIndexToStartTime(frameIndex));

                requireEqualImages(originImage->getImage(), importedImage->getImage());

                ++frameIndex;
                if (frameIndex == movies[movieIndex]->getTimingInfo().getNumTimes())
                {
                    frameIndex = 0;
                    ++movieIndex;
                }
            }

        }

    }

    isx::CoreShutdown();

    for (const auto & fn : filenames)
    {
        std::remove(fn.c_str());
    }
}

TEST_CASE("MovieTiffExportF32ComplexTest", "[core][export_tiff]")
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
        std::vector<float> buf(pixelsPerFrame * 5);   // longest movie segment has 5 frames

        int32_t i = 0;
        for (const auto & fn : filenames)
        {
            createFrameData(buf.data(), int32_t(timingInfos[i].getNumTimes()), pixelsPerFrame, i * 5);
            writeTestF32MovieGeneric(fn, timingInfos[i], spacingInfo, buf.data());
            ++i;
        }

        // export to nwb
        std::vector<isx::SpMovie_t> movies;
        for (const auto & fn : filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieTiffExporterParams params(
            movies,
            exportedTiffFileName);
        isx::runMovieTiffExporter(params);


        // verify exported data
        {
            isx::TiffMovie tiffMovie(exportedTiffFileName);
            REQUIRE(tiffMovie.getFrameHeight() == sizePixels.getHeight());
            REQUIRE(tiffMovie.getFrameWidth() == sizePixels.getWidth());
            REQUIRE(tiffMovie.getDataType() == isx::DataType::F32);
            REQUIRE(tiffMovie.getNumFrames() == 11);

            isx::isize_t frameIndex = 0;
            isx::isize_t movieIndex = 0;

            for (isx::isize_t globalFrameIndex = 0; globalFrameIndex < tiffMovie.getNumFrames(); globalFrameIndex++)
            {
                auto originImage = movies[movieIndex]->getFrame(frameIndex);
                if (originImage->getFrameType() != isx::VideoFrame::Type::VALID)
                    continue;

                auto importedImage = tiffMovie.getVideoFrame(globalFrameIndex, spacingInfo, timingInfos[movieIndex].convertIndexToStartTime(frameIndex));

                requireEqualImages(originImage->getImage(), importedImage->getImage());

                ++frameIndex;
                if (frameIndex == movies[movieIndex]->getTimingInfo().getNumTimes())
                {
                    frameIndex = 0;
                    ++movieIndex;
                }
            }

        }

    }

    isx::CoreShutdown();

    for (const auto & fn : filenames)
    {
        std::remove(fn.c_str());
    }
}

TEST_CASE("MovieTiffExportSplittedTest", "[core][export_tiff]")
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
        for (const auto & fn : filenames)
        {
            createFrameData(buf.data(), int32_t(timingInfos[i].getNumTimes()), pixelsPerFrame, i * 5);
            writeTestU16MovieGeneric(fn, timingInfos[i], spacingInfo, buf.data());
            ++i;
        }

        // export to nwb
        std::vector<isx::SpMovie_t> movies;
        for (const auto & fn : filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieTiffExporterParams params(
            movies,
            exportedTiffFileName,
            false,
            3);
        isx::runMovieTiffExporter(params);


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
            std::string exportedSecondTiffFileName = dirname + "/" + basename + "_" + isx::convertNumberToPaddedString(1, 1) + "." + extension;

            isx::TiffMovie tiffMovie(exportedSecondTiffFileName);
            REQUIRE(tiffMovie.getFrameHeight() == sizePixels.getHeight());
            REQUIRE(tiffMovie.getFrameWidth() == sizePixels.getWidth());
            REQUIRE(tiffMovie.getDataType() == isx::DataType::U16);
            REQUIRE(tiffMovie.getNumFrames() == 2);

            std::remove(exportedSecondTiffFileName.c_str());
        }
    }

    isx::CoreShutdown();

    for (const auto & fn : filenames)
    {
        std::remove(fn.c_str());
    }
}

TEST_CASE("MovieTiffExportWithDroppedTest", "[core][export_tiff]")
{
    std::array<const char *, 1> names =
    { {
            "seriesMovie0.isxd"
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

    std::vector<isx::isize_t> dropped = { 1 };
    std::array<isx::TimingInfo, 1> timingInfos =
    { {
        { isx::Time(2222, 4, 1, 3, 2, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 2, dropped }
        } };

    isx::SizeInPixels_t sizePixels(4, 3);
    isx::SpacingInfo spacingInfo(sizePixels);

    isx::CoreInitialize();

    SECTION("Export movie series")
    {
        // write isxds
        const auto pixelsPerFrame = int32_t(sizePixels.getWidth() * sizePixels.getHeight());
        std::vector<float> buf(pixelsPerFrame * 5);   // longest movie segment has 5 frames

        int32_t i = 0;
        for (const auto & fn : filenames)
        {
            createFrameData(buf.data(), int32_t(timingInfos[i].getNumTimes()), pixelsPerFrame, i * 5);
            writeTestF32MovieGeneric(fn, timingInfos[i], spacingInfo, buf.data());
            ++i;
        }

        // export to nwb
        std::vector<isx::SpMovie_t> movies;
        for (const auto & fn : filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieTiffExporterParams params(
            movies,
            exportedTiffFileName,
            true);
        isx::runMovieTiffExporter(params, nullptr, [](float) {return false; });


        // verify exported data
        {
            isx::TiffMovie tiffMovie(exportedTiffFileName);
            REQUIRE(tiffMovie.getFrameHeight() == sizePixels.getHeight());
            REQUIRE(tiffMovie.getFrameWidth() == sizePixels.getWidth());
            REQUIRE(tiffMovie.getDataType() == isx::DataType::F32);
            REQUIRE(tiffMovie.getNumFrames() == 2);

            isx::isize_t frameIndex = 0;
            isx::isize_t movieIndex = 0;

            for (isx::isize_t globalFrameIndex = 0; globalFrameIndex < tiffMovie.getNumFrames(); globalFrameIndex++)
            {
                auto originImage = movies[movieIndex]->getFrame(frameIndex);
                auto importedImage = tiffMovie.getVideoFrame(globalFrameIndex, spacingInfo, timingInfos[movieIndex].convertIndexToStartTime(frameIndex));

                if (originImage->getFrameType() == isx::VideoFrame::Type::VALID)
                {
                    requireEqualImages(originImage->getImage(), importedImage->getImage());
                }
                else
                {
                    requireZeroImage(originImage->getImage());
                }

                ++frameIndex;
                if (frameIndex == movies[movieIndex]->getTimingInfo().getNumTimes())
                {
                    frameIndex = 0;
                    ++movieIndex;
                }

            }

        }

    }

    isx::CoreShutdown();

    for (const auto & fn : filenames)
    {
        std::remove(fn.c_str());
    }
}

// Hidden because it write large files and thus takes quite a bit of time and space.
TEST_CASE("MovieTiffExportBigTiff", "[core][!hide]")
{
    const std::string dataDir = g_resources["unitTestDataPath"] + "/export_tiff";
    isx::makeDirectory(dataDir);

    const std::string inputFileName = dataDir + "/input.isxd";

    const std::string outputFileName = dataDir + "/output.tif";
    std::remove(outputFileName.c_str());

    // We want to generate a movie big enough to exceed the 4GB file size limit
    // of non-BigTIFF files.
    const isx::TimingInfo timingInfo(isx::Time(), isx::DurationInSeconds(50, 1000), 2049);
    const isx::isize_t numFrames = timingInfo.getNumTimes();
    const isx::SpacingInfo spacingInfo(isx::SizeInPixels_t(1024, 1024));
    const isx::isize_t numPixels = spacingInfo.getTotalNumPixels();

    isx::CoreInitialize();

    SECTION("Write a big dummy movie")
    {
        const isx::SpMovie_t inputMovie = writeTestU16MovieGeneric(inputFileName, timingInfo, spacingInfo);

        const isx::MovieTiffExporterParams params({ inputMovie }, { outputFileName });
        isx::runMovieTiffExporter(params);

        const isx::SpMovie_t outputMovie = isx::readMovie(outputFileName);
        REQUIRE(outputMovie->getTimingInfo() == timingInfo);
        REQUIRE(outputMovie->getSpacingInfo() == spacingInfo);
        REQUIRE(outputMovie->getDataType() == isx::DataType::U16);

        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            const isx::SpVideoFrame_t inputFrame = inputMovie->getFrame(f);
            const isx::SpVideoFrame_t outputFrame = outputMovie->getFrame(f);
            requireEqualImages(inputFrame->getImage(), outputFrame->getImage());
        }
    }

    isx::CoreShutdown();

    isx::removeDirectory(dataDir);
}

TEST_CASE("MovieTiffExport-one_image", "[core][export_tiff]")
{
    isx::CoreInitialize();

    const std::string snapshotDir = g_resources["unitTestDataPath"] + "/Snapshots";
    const std::string outputDir = g_resources["unitTestDataPath"] + "/export_tiff_output";
    isx::removeDirectory(outputDir);
    isx::makeDirectory(outputDir);

    /* TODO : this test fails due to the update in pixelSize since it is using an old file at 22/10 microns
    SECTION("Export an .isxd file containing a single frame")
    {
        const std::string inputFile = g_resources["unitTestDataPath"] + "/50fr10_l1-3cells_he-Mean Image-v2.isxd";
        exportImageAndCheckOutput(inputFile, outputDir);
    }
    */

    SECTION("Export an nVista 2 XML+TIFF snapshot")
    {
        const std::string inputFile = snapshotDir + "/nVista_PrismC1AAV1_tif/snapshot_20160613_102103.xml";
        exportImageAndCheckOutput(inputFile, outputDir);
    }

    SECTION("Export an nVista 2 TIFF snapshot")
    {
        const std::string inputFile = snapshotDir + "/nVista_ratPFC2_tif/snapshot_20160705_094404.tif";
        exportImageAndCheckOutput(inputFile, outputDir);
    }

    SECTION("Export an nVista 2 XML+HDF5 snapshot")
    {
        const std::string inputFile = snapshotDir + "/nVista_ratPFC3_raw2hdf5_converted/snapshot_20160622_154726.xml";
        exportImageAndCheckOutput(inputFile, outputDir);
    }

    SECTION("Export an nVoke XML snapshot")
    {
        const std::string inputFile = snapshotDir + "/nVoke/snapshot_20170619_160245.xml";
        exportImageAndCheckOutput(inputFile, outputDir);
    }

    SECTION("Export an nVoke TIFF snapshot")
    {
        const std::string inputFile = snapshotDir + "/nVoke/snapshot_20170619_160245.tif";
        exportImageAndCheckOutput(inputFile, outputDir);
    }

    SECTION("Export an nVista 3 TIFF snapshot")
    {
        const std::string inputFile = snapshotDir + "/nVista3/2018-06-19-11-34-37_snap.tiff";
        exportImageAndCheckOutput(inputFile, outputDir);
    }

    isx::removeDirectory(outputDir);

    isx::CoreShutdown();
}

TEST_CASE("MovieTiffExport-benchmark", "[!hide]")
{
    isx::CoreInitialize();

    const std::string outputDir = g_resources["unitTestDataPath"] + "/export_tiff_output";
    isx::removeDirectory(outputDir);
    isx::makeDirectory(outputDir);

    const std::string inputFile = g_resources["unitTestDataPath"] + "/cnmfe/recording_20160517_172653-cropped.isxd";
    const std::string outputFile = outputDir + "/output.tif";

    isx::MovieTiffExporterParams params({isx::readMovie(inputFile)}, {outputFile});

    isx::StopWatch sw;
    sw.start();
    isx::runMovieTiffExporter(params);
    sw.stop();
    ISX_LOG_INFO("TIFF exporter took ", sw.getElapsedMs(), " ms.");

    isx::removeDirectory(outputDir);

    isx::CoreShutdown();
}
