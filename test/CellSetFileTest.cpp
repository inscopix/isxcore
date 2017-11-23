#include "isxCellSetFile.h"
#include "catch.hpp"
#include "isxTest.h"

#include <cstring>

void
writeDefaultCells(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        isx::Image & inImage,
        isx::Trace<float> & inTrace,
        const size_t inNumCells)
{
    isx::CellSetFile file(inFileName, inTimingInfo, inSpacingInfo);
    for (size_t i = 0; i < inNumCells; ++i)
    {
        file.writeCellData(i, inImage, inTrace);
    }
    file.closeForWriting();
}

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
    std::memset(originalPixels, 0, sizeof(float) * originalImage.getSpacingInfo().getTotalNumPixels());
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
        file.closeForWriting();
    }

    SECTION("Constructor for existing file")
    {
        {
            isx::CellSetFile newFile(fileName, timingInfo, spacingInfo);
            REQUIRE(newFile.isValid());
            newFile.closeForWriting();
        }
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
        REQUIRE(file.getCellStatus(0) == isx::CellSet::CellStatus::UNDECIDED);
        REQUIRE(file.getCellName(0).compare("") == 0);

        /// Cell trace with given name
        file.writeCellData(1, originalImage, originalTrace, "testName");
        REQUIRE(file.numberOfCells() == 2);
        REQUIRE(file.getCellStatus(0) == isx::CellSet::CellStatus::UNDECIDED);
        REQUIRE(file.getCellName(1).compare("testName") == 0);
        file.closeForWriting();
    }

    SECTION("Read trace")
    {
        isx::CellSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        file.writeCellData(0, originalImage, originalTrace);  
        REQUIRE(file.numberOfCells() == 1);
        file.closeForWriting();

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
        file.closeForWriting();

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
        REQUIRE(file.getCellStatus(0) == isx::CellSet::CellStatus::UNDECIDED);
        file.setCellStatus(0, isx::CellSet::CellStatus::REJECTED);
        REQUIRE(file.getCellStatus(0) == isx::CellSet::CellStatus::REJECTED);
        file.setCellStatus(0, isx::CellSet::CellStatus::ACCEPTED);
        REQUIRE(file.getCellStatus(0) == isx::CellSet::CellStatus::ACCEPTED);
        file.closeForWriting();
    }

    SECTION("Set/Get cell name")
    {
        isx::CellSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        file.writeCellData(0, originalImage, originalTrace);  
        REQUIRE(file.numberOfCells() == 1);
        REQUIRE(file.getCellStatus(0) == isx::CellSet::CellStatus::UNDECIDED);
        REQUIRE(file.getCellName(0).compare("") == 0);

        file.setCellName(0, "newName");
        REQUIRE(file.getCellName(0).compare("newName") == 0);
        file.closeForWriting();
    }

    SECTION("Write 10 cells then read it back in")
    {
        {
            isx::CellSetFile file(fileName, timingInfo, spacingInfo);
            for (size_t i = 0; i < 10; ++i)
            {
                file.writeCellData(i, originalImage, originalTrace);
            }
            file.closeForWriting();
        }

        isx::CellSetFile file(fileName);

        REQUIRE(file.isValid());
        REQUIRE(file.getTimingInfo() == timingInfo);
        REQUIRE(file.getSpacingInfo() == spacingInfo);
        REQUIRE(file.numberOfCells() == 10);

        for (size_t i = 0; i < 10; ++i)
        {
            REQUIRE(file.getCellStatus(i) == isx::CellSet::CellStatus::UNDECIDED);
            REQUIRE(file.getCellName(i) == "C" + std::to_string(i));

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

    SECTION("Modify image / cell data after calling closeForWriting")
    {
        isx::CellSetFile file(fileName, timingInfo, spacingInfo);
        file.writeCellData(0, originalImage, originalTrace);
        file.closeForWriting();
        
        ISX_REQUIRE_EXCEPTION(
            file.writeCellData(0, originalImage, originalTrace),
            isx::ExceptionFileIO,
            "Writing data after file was closed for writing." + fileName);
    }
    
    SECTION("Modify cell validity after calling closeForWriting")
    {
        isx::CellSetFile file(fileName, timingInfo, spacingInfo);
        file.writeCellData(0, originalImage, originalTrace);
        file.closeForWriting();
        
        ISX_REQUIRE_EXCEPTION(
            file.setCellStatus(0, isx::CellSet::CellStatus::ACCEPTED),
            isx::ExceptionFileIO,
            "Writing data after file was closed for writing." + fileName);
    }

    SECTION("Modify cell name after calling closeForWriting")
    {
        isx::CellSetFile file(fileName, timingInfo, spacingInfo);
        file.writeCellData(0, originalImage, originalTrace);
        file.closeForWriting();
        ISX_REQUIRE_EXCEPTION(
            file.setCellName(0, "newName"),
            isx::ExceptionFileIO,
            "Writing data after file was closed for writing." + fileName);
    }

    SECTION("Check default names for 1 cell")
    {
        writeDefaultCells(fileName, timingInfo, spacingInfo, originalImage, originalTrace, 1);

        isx::CellSetFile file(fileName);
        REQUIRE(file.getCellName(0) == "C0");
    }

    SECTION("Check default names for 9 cells")
    {
        const size_t numCells = 9;
        writeDefaultCells(fileName, timingInfo, spacingInfo, originalImage, originalTrace, numCells);

        isx::CellSetFile file(fileName);
        for (size_t i = 0; i < numCells; ++i)
        {
            REQUIRE(file.getCellName(i) == "C" + std::to_string(i));
        }
    }

    SECTION("Check default names for 10 cells")
    {
        const size_t numCells = 10;
        writeDefaultCells(fileName, timingInfo, spacingInfo, originalImage, originalTrace, numCells);

        isx::CellSetFile file(fileName);
        for (size_t i = 0; i < numCells; ++i)
        {
            REQUIRE(file.getCellName(i) == "C" + std::to_string(i));
        }
    }

    SECTION("Check default names for 11 cells")
    {
        const size_t numCells = 11;
        writeDefaultCells(fileName, timingInfo, spacingInfo, originalImage, originalTrace, numCells);

        isx::CellSetFile file(fileName);
        for (size_t i = 0; i < numCells; ++i)
        {
            if (i < 10)
            {
                REQUIRE(file.getCellName(i) == "C0" + std::to_string(i));
            }
            else
            {
                REQUIRE(file.getCellName(i) == "C" + std::to_string(i));
            }
        }
    }

    SECTION("Check default names for 100 cells")
    {
        const size_t numCells = 100;
        writeDefaultCells(fileName, timingInfo, spacingInfo, originalImage, originalTrace, numCells);

        isx::CellSetFile file(fileName);
        for (size_t i = 0; i < numCells; ++i)
        {
            if (i < 10)
            {
                REQUIRE(file.getCellName(i) == "C0" + std::to_string(i));
            }
            else
            {
                REQUIRE(file.getCellName(i) == "C" + std::to_string(i));
            }
        }
    }

    SECTION("Check default names for 101 cells")
    {
        const size_t numCells = 101;
        writeDefaultCells(fileName, timingInfo, spacingInfo, originalImage, originalTrace, numCells);

        isx::CellSetFile file(fileName);
        for (size_t i = 0; i < numCells; ++i)
        {
            if (i < 10)
            {
                REQUIRE(file.getCellName(i) == "C00" + std::to_string(i));
            }
            else if (i < 100)
            {
                REQUIRE(file.getCellName(i) == "C0" + std::to_string(i));
            }
            else
            {
                REQUIRE(file.getCellName(i) == "C" + std::to_string(i));
            }
        }
    }

}
