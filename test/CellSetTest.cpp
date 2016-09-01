#include "isxCellSet.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"

namespace
{

/// Checks that two traces are equal use Catch REQUIRE statements.
///
/// This compares both data values and meta-data (e.g. timing info).
///
/// \param  inActual    The actual trace.
/// \param  inExpected  The expected trace.
template <typename T>
void
requireEqualTraces(
        const std::shared_ptr<isx::Trace<T>> & inActual,
        const std::shared_ptr<isx::Trace<T>> & inExpected)
{
    const isx::TimingInfo timingInfo = inExpected->getTimingInfo();

    REQUIRE(inActual->getTimingInfo() == timingInfo);

    const isx::isize_t numTimes = timingInfo.getNumTimes();

    T * actualValues = inActual->getValues();
    T * expectedValues = inExpected->getValues();
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
    std::string fileName = g_resources["testDataPath"] + "/cellset.isxd";

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
        isx::CellSet cellSet;

        REQUIRE(!cellSet.isValid());
    }

    SECTION("Write constructor")
    {
        isx::SpCellSet_t cellSet = std::make_shared<isx::CellSet>(
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
            isx::SpCellSet_t cellSet = std::make_shared<isx::CellSet>(
                    fileName, timingInfo, spacingInfo);
        }
        isx::SpCellSet_t cellSet = std::make_shared<isx::CellSet>(fileName);

        REQUIRE(cellSet->isValid());
        REQUIRE(cellSet->getFileName() == fileName);
        REQUIRE(cellSet->getTimingInfo() == timingInfo);
        REQUIRE(cellSet->getSpacingInfo() == spacingInfo);
        REQUIRE(cellSet->getNumCells() == 0);
    }

    SECTION("Set data for one cell and check values are correct")
    {
        isx::SpCellSet_t cellSet = std::make_shared<isx::CellSet>(
                fileName, timingInfo, spacingInfo);
        cellSet->setImageAndTrace(0, originalImage, originalTrace);

        REQUIRE(cellSet->getNumCells() == 1);
        REQUIRE(cellSet->isCellValid(0) == true);
        requireEqualImages(cellSet->getImage(0), originalImage);
        requireEqualTraces(cellSet->getTrace(0), originalTrace);
    }

    SECTION("Set data for one cell and check read values are correct")
    {
        {
            isx::SpCellSet_t cellSet = std::make_shared<isx::CellSet>(
                    fileName, timingInfo, spacingInfo);
            cellSet->setImageAndTrace(0, originalImage, originalTrace);
        }
        isx::SpCellSet_t cellSet = std::make_shared<isx::CellSet>(fileName);

        REQUIRE(cellSet->getNumCells() == 1);
        REQUIRE(cellSet->isCellValid(0) == true);
        requireEqualImages(cellSet->getImage(0), originalImage);
        requireEqualTraces(cellSet->getTrace(0), originalTrace);
    }

    SECTION("Set data for 3 cells and check values are correct")
    {
        isx::SpCellSet_t cellSet = std::make_shared<isx::CellSet>(
                fileName, timingInfo, spacingInfo);
        for (size_t i = 0; i < 3; ++i)
        {
            cellSet->setImageAndTrace(i, originalImage, originalTrace);
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
            isx::SpCellSet_t cellSet = std::make_shared<isx::CellSet>(
                    fileName, timingInfo, spacingInfo);
            for (size_t i = 0; i < 3; ++i)
            {
                cellSet->setImageAndTrace(i, originalImage, originalTrace);
            }
            cellSet->setCellValid(0, true);
            cellSet->setCellValid(1, false);
            cellSet->setCellValid(2, true);
        }
        isx::SpCellSet_t cellSet = std::make_shared<isx::CellSet>(fileName);

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
        isx::SpCellSet_t cellSet = std::make_shared<isx::CellSet>(
                fileName, timingInfo, spacingInfo);
        for (size_t i = 0; i < 3; ++i)
        {
            cellSet->setImageAndTrace(i, originalImage, originalTrace);
        }

        isx::CellSet::GetTraceCB_t callBack = [originalTrace](const isx::SpFTrace_t inTrace)
        {
            requireEqualTraces(inTrace, originalTrace);
        };
        for (size_t i = 0; i < 3; ++i)
        {
            cellSet->getTraceAsync(i, callBack);
        }
    }

    SECTION("Read image data for 3 cells asynchronously")
    {
        isx::SpCellSet_t cellSet = std::make_shared<isx::CellSet>(
                fileName, timingInfo, spacingInfo);
        for (size_t i = 0; i < 3; ++i)
        {
            cellSet->setImageAndTrace(i, originalImage, originalTrace);
        }

        isx::CellSet::GetImageCB_t callBack = [originalImage](const isx::SpImage_t inImage)
        {
            requireEqualImages(inImage, originalImage);
        };
        for (size_t i = 0; i < 3; ++i)
        {
            cellSet->getImageAsync(i, callBack);
        }
    }

    isx::CoreShutdown();

}
