#include "isxCellSetFactory.h"
#include "isxCellSetSimple.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"
#include "isxMovieFactory.h"
#include "isxProject.h"
#include <cstring>
#include <atomic>

namespace
{

/// Checks that two traces are equal use Catch REQUIRE statements.
///
/// This compares both data values and meta-data (e.g. timing info).
///
/// \param  inActual    The actual trace.
/// \param  inExpected  The expected trace.
void
requireEqualTraces(
        const isx::SpFTrace_t & inActual,
        const isx::SpFTrace_t & inExpected)
{
    const isx::TimingInfo timingInfo = inExpected->getTimingInfo();

    REQUIRE(inActual->getTimingInfo() == timingInfo);

    const isx::isize_t numTimes = timingInfo.getNumTimes();

    float * actualValues = inActual->getValues();
    float * expectedValues = inExpected->getValues();
    for (isx::isize_t i = 0; i < numTimes; ++i)
    {
        REQUIRE(actualValues[i] == expectedValues[i]);
    }
}

/// Checks that two traces are equal use Catch REQUIRE statements.
///
/// This compares both data values and meta-data (e.g. spacing info).
/// The data types must also match.
///
/// \param  inActual    The actual image.
/// \param  inExpected  The expected image.
void
requireEqualImages(
        const isx::SpImage_t & inActual,
        const isx::SpImage_t & inExpected)
{
    const isx::DataType dataType = inExpected->getDataType();
    const isx::SpacingInfo spacingInfo = inExpected->getSpacingInfo();

    REQUIRE(inActual->getDataType() == dataType);
    REQUIRE(inActual->getSpacingInfo() == spacingInfo);

    const isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
    switch (dataType)
    {
        case isx::DataType::U16:
        {
            uint16_t * actualPixels = inActual->getPixelsAsU16();
            uint16_t * expectedPixels = inExpected->getPixelsAsU16();
            for (isx::isize_t i = 0; i < numPixels; ++i)
            {
                REQUIRE(actualPixels[i] == expectedPixels[i]);
            }
            break;
        }
        case isx::DataType::F32:
        {
            float * actualPixels = inActual->getPixelsAsF32();
            float * expectedPixels = inExpected->getPixelsAsF32();
            for (isx::isize_t i = 0; i < numPixels; ++i)
            {
                REQUIRE(actualPixels[i] == expectedPixels[i]);
            }
            break;
        }
        default:
        {
            ISX_THROW(isx::ExceptionDataIO,
                    "Data type not recognized: ", dataType);
        }
    }
}

}

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
    }

    SECTION("Read constructor")
    {
        {
            isx::SpCellSet_t cellSet = isx::writeCellSet(
                    fileName, timingInfo, spacingInfo);
        }
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);

        REQUIRE(cellSet->isValid());
        REQUIRE(cellSet->getFileName() == fileName);
        REQUIRE(cellSet->getTimingInfo() == timingInfo);
        REQUIRE(cellSet->getSpacingInfo() == spacingInfo);
        REQUIRE(cellSet->getNumCells() == 0);
    }

    SECTION("Set data for one cell and check values are correct")
    {
        isx::SpCellSet_t cellSet = isx::writeCellSet(
                fileName, timingInfo, spacingInfo);
        cellSet->writeImageAndTrace(0, originalImage, originalTrace);

        REQUIRE(cellSet->getNumCells() == 1);
        REQUIRE(cellSet->isCellValid(0) == true);
        requireEqualImages(cellSet->getImage(0), originalImage);
        requireEqualTraces(cellSet->getTrace(0), originalTrace);
    }

    SECTION("Set data for one cell and check read values are correct")
    {
        {
            isx::SpCellSet_t cellSet = isx::writeCellSet(
                    fileName, timingInfo, spacingInfo);
            cellSet->writeImageAndTrace(0, originalImage, originalTrace, "mycell");
        }
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);

        REQUIRE(cellSet->getNumCells() == 1);
        REQUIRE(cellSet->isCellValid(0) == true);
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
        REQUIRE(cellSet->isCellValid(0) == true);
        REQUIRE(cellSet->getCellName(0).compare("C0") == 0);

        cellSet->setCellName(0, "newName");
        REQUIRE(cellSet->getCellName(0).compare("newName") == 0);
        
    }

    SECTION("Set data for 3 cells and check values are correct")
    {
        isx::SpCellSet_t cellSet = isx::writeCellSet(
                fileName, timingInfo, spacingInfo);
        for (size_t i = 0; i < 3; ++i)
        {
            cellSet->writeImageAndTrace(i, originalImage, originalTrace);
        }
        cellSet->setCellValid(0, true);
        cellSet->setCellValid(1, false);
        cellSet->setCellValid(2, true);

        REQUIRE(cellSet->getNumCells() == 3);
        REQUIRE(cellSet->isCellValid(0) == true);
        REQUIRE(cellSet->isCellValid(1) == false);
        REQUIRE(cellSet->isCellValid(2) == true);

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
            cellSet->setCellValid(0, true);
            cellSet->setCellValid(1, false);
            cellSet->setCellValid(2, true);
        }
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);

        REQUIRE(cellSet->getNumCells() == 3);
        REQUIRE(cellSet->isCellValid(0) == true);
        REQUIRE(cellSet->isCellValid(1) == false);
        REQUIRE(cellSet->isCellValid(2) == true);

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

        isx::CellSet::CellSetGetTraceCB_t callBack = [originalTrace, &doneCount](const isx::SpFTrace_t inTrace)
        {
            requireEqualTraces(inTrace, originalTrace);
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

        isx::CellSet::CellSetGetImageCB_t callBack = [originalImage, &doneCount](const isx::SpImage_t inImage)
        {
            requireEqualImages(inImage, originalImage);
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

        isx::Project project(projectFile, "Full Frame");
        project.importDataSet(
                "/movie-full-frame",
                isx::DataSet::Type::MOVIE,
                movieFile,
                {
                  {isx::DataSet::PROP_DATA_MIN, isx::Variant(0.f)},
                  {isx::DataSet::PROP_DATA_MAX, isx::Variant(1.f)},
                  {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                  {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                });
        project.createDataSet(
                "/movie-full-frame/derived/cellset-full-frame",
                isx::DataSet::Type::CELLSET,
                cellSetFile);
        project.save();
    }

    isx::CoreShutdown();
}