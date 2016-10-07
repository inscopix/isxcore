#include "isxCellSetFile.h"
#include "catch.hpp"
#include "isxTest.h"

TEST_CASE("CellSetFileTest", "[core-internal]")
{
	std::string fileName = g_resources["unitTestDataPath"] + "/cellset.isxd";
    
    isx::Time start;
    isx::DurationInSeconds step(50, 1000);
    isx::isize_t numFrames = 5;
    isx::TimingInfo timingInfo(start, step, numFrames);

    isx::SizeInPixels_t numPixels(4, 3);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

    // Set image
    isx::Image originalImage(
            spacingInfo,
            sizeof(float) * spacingInfo.getNumColumns(),
            1,
            isx::DataType::F32);
    float * originalPixels = originalImage.getPixelsAsF32();
    memset(originalPixels, 0, sizeof(float) * originalImage.getSpacingInfo().getTotalNumPixels());
    originalPixels[0] = 1.0f;
    originalPixels[1] = 2.5f;

    // Set data
    isx::Trace<float> originalTrace(timingInfo);
    float * originalValues = originalTrace.getValues();
    float val = 0.0f;
    for (isx::isize_t i(0); i < timingInfo.getNumTimes(); ++i)
    {
        originalValues[i] = val;
        val += 0.01f;
    }

    SECTION("Empty constructor")
    {
        isx::CellSetFile file;
        REQUIRE(!file.isValid());
    }

    SECTION("Constructor for new file")
    {
        isx::CellSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        REQUIRE(file.numberOfCells() == 0);
        REQUIRE(file.getTimingInfo() == timingInfo);
        REQUIRE(file.getSpacingInfo() == spacingInfo);
    }

    SECTION("Constructor for existing file")
    {
        isx::CellSetFile newFile(fileName, timingInfo, spacingInfo);
        REQUIRE(newFile.isValid());
        isx::CellSetFile existingFile(fileName);
        REQUIRE(existingFile.isValid());
        REQUIRE(existingFile.numberOfCells() == 0);
    }

    SECTION("Write cell data")
    {
        isx::CellSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        REQUIRE(file.numberOfCells() == 0);        

        // Write to file
        /// Cell trace with default name
        file.writeCellData(0, originalImage, originalTrace);
        REQUIRE(file.numberOfCells() == 1);
        REQUIRE(file.isCellValid(0) == true);
        REQUIRE(file.getCellName(0).compare("C0") == 0);

        /// Cell trace with given name
        file.writeCellData(1, originalImage, originalTrace, "testName");
        REQUIRE(file.numberOfCells() == 2);
        REQUIRE(file.isCellValid(1) == true);
        REQUIRE(file.getCellName(1).compare("testName") == 0);
    }

    SECTION("Read trace")
    {
        isx::CellSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        file.writeCellData(0, originalImage, originalTrace);  
        REQUIRE(file.numberOfCells() == 1);
        
        isx::SpFTrace_t trace = file.readTrace(0);
        float * values = trace->getValues();

        REQUIRE(trace->getTimingInfo().getNumTimes() == originalTrace.getTimingInfo().getNumTimes());

        bool valsAreEqual = false;

        for (isx::isize_t i(0); i < trace->getTimingInfo().getNumTimes(); ++i)
        {
            if (values[i] != originalValues[i])
            {
                break;
            }

            if (i == trace->getTimingInfo().getNumTimes() - 1)
            {
                valsAreEqual = true;
            }
        }

        REQUIRE(valsAreEqual);        
    }

    SECTION("Read segmentation image")
    {
        isx::CellSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        file.writeCellData(0, originalImage, originalTrace);  
        REQUIRE(file.numberOfCells() == 1);

        isx::SpImage_t im = file.readSegmentationImage(0);
        bool pixelsAreEqual = false;
        float * pixels = im->getPixelsAsF32();

        REQUIRE(im->getSpacingInfo().getTotalNumPixels() == originalImage.getSpacingInfo().getTotalNumPixels());

        for (isx::isize_t j(0); j < im->getSpacingInfo().getTotalNumPixels(); ++j)
        {
            if (pixels[j] != originalPixels[j])
            {
                break;
            }

            if (j == im->getSpacingInfo().getTotalNumPixels() - 1)
            {
                pixelsAreEqual = true;
            }
        }
        REQUIRE(pixelsAreEqual);
    }

    SECTION("Validate/Invalidate cell")
    {
        isx::CellSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        file.writeCellData(0, originalImage, originalTrace);  
        REQUIRE(file.numberOfCells() == 1);
        REQUIRE(file.isCellValid(0) == true);
        file.setCellValid(0, false);
        REQUIRE(file.isCellValid(0) == false);
        file.setCellValid(0, true);
        REQUIRE(file.isCellValid(0) == true);
    }

    SECTION("Set/Get cell name")
    {
        isx::CellSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        file.writeCellData(0, originalImage, originalTrace);  
        REQUIRE(file.numberOfCells() == 1);
        REQUIRE(file.isCellValid(0) == true);
        REQUIRE(file.getCellName(0).compare("C0") == 0);

        file.setCellName(0, "newName");
        REQUIRE(file.getCellName(0).compare("newName") == 0);
        
    }

    SECTION("Write 10 cells then read it back in")
    {
        {
            isx::CellSetFile file(fileName, timingInfo, spacingInfo);
            for (size_t i = 0; i < 10; ++i)
            {
                file.writeCellData(i, originalImage, originalTrace);
            }
        }

        isx::CellSetFile file(fileName);

        REQUIRE(file.isValid());
        REQUIRE(file.getTimingInfo() == timingInfo);
        REQUIRE(file.getSpacingInfo() == spacingInfo);
        REQUIRE(file.numberOfCells() == 10);

        for (size_t i = 0; i < 10; ++i)
        {
            REQUIRE(file.isCellValid(i) == true);

            isx::SpFTrace_t trace = file.readTrace(i);
            float * values = trace->getValues();

            isx::SpImage_t image = file.readSegmentationImage(i);
            float * pixels = image->getPixelsAsF32();

            for (isx::isize_t i = 0; i < trace->getTimingInfo().getNumTimes(); ++i)
            {
                REQUIRE(values[i] == originalValues[i]);
            }

            for (isx::isize_t i = 0; i < spacingInfo.getTotalNumPixels(); ++i)
            {
                REQUIRE(pixels[i] == originalPixels[i]);
            }
        }
    }

}
