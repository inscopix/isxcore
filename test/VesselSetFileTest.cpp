#include "isxVesselSetFile.h"
#include "catch.hpp"
#include "isxTest.h"

#include <cstring>

void
writeDefaultVessels(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        isx::Image & inImage,
        isx::Trace<float> & inTrace,
        const size_t inNumVessels)
{
    isx::VesselSetFile file(inFileName, inTimingInfo, inSpacingInfo);
    for (size_t i = 0; i < inNumVessels; ++i)
    {
        file.writeVesselData(i, inImage, inTrace);
    }
    file.closeForWriting();
}

TEST_CASE("VesselSetFileTest", "[core-internal]")
{
    std::string fileName = g_resources["unitTestDataPath"] + "/vesselset.isxd";
    
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
        isx::VesselSetFile file;
        REQUIRE(!file.isValid());
    }

    SECTION("Constructor for new file")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        REQUIRE(file.numberOfVessels() == 0);
        REQUIRE(file.getTimingInfo() == timingInfo);
        REQUIRE(file.getSpacingInfo() == spacingInfo);
        file.closeForWriting();
    }

    SECTION("Constructor for existing file")
    {
        {
            isx::VesselSetFile newFile(fileName, timingInfo, spacingInfo);
            REQUIRE(newFile.isValid());
            newFile.closeForWriting();
        }
        isx::VesselSetFile existingFile(fileName);
        REQUIRE(existingFile.isValid());
        REQUIRE(existingFile.numberOfVessels() == 0);
    }

    SECTION("Write vessel data")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        REQUIRE(file.numberOfVessels() == 0);        

        // Write to file
        /// Cell trace with default name
        file.writeVesselData(0, originalImage, originalTrace);
        REQUIRE(file.numberOfVessels() == 1);
        REQUIRE(file.getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(file.getVesselName(0).compare("") == 0);

        /// Cell trace with given name
        file.writeVesselData(1, originalImage, originalTrace, "testName");
        REQUIRE(file.numberOfVessels() == 2);
        REQUIRE(file.getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(file.getVesselName(1).compare("testName") == 0);
        file.closeForWriting();
    }

    SECTION("Read trace")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        file.writeVesselData(0, originalImage, originalTrace);
        REQUIRE(file.numberOfVessels() == 1);
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
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        file.writeVesselData(0, originalImage, originalTrace);
        REQUIRE(file.numberOfVessels() == 1);
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

    SECTION("Validate/Invalidate vessel")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        file.writeVesselData(0, originalImage, originalTrace);
        REQUIRE(file.numberOfVessels() == 1);
        REQUIRE(file.getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        file.setVesselStatus(0, isx::VesselSet::VesselStatus::REJECTED);
        REQUIRE(file.getVesselStatus(0) == isx::VesselSet::VesselStatus::REJECTED);
        file.setVesselStatus(0, isx::VesselSet::VesselStatus::ACCEPTED);
        REQUIRE(file.getVesselStatus(0) == isx::VesselSet::VesselStatus::ACCEPTED);
        file.closeForWriting();
    }

    SECTION("Set/Get vessel name")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo);
        REQUIRE(file.isValid());
        file.writeVesselData(0, originalImage, originalTrace);
        REQUIRE(file.numberOfVessels() == 1);
        REQUIRE(file.getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(file.getVesselName(0).compare("") == 0);

        file.setVesselName(0, "newVesselName");
        REQUIRE(file.getVesselName(0).compare("newVesselName") == 0);
        file.closeForWriting();
    }

    SECTION("Write 10 vessels then read it back in")
    {
        {
            isx::VesselSetFile file(fileName, timingInfo, spacingInfo);
            for (size_t i = 0; i < 10; ++i)
            {
                file.writeVesselData(i, originalImage, originalTrace);
            }
            file.closeForWriting();
        }

        isx::VesselSetFile file(fileName);

        REQUIRE(file.isValid());
        REQUIRE(file.getTimingInfo() == timingInfo);
        REQUIRE(file.getSpacingInfo() == spacingInfo);
        REQUIRE(file.numberOfVessels() == 10);

        for (size_t i = 0; i < 10; ++i)
        {
            REQUIRE(file.getVesselStatus(i) == isx::VesselSet::VesselStatus::UNDECIDED);
            REQUIRE(file.getVesselName(i) == "V" + std::to_string(i));

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

    SECTION("Modify image / vessel data after calling closeForWriting")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo);
        file.writeVesselData(0, originalImage, originalTrace);
        file.closeForWriting();

        ISX_REQUIRE_EXCEPTION(
            file.writeVesselData(0, originalImage, originalTrace),
            isx::ExceptionFileIO,
            "Writing data after file was closed for writing." + fileName);
    }

    SECTION("Modify cell validity after calling closeForWriting")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo);
        file.writeVesselData(0, originalImage, originalTrace);
        file.closeForWriting();

        ISX_REQUIRE_EXCEPTION(
            file.setVesselStatus(0, isx::VesselSet::VesselStatus::ACCEPTED),
            isx::ExceptionFileIO,
            "Writing data after file was closed for writing." + fileName);
    }

    SECTION("Modify cell name after calling closeForWriting")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo);
        file.writeVesselData(0, originalImage, originalTrace);
        file.closeForWriting();
        ISX_REQUIRE_EXCEPTION(
            file.setVesselName(0, "newName"),
            isx::ExceptionFileIO,
            "Writing data after file was closed for writing." + fileName);
    }

    SECTION("Check default names for 1 cell")
    {
        writeDefaultVessels(fileName, timingInfo, spacingInfo, originalImage, originalTrace, 1);

        isx::VesselSetFile file(fileName);
        REQUIRE(file.getVesselName(0) == "V0");
    }

    SECTION("Check default names for 9 cells")
    {
        const size_t numCells = 9;
        writeDefaultVessels(fileName, timingInfo, spacingInfo, originalImage, originalTrace, numCells);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numCells; ++i)
        {
            REQUIRE(file.getVesselName(i) == "V" + std::to_string(i));
        }
    }

    SECTION("Check default names for 10 cells")
    {
        const size_t numCells = 10;
        writeDefaultVessels(fileName, timingInfo, spacingInfo, originalImage, originalTrace, numCells);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numCells; ++i)
        {
            REQUIRE(file.getVesselName(i) == "V" + std::to_string(i));
        }
    }

    SECTION("Check default names for 11 cells")
    {
        const size_t numCells = 11;
        writeDefaultVessels(fileName, timingInfo, spacingInfo, originalImage, originalTrace, numCells);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numCells; ++i)
        {
            if (i < 10)
            {
                REQUIRE(file.getVesselName(i) == "V0" + std::to_string(i));
            }
            else
            {
                REQUIRE(file.getVesselName(i) == "V" + std::to_string(i));
            }
        }
    }

    SECTION("Check default names for 100 cells")
    {
        const size_t numCells = 100;
        writeDefaultVessels(fileName, timingInfo, spacingInfo, originalImage, originalTrace, numCells);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numCells; ++i)
        {
            if (i < 10)
            {
                REQUIRE(file.getVesselName(i) == "V0" + std::to_string(i));
            }
            else
            {
                REQUIRE(file.getVesselName(i) == "V" + std::to_string(i));
            }
        }
    }

    SECTION("Check default names for 101 cells")
    {
        const size_t numCells = 101;
        writeDefaultVessels(fileName, timingInfo, spacingInfo, originalImage, originalTrace, numCells);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numCells; ++i)
        {
            if (i < 10)
            {
                REQUIRE(file.getVesselName(i) == "V00" + std::to_string(i));
            }
            else if (i < 100)
            {
                REQUIRE(file.getVesselName(i) == "V0" + std::to_string(i));
            }
            else
            {
                REQUIRE(file.getVesselName(i) == "V" + std::to_string(i));
            }
        }
    }
}
