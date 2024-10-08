#include "isxVesselSetFile.h"
#include "catch.hpp"
#include "isxTest.h"

#include <cstring>

void
writeDefaultDiameterVessels(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        const isx::SpImage_t & inImage,
        const isx::SpVesselLine_t & inLineEndpoints,
        const isx::SpFTrace_t & inTrace,
        const isx::SpFTrace_t inCenterTrace,
        const size_t inNumVessels)
{
    isx::VesselSetFile file(inFileName, inTimingInfo, inSpacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
    file.writeImage(inImage);
    for (size_t i = 0; i < inNumVessels; ++i)
    {
        file.writeVesselDiameterData(i, inLineEndpoints, inTrace, inCenterTrace, "");
    }
    file.closeForWriting();
}

void
writeDefaultVelocityVessels(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        const isx::SpImage_t & inImage,
        const isx::SpVesselLine_t & inLineEndpoints,
        const isx::SpFTrace_t & inTrace,
        const isx::SpFTrace_t inDirectionTrace,
        const size_t inNumVessels)
{
    isx::VesselSetFile file(inFileName, inTimingInfo, inSpacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
    file.writeImage(inImage);
    for (size_t i = 0; i < inNumVessels; ++i)
    {
        file.writeVesselVelocityData(i, inLineEndpoints, inTrace, inDirectionTrace, nullptr, "");
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
    isx::SpImage_t originalImage = std::make_shared<isx::Image>(
            spacingInfo,
            sizeof(float) * spacingInfo.getNumColumns(),
            1,
            isx::DataType::F32);
    float * originalPixels = originalImage->getPixelsAsF32();
    std::memset(originalPixels, 0, sizeof(float) * originalImage->getSpacingInfo().getTotalNumPixels());
    originalPixels[0] = 1.0f;
    originalPixels[1] = 2.5f;

    // Set data
    isx::SpFTrace_t originalTrace = std::make_shared<isx::FTrace_t>(timingInfo);
    float * originalValues = originalTrace->getValues();

    isx::SpFTrace_t originalCenterTrace = std::make_shared<isx::FTrace_t>(timingInfo);
    float * originalCenterValues = originalTrace->getValues();

    float val = 0.0f;
    for (isx::isize_t i(0); i < timingInfo.getNumTimes(); ++i)
    {
        originalValues[i] = val;
        originalCenterValues[i] = val * 10.0f;
        val += 0.01f;
    }

    // set line endpoints
    isx::SpVesselLine_t lineEndpoints = std::make_shared<isx::VesselLine>(isx::Contour_t{isx::PointInPixels_t(0,0), isx::PointInPixels_t(1,1)});

    SECTION("Empty constructor")
    {
        isx::VesselSetFile file;
        REQUIRE(!file.isValid());
    }

    SECTION("Constructor for new file")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        REQUIRE(file.isValid());
        REQUIRE(file.numberOfVessels() == 0);
        REQUIRE(file.getTimingInfo() == timingInfo);
        REQUIRE(file.getSpacingInfo() == spacingInfo);
        file.closeForWriting();
    }

    SECTION("Constructor for existing file")
    {
        {
            isx::VesselSetFile newFile(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
            REQUIRE(newFile.isValid());
            newFile.closeForWriting();
        }
        isx::VesselSetFile existingFile(fileName);
        REQUIRE(existingFile.isValid());
        REQUIRE(existingFile.numberOfVessels() == 0);
    }

    SECTION("Write vessel data")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        REQUIRE(file.isValid());
        REQUIRE(file.numberOfVessels() == 0);

        // Write to file
        /// Vessel trace with default name
        file.writeImage(originalImage);
        file.writeVesselDiameterData(0, lineEndpoints, originalTrace, originalCenterTrace);
        REQUIRE(file.numberOfVessels() == 1);
        REQUIRE(file.getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(file.getVesselName(0).compare("") == 0);

        /// Vessel trace with given name
        file.writeVesselDiameterData(1, lineEndpoints, originalTrace, originalCenterTrace, "testName");
        REQUIRE(file.numberOfVessels() == 2);
        REQUIRE(file.getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(file.getVesselName(1).compare("testName") == 0);
        file.closeForWriting();
    }

    SECTION("Read projection image")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        REQUIRE(file.isValid());
        file.writeImage(originalImage);
        file.writeVesselDiameterData(0, lineEndpoints, originalTrace, originalCenterTrace);
        REQUIRE(file.numberOfVessels() == 1);
        file.closeForWriting();

        isx::SpImage_t im = file.readProjectionImage();
        bool pixelsAreEqual = false;
        float * pixels = im->getPixelsAsF32();

        REQUIRE(im->getSpacingInfo().getTotalNumPixels() == originalImage->getSpacingInfo().getTotalNumPixels());

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

    SECTION("Read trace")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        REQUIRE(file.isValid());
        file.writeImage(originalImage);
        file.writeVesselDiameterData(0, lineEndpoints, originalTrace, originalCenterTrace);
        REQUIRE(file.numberOfVessels() == 1);
        file.closeForWriting();

        isx::SpFTrace_t trace = file.readTrace(0);
        float * values = trace->getValues();

        REQUIRE(trace->getTimingInfo().getNumTimes() == originalTrace->getTimingInfo().getNumTimes());

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

    SECTION("Read line endpoints")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        REQUIRE(file.isValid());
        file.writeImage(originalImage);
        file.writeVesselDiameterData(0, lineEndpoints, originalTrace, originalCenterTrace);
        REQUIRE(file.numberOfVessels() == 1);
        file.closeForWriting();

        isx::SpVesselLine_t vesselLineEndpoints = file.readLineEndpoints(0);
        requireEqualVesselLines(vesselLineEndpoints, lineEndpoints);
    }

    SECTION("Validate/Invalidate vessel")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        REQUIRE(file.isValid());
        file.writeImage(originalImage);
        file.writeVesselDiameterData(0, lineEndpoints, originalTrace, originalCenterTrace);
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
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        REQUIRE(file.isValid());
        file.writeImage(originalImage);
        file.writeVesselDiameterData(0, lineEndpoints, originalTrace, originalCenterTrace);
        REQUIRE(file.getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(file.getVesselName(0).compare("") == 0);

        file.setVesselName(0, "newVesselName");
        REQUIRE(file.getVesselName(0).compare("newVesselName") == 0);
        file.closeForWriting();
    }

    SECTION("Write 10 vessels then read it back in")
    {
        {
            isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
            file.writeImage(originalImage);
            for (size_t i = 0; i < 10; ++i)
            {
                file.writeVesselDiameterData(i, lineEndpoints, originalTrace, originalCenterTrace);
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

            isx::SpImage_t image = file.readProjectionImage();
            float * pixels = image->getPixelsAsF32();

            for (isx::isize_t i = 0; i < trace->getTimingInfo().getNumTimes(); ++i)
            {
                REQUIRE(values[i] == originalValues[i]);
            }

            for (isx::isize_t i = 0; i < spacingInfo.getTotalNumPixels(); ++i)
            {
                REQUIRE(pixels[i] == originalPixels[i]);
            }

            isx::SpVesselLine_t vesselLineEndpoints = file.readLineEndpoints(i);
            requireEqualVesselLines(vesselLineEndpoints, lineEndpoints);
        }
    }

    SECTION("Modify image / vessel data after calling closeForWriting")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        file.writeImage(originalImage);
        file.writeVesselDiameterData(0, lineEndpoints, originalTrace, originalCenterTrace);
        file.closeForWriting();

        ISX_REQUIRE_EXCEPTION(
            file.writeVesselDiameterData(0, lineEndpoints, originalTrace, originalCenterTrace);,
            isx::ExceptionFileIO,
            "Writing data after file was closed for writing." + fileName);
    }

    SECTION("Modify vessel validity after calling closeForWriting")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        file.writeImage(originalImage);
        file.writeVesselDiameterData(0, lineEndpoints, originalTrace, originalCenterTrace);
        file.closeForWriting();

        ISX_REQUIRE_EXCEPTION(
            file.setVesselStatus(0, isx::VesselSet::VesselStatus::ACCEPTED),
            isx::ExceptionFileIO,
            "Writing data after file was closed for writing." + fileName);
    }

    SECTION("Modify vessel name after calling closeForWriting")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        file.writeImage(originalImage);
        file.writeVesselDiameterData(0, lineEndpoints, originalTrace, originalCenterTrace);
        file.closeForWriting();
        ISX_REQUIRE_EXCEPTION(
            file.setVesselName(0, "newName"),
            isx::ExceptionFileIO,
            "Writing data after file was closed for writing." + fileName);
    }

    SECTION("Check default names for 1 vessel")
    {
        writeDefaultDiameterVessels(fileName, timingInfo, spacingInfo, originalImage, lineEndpoints, originalTrace, originalCenterTrace, 1);

        isx::VesselSetFile file(fileName);
        REQUIRE(file.getVesselName(0) == "V0");
    }

    SECTION("Check default names for 9 vessels")
    {
        const size_t numVessels = 9;
        writeDefaultDiameterVessels(fileName, timingInfo, spacingInfo, originalImage, lineEndpoints, originalTrace, originalCenterTrace, numVessels);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numVessels; ++i)
        {
            REQUIRE(file.getVesselName(i) == "V" + std::to_string(i));
        }
    }

    SECTION("Check default names for 10 vessels")
    {
        const size_t numVessels = 10;
        writeDefaultDiameterVessels(fileName, timingInfo, spacingInfo, originalImage, lineEndpoints, originalTrace, originalCenterTrace, numVessels);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numVessels; ++i)
        {
            REQUIRE(file.getVesselName(i) == "V" + std::to_string(i));
        }
    }

    SECTION("Check default names for 11 vessels")
    {
        const size_t numVessels = 11;
        writeDefaultDiameterVessels(fileName, timingInfo, spacingInfo, originalImage, lineEndpoints, originalTrace, originalCenterTrace, numVessels);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numVessels; ++i)
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

    SECTION("Check default names for 100 vessels")
    {
        const size_t numVessels = 100;
        writeDefaultDiameterVessels(fileName, timingInfo, spacingInfo, originalImage, lineEndpoints, originalTrace, originalCenterTrace, numVessels);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numVessels; ++i)
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

    SECTION("Check default names for 101 vessels")
    {
        const size_t numVessels = 101;
        writeDefaultDiameterVessels(fileName, timingInfo, spacingInfo, originalImage, lineEndpoints, originalTrace, originalCenterTrace, numVessels);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numVessels; ++i)
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

TEST_CASE("VesselSetFileTest-RbcVelocity", "[core-internal]")
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
    isx::SpImage_t originalImage = std::make_shared<isx::Image>(
            spacingInfo,
            sizeof(float) * spacingInfo.getNumColumns(),
            1,
            isx::DataType::F32);
    float * originalPixels = originalImage->getPixelsAsF32();
    std::memset(originalPixels, 0, sizeof(float) * originalImage->getSpacingInfo().getTotalNumPixels());
    originalPixels[0] = 1.0f;
    originalPixels[1] = 2.5f;

    // Set data
    isx::SpFTrace_t originalTrace = std::make_shared<isx::FTrace_t>(timingInfo);
    float * originalValues = originalTrace->getValues();
    float val = 0.0f;
    for (isx::isize_t i(0); i < timingInfo.getNumTimes(); ++i)
    {
        originalValues[i] = val;
        val += 0.01f;
    }

    // set line endpoints
    isx::SpVesselLine_t lineEndpoints = std::make_shared<isx::VesselLine>(isx::Contour_t{
        isx::PointInPixels_t(0,0), isx::PointInPixels_t(1,1), isx::PointInPixels_t(2,2), isx::PointInPixels_t(3,3)
    });

    isx::SpFTrace_t originalDirection = std::make_shared<isx::FTrace_t>(timingInfo);
    float * direction = originalDirection->getValues();
    float d = 0.0f;
    for (isx::isize_t i(0); i < timingInfo.getNumTimes(); ++i)
    {
        direction[i] = d;
        d += 0.001f;
    }

    isx::SizeInPixels_t correlationSize(10, 20);
    size_t corrNumPixels = correlationSize.getWidth() * correlationSize.getHeight();
    isx::SpVesselCorrelationsTrace_t originalCorrTriptychs = std::make_shared<isx::VesselCorrelationsTrace>(timingInfo, correlationSize);
    for (size_t i = 0; i < timingInfo.getNumTimes(); i++)
    {
        isx::SpVesselCorrelations_t triptych = originalCorrTriptychs->getValue(i);
        for (int offset = -1; offset <= 1; offset++)
        {
            float * data = triptych->getValues(offset);
            for (size_t j = 0; j < corrNumPixels; j++)
            {
                data[j] = float(i + j) / float(offset + 2);
            }
        }
    }

    SECTION("Constructor for new file")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        REQUIRE(file.isValid());
        REQUIRE(file.numberOfVessels() == 0);
        REQUIRE(file.getTimingInfo() == timingInfo);
        REQUIRE(file.getSpacingInfo() == spacingInfo);
        file.closeForWriting();
    }

    SECTION("Constructor for existing file")
    {
        {
            isx::VesselSetFile newFile(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            REQUIRE(newFile.isValid());
            newFile.closeForWriting();
        }
        isx::VesselSetFile existingFile(fileName);
        REQUIRE(existingFile.isValid());
        REQUIRE(existingFile.numberOfVessels() == 0);
    }

    SECTION("Write vessel data")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        REQUIRE(file.isValid());
        REQUIRE(file.numberOfVessels() == 0);

        // Write to file
        /// Vessel trace with default name
        file.writeImage(originalImage);
        file.writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection);
        REQUIRE(file.numberOfVessels() == 1);
        REQUIRE(file.getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(file.getVesselName(0).compare("") == 0);

        /// Vessel trace with given name
        file.writeVesselVelocityData(1, lineEndpoints, originalTrace, originalDirection, nullptr, "testName");
        REQUIRE(file.numberOfVessels() == 2);
        REQUIRE(file.getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(file.getVesselName(1).compare("testName") == 0);
        file.closeForWriting();
    }

    SECTION("Read projection image")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        REQUIRE(file.isValid());
        file.writeImage(originalImage);
        file.writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection);
        REQUIRE(file.numberOfVessels() == 1);
        file.closeForWriting();

        isx::SpImage_t im = file.readProjectionImage();
        bool pixelsAreEqual = false;
        float * pixels = im->getPixelsAsF32();

        REQUIRE(im->getSpacingInfo().getTotalNumPixels() == originalImage->getSpacingInfo().getTotalNumPixels());

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

    SECTION("Read trace")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        REQUIRE(file.isValid());
        file.writeImage(originalImage);
        file.writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection);
        REQUIRE(file.numberOfVessels() == 1);
        file.closeForWriting();

        isx::SpFTrace_t trace = file.readTrace(0);
        float * values = trace->getValues();

        REQUIRE(trace->getTimingInfo().getNumTimes() == originalTrace->getTimingInfo().getNumTimes());

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

    SECTION("Read line endpoints")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        REQUIRE(file.isValid());
        file.writeImage(originalImage);
        file.writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection);
        REQUIRE(file.numberOfVessels() == 1);
        file.closeForWriting();

        isx::SpVesselLine_t vesselLineEndpoints = file.readLineEndpoints(0);
        requireEqualVesselLines(vesselLineEndpoints, lineEndpoints);
    }

    SECTION("Read direction")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        REQUIRE(file.isValid());
        file.writeImage(originalImage);
        file.writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection);
        REQUIRE(file.numberOfVessels() == 1);
        file.closeForWriting();

        isx::SpFTrace_t vesselDirection = file.readDirectionTrace(0);
        requireEqualTraces(vesselDirection, originalDirection);
    }

    SECTION("Read correlation triptychs")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        REQUIRE(file.isValid());
        file.writeImage(originalImage);
        file.writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection, originalCorrTriptychs);
        REQUIRE(file.numberOfVessels() == 1);
        file.closeForWriting();

        for (size_t t = 0; t < timingInfo.getNumTimes(); t++)
        {
            requireEqualVesselCorrelations(file.readCorrelations(0, t), originalCorrTriptychs->getValue(t));
        }
    }

    SECTION("Validate/Invalidate vessel")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        REQUIRE(file.isValid());
        file.writeImage(originalImage);
        file.writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection);
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
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        REQUIRE(file.isValid());
        file.writeImage(originalImage);
        file.writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection);
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
            isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            file.writeImage(originalImage);
            for (size_t i = 0; i < 10; ++i)
            {
                file.writeVesselVelocityData(i, lineEndpoints, originalTrace, originalDirection, originalCorrTriptychs);
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

            isx::SpImage_t image = file.readProjectionImage();
            float * pixels = image->getPixelsAsF32();

            for (isx::isize_t j = 0; j < trace->getTimingInfo().getNumTimes(); ++j)
            {
                REQUIRE(values[j] == originalValues[j]);
            }

            for (isx::isize_t j = 0; j < spacingInfo.getTotalNumPixels(); ++j)
            {
                REQUIRE(pixels[j] == originalPixels[j]);
            }

            isx::SpVesselLine_t vesselLineEndpoints = file.readLineEndpoints(i);
            requireEqualVesselLines(vesselLineEndpoints, lineEndpoints);

            isx::SpFTrace_t vesselDirection = file.readDirectionTrace(i);
            requireEqualTraces(vesselDirection, originalDirection);

            for (size_t t = 0; t < timingInfo.getNumTimes(); t++)
            {
                requireEqualVesselCorrelations(file.readCorrelations(0, t), originalCorrTriptychs->getValue(t));
            }
        }
    }

    SECTION("Modify image / vessel data after calling closeForWriting")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        file.writeImage(originalImage);
        file.writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection);
        file.closeForWriting();

        ISX_REQUIRE_EXCEPTION(
            file.writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection),
            isx::ExceptionFileIO,
            "Writing data after file was closed for writing." + fileName);
    }

    SECTION("Modify vessel validity after calling closeForWriting")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        file.writeImage(originalImage);
        file.writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection);
        file.closeForWriting();

        ISX_REQUIRE_EXCEPTION(
            file.setVesselStatus(0, isx::VesselSet::VesselStatus::ACCEPTED),
            isx::ExceptionFileIO,
            "Writing data after file was closed for writing." + fileName);
    }

    SECTION("Modify vessel name after calling closeForWriting")
    {
        isx::VesselSetFile file(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        file.writeImage(originalImage);
        file.writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection);
        file.closeForWriting();
        ISX_REQUIRE_EXCEPTION(
            file.setVesselName(0, "newName"),
            isx::ExceptionFileIO,
            "Writing data after file was closed for writing." + fileName);
    }

    SECTION("Check default names for 1 vessel")
    {
        writeDefaultVelocityVessels(fileName, timingInfo, spacingInfo, originalImage, lineEndpoints, originalTrace, originalDirection, 1);

        isx::VesselSetFile file(fileName);
        REQUIRE(file.getVesselName(0) == "V0");
    }

    SECTION("Check default names for 9 vessels")
    {
        const size_t numVessels = 9;
        writeDefaultVelocityVessels(fileName, timingInfo, spacingInfo, originalImage, lineEndpoints, originalTrace, originalDirection, numVessels);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numVessels; ++i)
        {
            REQUIRE(file.getVesselName(i) == "V" + std::to_string(i));
        }
    }

    SECTION("Check default names for 10 vessels")
    {
        const size_t numVessels = 10;
        writeDefaultVelocityVessels(fileName, timingInfo, spacingInfo, originalImage, lineEndpoints, originalTrace, originalDirection, numVessels);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numVessels; ++i)
        {
            REQUIRE(file.getVesselName(i) == "V" + std::to_string(i));
        }
    }

    SECTION("Check default names for 11 vessels")
    {
        const size_t numVessels = 11;
        writeDefaultVelocityVessels(fileName, timingInfo, spacingInfo, originalImage, lineEndpoints, originalTrace, originalDirection, numVessels);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numVessels; ++i)
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

    SECTION("Check default names for 100 vessels")
    {
        const size_t numVessels = 100;
        writeDefaultVelocityVessels(fileName, timingInfo, spacingInfo, originalImage, lineEndpoints, originalTrace, originalDirection, numVessels);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numVessels; ++i)
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

    SECTION("Check default names for 101 vessels")
    {
        const size_t numVessels = 101;
        writeDefaultVelocityVessels(fileName, timingInfo, spacingInfo, originalImage, lineEndpoints, originalTrace, originalDirection, numVessels);

        isx::VesselSetFile file(fileName);
        for (size_t i = 0; i < numVessels; ++i)
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
