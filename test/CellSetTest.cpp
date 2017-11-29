#include "isxCellSetFactory.h"
#include "isxCellSetSimple.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"
#include "isxMovieFactory.h"
#include "isxProject.h"
#include <cstring>
#include <atomic>

TEST_CASE("CellSetTest", "[core]")
{
    std::string fileName = g_resources["unitTestDataPath"] + "/cellset.isxd";
    std::remove(fileName.c_str());

    isx::Time start;
    isx::DurationInSeconds step(50, 1000);
    isx::isize_t numFrames = 5;
    isx::TimingInfo timingInfo(start, step, numFrames);

    isx::SizeInPixels_t numPixels(4, 3);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

    isx::SpImage_t originalImage = std::make_shared<isx::Image>(
            spacingInfo,
            sizeof(float) * spacingInfo.getNumColumns(),
            1,
            isx::DataType::F32);
    float * originalPixels = originalImage->getPixelsAsF32();
    std::memset(originalPixels, 0, sizeof(float) * spacingInfo.getTotalNumPixels());
    originalPixels[0] = 1.0f;
    originalPixels[1] = 2.5f;

    isx::SpFTrace_t originalTrace = std::make_shared<isx::Trace<float>>(timingInfo);
    float * originalValues = originalTrace->getValues();
    float val = 0.0f;
    for (isx::isize_t i(0); i < timingInfo.getNumTimes(); ++i)
    {
        originalValues[i] = val;
        val += 0.01f;
    }


    isx::CoreInitialize();

    SECTION("Empty constructor")
    {
        isx::CellSetSimple cellSet;

        REQUIRE(!cellSet.isValid());
    }

    SECTION("Write constructor")
    {
        isx::SpCellSet_t cellSet = isx::writeCellSet(
                fileName, timingInfo, spacingInfo);

        REQUIRE(cellSet->isValid());
        REQUIRE(cellSet->getFileName() == fileName);
        REQUIRE(cellSet->getTimingInfo() == timingInfo);
        REQUIRE(cellSet->getSpacingInfo() == spacingInfo);
        REQUIRE(cellSet->getNumCells() == 0);
        REQUIRE(!cellSet->isRoiSet());
        cellSet->closeForWriting();
    }

    SECTION("Read constructor")
    {
        {
            isx::SpCellSet_t cellSet = isx::writeCellSet(
                    fileName, timingInfo, spacingInfo);
            cellSet->closeForWriting();
        }
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);

        REQUIRE(cellSet->isValid());
        REQUIRE(cellSet->getFileName() == fileName);
        REQUIRE(cellSet->getTimingInfo() == timingInfo);
        REQUIRE(cellSet->getSpacingInfo() == spacingInfo);
        REQUIRE(cellSet->getNumCells() == 0);
        REQUIRE(!cellSet->isRoiSet());
    }

    SECTION("Create an ROI set")
    {
        {
            isx::SpCellSet_t cellSet = isx::writeCellSet(
                    fileName, timingInfo, spacingInfo, true);
            cellSet->closeForWriting();
        }
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);

        REQUIRE(cellSet->isValid());
        REQUIRE(cellSet->getFileName() == fileName);
        REQUIRE(cellSet->getTimingInfo() == timingInfo);
        REQUIRE(cellSet->getSpacingInfo() == spacingInfo);
        REQUIRE(cellSet->getNumCells() == 0);
        REQUIRE(cellSet->isRoiSet());
    }

    SECTION("Set data for one cell and check values are correct")
    {
        isx::SpCellSet_t cellSet = isx::writeCellSet(
                fileName, timingInfo, spacingInfo);
        cellSet->writeImageAndTrace(0, originalImage, originalTrace);
        cellSet->closeForWriting();

        REQUIRE(cellSet->getNumCells() == 1);
        REQUIRE(cellSet->getCellStatus(0) == isx::CellSet::CellStatus::UNDECIDED);
        requireEqualImages(cellSet->getImage(0), originalImage);
        requireEqualTraces(cellSet->getTrace(0), originalTrace);
    }

    SECTION("Set data for one cell and check read values are correct")
    {
        {
            isx::SpCellSet_t cellSet = isx::writeCellSet(
                    fileName, timingInfo, spacingInfo);
            cellSet->writeImageAndTrace(0, originalImage, originalTrace, "mycell");
            cellSet->closeForWriting();
        }
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);

        REQUIRE(cellSet->getNumCells() == 1);
        REQUIRE(cellSet->getCellStatus(0) == isx::CellSet::CellStatus::UNDECIDED);
        REQUIRE(cellSet->getCellName(0).compare("mycell") == 0);
        requireEqualImages(cellSet->getImage(0), originalImage);
        requireEqualTraces(cellSet->getTrace(0), originalTrace);
    }
    SECTION("Set/Get cell name")
    {
        isx::SpCellSet_t cellSet = isx::writeCellSet(
                    fileName, timingInfo, spacingInfo);
        cellSet->writeImageAndTrace(0, originalImage, originalTrace);
        REQUIRE(cellSet->getNumCells() == 1);
        REQUIRE(cellSet->getCellStatus(0) == isx::CellSet::CellStatus::UNDECIDED);
        REQUIRE(cellSet->getCellName(0).compare("") == 0);

        cellSet->setCellName(0, "newName");
        REQUIRE(cellSet->getCellName(0).compare("newName") == 0);
        cellSet->closeForWriting();
    }

    SECTION("Set data for 3 cells and check values are correct")
    {
        isx::SpCellSet_t cellSet = isx::writeCellSet(
                fileName, timingInfo, spacingInfo);
        for (size_t i = 0; i < 3; ++i)
        {
            cellSet->writeImageAndTrace(i, originalImage, originalTrace);
        }
        cellSet->setCellStatus(0, isx::CellSet::CellStatus::ACCEPTED);
        cellSet->setCellStatus(1, isx::CellSet::CellStatus::UNDECIDED);
        cellSet->setCellStatus(2, isx::CellSet::CellStatus::REJECTED);
        cellSet->closeForWriting();

        REQUIRE(cellSet->getNumCells() == 3);
        REQUIRE(cellSet->getCellStatus(0) == isx::CellSet::CellStatus::ACCEPTED);
        REQUIRE(cellSet->getCellStatus(1) == isx::CellSet::CellStatus::UNDECIDED);
        REQUIRE(cellSet->getCellStatus(2) == isx::CellSet::CellStatus::REJECTED);

        for (size_t i = 0; i < 3; ++i)
        {
            requireEqualImages(cellSet->getImage(i), originalImage);
            requireEqualTraces(cellSet->getTrace(i), originalTrace);
        }
    }

    SECTION("Set data for 3 cells and check read values are correct")
    {
        {
            isx::SpCellSet_t cellSet = isx::writeCellSet(
                    fileName, timingInfo, spacingInfo);
            for (size_t i = 0; i < 3; ++i)
            {
                cellSet->writeImageAndTrace(i, originalImage, originalTrace);
            }
            cellSet->setCellStatus(0, isx::CellSet::CellStatus::ACCEPTED);
            cellSet->setCellStatus(1, isx::CellSet::CellStatus::UNDECIDED);
            cellSet->setCellStatus(2, isx::CellSet::CellStatus::REJECTED);
            cellSet->closeForWriting();
        }
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);

        REQUIRE(cellSet->getNumCells() == 3);
        REQUIRE(cellSet->getCellStatus(0) == isx::CellSet::CellStatus::ACCEPTED);
        REQUIRE(cellSet->getCellStatus(1) == isx::CellSet::CellStatus::UNDECIDED);
        REQUIRE(cellSet->getCellStatus(2) == isx::CellSet::CellStatus::REJECTED);

        for (size_t i = 0; i < 3; ++i)
        {
            requireEqualImages(cellSet->getImage(i), originalImage);
            requireEqualTraces(cellSet->getTrace(i), originalTrace);
        }
    }

    SECTION("Read trace data for 3 cells asynchronously")
    {
        std::atomic_int doneCount(0);
        size_t numCells = 3;
        isx::SpCellSet_t cellSet = isx::writeCellSet(
                fileName, timingInfo, spacingInfo);
        for (size_t i = 0; i < 3; ++i)
        {
            cellSet->writeImageAndTrace(i, originalImage, originalTrace);
        }
        cellSet->closeForWriting();

        isx::CellSet::CellSetGetTraceCB_t callBack = [originalTrace, &doneCount](isx::AsyncTaskResult<isx::SpFTrace_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            requireEqualTraces(inAsyncTaskResult.get(), originalTrace);
            ++doneCount;

        };
        for (size_t i = 0; i < numCells; ++i)
        {
            cellSet->getTraceAsync(i, callBack);
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numCells))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == int(numCells));
    }

    SECTION("Read image data for 3 cells asynchronously")
    {
        std::atomic_int doneCount(0);
        size_t numCells = 3;

        isx::SpCellSet_t cellSet = isx::writeCellSet(
                fileName, timingInfo, spacingInfo);
        for (size_t i = 0; i < 3; ++i)
        {
            cellSet->writeImageAndTrace(i, originalImage, originalTrace);
        }
        cellSet->closeForWriting();

        isx::CellSet::CellSetGetImageCB_t callBack = [originalImage, &doneCount](isx::AsyncTaskResult<isx::SpImage_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            requireEqualImages(inAsyncTaskResult.get(), originalImage);
            ++doneCount;
        };
        for (size_t i = 0; i < 3; ++i)
        {
            cellSet->getImageAsync(i, callBack);
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numCells))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == int(numCells));
    }

    isx::CoreShutdown();

}

TEST_CASE("CellSetSynth", "[data][!hide]")
{

    isx::CoreInitialize();

    isx::HistoricalDetails hd("Imported", "");

    SECTION("Test with completely filled frame")
    {
        const std::string cellSetFile = g_resources["unitTestDataPath"] + "/cellset-full-frame.isxd";
        std::remove(cellSetFile.c_str());

        const std::string movieFile = g_resources["unitTestDataPath"] + "/movie-full-frame.isxd";
        std::remove(movieFile.c_str());

        const std::string projectFile = g_resources["unitTestDataPath"] + "/project-full-frame.isxp";
        std::remove(projectFile.c_str());

        const isx::SpacingInfo spacingInfo(isx::SizeInPixels_t(6, 5));
        const isx::TimingInfo timingInfo(isx::Time(), isx::DurationInSeconds(1), 7);

        isx::SpCellSet_t cellSet = writeCellSet(cellSetFile, timingInfo, spacingInfo);
        isx::SpWritableMovie_t movie = isx::writeMosaicMovie(
                movieFile,
                timingInfo,
                spacingInfo,
                isx::DataType::F32);
        isx::SpImage_t image = std::make_shared<isx::Image>(
                spacingInfo,
                sizeof(float) * spacingInfo.getNumColumns(),
                1,
                isx::DataType::F32);
        float imageData[] = {
            1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1,
            1, 1, 0, 1, 1, 1,
            1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1 };
        std::memcpy(image->getPixels(), reinterpret_cast<char *>(imageData), image->getImageSizeInBytes());

        isx::SpFTrace_t trace = std::make_shared<isx::FTrace_t>(timingInfo);
        float * traceData = trace->getValues();
        for (isx::isize_t t = 0; t < timingInfo.getNumTimes(); ++t)
        {
            traceData[t] = float(t % 2);

            isx::SpVideoFrame_t frame = movie->makeVideoFrame(t);
            float * frameData = frame->getPixelsAsF32();
            for (isx::isize_t p = 0; p < spacingInfo.getTotalNumPixels(); ++p)
            {
                frameData[p] = imageData[p] * traceData[t];
            }
            movie->writeFrame(frame);
        }

        cellSet->writeImageAndTrace(0, image, trace);
        cellSet->closeForWriting();

        isx::Project project(projectFile, "Full Frame");
        auto s = project.importDataSetInRoot(
                "/movie-full-frame",
                isx::DataSet::Type::MOVIE,
                movieFile,
                hd,
                {
                  {isx::DataSet::PROP_DATA_MIN, isx::Variant(0.f)},
                  {isx::DataSet::PROP_DATA_MAX, isx::Variant(1.f)},
                  {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                  {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                });
        isx::DataSet::Properties prop;
        auto ds = std::make_shared<isx::Series>(
            "cellset-full-frame",
            isx::DataSet::Type::CELLSET,
            cellSetFile,
            hd,
            prop);
        s->insertUnitarySeries(ds);
        project.save();
    }

    isx::CoreShutdown();
}
