#include "isxVesselSetFactory.h"
#include "isxVesselSetSimple.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"
#include "isxMovieFactory.h"
#include "isxProject.h"
#include "json.hpp"
#include <cstring>
#include <atomic>

TEST_CASE("VesselSetTest", "[core]")
{
    std::string fileName = g_resources["unitTestDataPath"] + "/vesselset.isxd";
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

    isx::SpFTrace_t originalCenterTrace = std::make_shared<isx::FTrace_t>(timingInfo);
    float * originalCenterValues = originalTrace->getValues();


    float val = 0.0f;
    for (isx::isize_t i(0); i < timingInfo.getNumTimes(); ++i)
    {
        originalValues[i] = val;
        originalCenterValues[i] = val * 10.0f;
        val += 0.01f;
    }

    // line endpoints
    isx::SpVesselLine_t lineEndpoints = std::make_shared<isx::VesselLine>(isx::Contour_t{isx::PointInPixels_t(0,0), isx::PointInPixels_t(1,1)});

    isx::CoreInitialize();

    SECTION("Empty constructor")
    {
        isx::VesselSetSimple vesselSet;
        REQUIRE(!vesselSet.isValid());
    }

    SECTION("Write constructor")
    {
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);

        REQUIRE(vesselSet->isValid());
        REQUIRE(vesselSet->getFileName() == fileName);
        REQUIRE(vesselSet->getTimingInfo() == timingInfo);
        REQUIRE(vesselSet->getSpacingInfo() == spacingInfo);
        REQUIRE(vesselSet->getOriginalSpacingInfo() == isx::SpacingInfo::getDefault());
        REQUIRE(vesselSet->getNumVessels() == 0);
        vesselSet->closeForWriting();
    }

    SECTION("Write nVista 3 vessel set")
    {
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);

        vesselSet->setExtraProperties("{\"microscope\":\"nVista3\"}");
        REQUIRE(vesselSet->getSpacingInfo() == spacingInfo);
        REQUIRE(vesselSet->getOriginalSpacingInfo() == isx::SpacingInfo::getDefaultForNVista3());
        vesselSet->closeForWriting();
    }

    SECTION("Read constructor")
    {
        {
            isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
            vesselSet->closeForWriting();
        }
        isx::SpVesselSet_t vesselSet = isx::readVesselSet(fileName);

        REQUIRE(vesselSet->isValid());
        REQUIRE(vesselSet->getFileName() == fileName);
        REQUIRE(vesselSet->getTimingInfo() == timingInfo);
        REQUIRE(vesselSet->getExtraProperties() == "{\"idps\":{\"vesselset\":{\"type\":\"vessel diameter\"}}}");
        REQUIRE(vesselSet->getSpacingInfo() == spacingInfo);
        REQUIRE(vesselSet->getNumVessels() == 0);
    }

    SECTION("Read nVista 3 extra properties")
    {
        std::string extraProperties = "{\"microscope\":\"nVista3\"}";

        {
            isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
            vesselSet->setExtraProperties(extraProperties);
            vesselSet->closeForWriting();
        }

        isx::SpVesselSet_t vesselSet = isx::readVesselSet(fileName);

        // vessel set type is saved to metadata on closeForWriting
        extraProperties = "{\"idps\":{\"vesselset\":{\"type\":\"vessel diameter\"}},\"microscope\":\"nVista3\"}";

        REQUIRE(vesselSet->isValid());
        REQUIRE(vesselSet->getFileName() == fileName);
        REQUIRE(vesselSet->getTimingInfo() == timingInfo);
        REQUIRE(vesselSet->getSpacingInfo() == spacingInfo);
        REQUIRE(vesselSet->getExtraProperties() == extraProperties);
        REQUIRE(vesselSet->getNumVessels() == 0);
    }

    SECTION("Get vessel set type")
    {
        {
            isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
            vesselSet->closeForWriting();
        }

        isx::SpVesselSet_t vesselSet = isx::readVesselSet(fileName);
        REQUIRE(vesselSet->getVesselSetType() == isx::VesselSetType_t::VESSEL_DIAMETER);
    }

    SECTION("Set data for one vessel and check values are correct")
    {
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        vesselSet->writeImage(originalImage);
        vesselSet->writeVesselDiameterData(0, lineEndpoints, originalTrace, originalCenterTrace);
        vesselSet->closeForWriting();

        REQUIRE(vesselSet->getNumVessels() == 1);
        REQUIRE(vesselSet->getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        requireEqualImages(vesselSet->getImage(0), originalImage);
        requireEqualTraces(vesselSet->getTrace(0), originalTrace);
    }

    SECTION("Set data for one vessel and check read values are correct")
    {
        {
            isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
            vesselSet->writeImage(originalImage);
            vesselSet->writeVesselDiameterData(0, lineEndpoints, originalTrace, originalCenterTrace, "myvessel");
            vesselSet->closeForWriting();
        }
        isx::SpVesselSet_t vesselSet = isx::readVesselSet(fileName);

        REQUIRE(vesselSet->getNumVessels() == 1);
        REQUIRE(vesselSet->getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(vesselSet->getVesselName(0).compare("myvessel") == 0);
        requireEqualImages(vesselSet->getImage(0), originalImage);
        requireEqualTraces(vesselSet->getTrace(0), originalTrace);
    }

    SECTION("Set/Get vessel name")
    {
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        vesselSet->writeImage(originalImage);
        vesselSet->writeVesselDiameterData(0, lineEndpoints, originalTrace, originalCenterTrace);
        REQUIRE(vesselSet->getNumVessels() == 1);
        REQUIRE(vesselSet->getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(vesselSet->getVesselName(0).compare("") == 0);

        vesselSet->setVesselName(0, "newVesselName");
        REQUIRE(vesselSet->getVesselName(0).compare("newVesselName") == 0);
        vesselSet->closeForWriting();
    }

    SECTION("Set data for 3 vessels and check values are correct")
    {
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        vesselSet->writeImage(originalImage);
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeVesselDiameterData(i, lineEndpoints, originalTrace, originalCenterTrace);
        }
        vesselSet->setVesselStatus(0, isx::VesselSet::VesselStatus::ACCEPTED);
        vesselSet->setVesselStatus(1, isx::VesselSet::VesselStatus::UNDECIDED);
        vesselSet->setVesselStatus(2, isx::VesselSet::VesselStatus::REJECTED);
        vesselSet->closeForWriting();

        REQUIRE(vesselSet->getNumVessels() == 3);
        REQUIRE(vesselSet->getVesselStatus(0) == isx::VesselSet::VesselStatus::ACCEPTED);
        REQUIRE(vesselSet->getVesselStatus(1) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(vesselSet->getVesselStatus(2) == isx::VesselSet::VesselStatus::REJECTED);

        for (size_t i = 0; i < 3; ++i)
        {
            requireEqualImages(vesselSet->getImage(i), originalImage);
            requireEqualTraces(vesselSet->getTrace(i), originalTrace);
            requireEqualVesselLines(vesselSet->getLineEndpoints(i), lineEndpoints);
        }
    }

    SECTION("Set data for 3 vessels and check read values are correct")
    {
        {
            isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
            vesselSet->writeImage(originalImage);
            for (size_t i = 0; i < 3; ++i)
            {
                vesselSet->writeVesselDiameterData(i, lineEndpoints, originalTrace, originalCenterTrace);
            }
            vesselSet->setVesselStatus(0, isx::VesselSet::VesselStatus::ACCEPTED);
            vesselSet->setVesselStatus(1, isx::VesselSet::VesselStatus::UNDECIDED);
            vesselSet->setVesselStatus(2, isx::VesselSet::VesselStatus::REJECTED);
            vesselSet->closeForWriting();
        }
        isx::SpVesselSet_t vesselSet = isx::readVesselSet(fileName);

        REQUIRE(vesselSet->getNumVessels() == 3);
        REQUIRE(vesselSet->getVesselStatus(0) == isx::VesselSet::VesselStatus::ACCEPTED);
        REQUIRE(vesselSet->getVesselStatus(1) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(vesselSet->getVesselStatus(2) == isx::VesselSet::VesselStatus::REJECTED);

        for (size_t i = 0; i < 3; ++i)
        {
            requireEqualImages(vesselSet->getImage(i), originalImage);
            requireEqualTraces(vesselSet->getTrace(i), originalTrace);
             requireEqualVesselLines(vesselSet->getLineEndpoints(i), lineEndpoints);
        }
    }

    SECTION("Read trace data for 3 vessels asynchronously")
    {
        std::atomic_int doneCount(0);
        size_t numVessels = 3;
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        vesselSet->writeImage(originalImage);
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeVesselDiameterData(i, lineEndpoints, originalTrace, originalCenterTrace);
        }
        vesselSet->closeForWriting();

        isx::VesselSet::VesselSetGetTraceCB_t callBack = [originalTrace, &doneCount](isx::AsyncTaskResult<isx::SpFTrace_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            requireEqualTraces(inAsyncTaskResult.get(), originalTrace);
            ++doneCount;

        };
        for (size_t i = 0; i < numVessels; ++i)
        {
            vesselSet->getTraceAsync(i, callBack);
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numVessels))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == int(numVessels));
    }

    SECTION("Read image data for 3 vessels asynchronously")
    {
        std::atomic_int doneCount(0);
        size_t numVessels = 3;

        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        vesselSet->writeImage(originalImage);
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeVesselDiameterData(i, lineEndpoints, originalTrace, originalCenterTrace);
        }
        vesselSet->closeForWriting();

        isx::VesselSet::VesselSetGetImageCB_t callBack = [originalImage, &doneCount](isx::AsyncTaskResult<isx::SpImage_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            requireEqualImages(inAsyncTaskResult.get(), originalImage);
            ++doneCount;
        };
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->getImageAsync(i, callBack);
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numVessels))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == int(numVessels));
    }

    SECTION("Read line endpoints data for 3 vessels asynchronously")
    {
        std::atomic_int doneCount(0);
        size_t numVessels = 3;

        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
        vesselSet->writeImage(originalImage);
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeVesselDiameterData(i, lineEndpoints, originalTrace, originalCenterTrace);
        }
        vesselSet->closeForWriting();

        isx::VesselSet::VesselSetGetLineEndpointsCB_t callBack = [lineEndpoints, &doneCount](isx::AsyncTaskResult<isx::SpVesselLine_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            requireEqualVesselLines(inAsyncTaskResult.get(), lineEndpoints);
            ++doneCount;
        };
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->getLineEndpointsAsync(i, callBack);
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numVessels))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == int(numVessels));
    }

    isx::CoreShutdown();
}

TEST_CASE("VesselSetTest-RbcVelocity", "[core]")
{
    std::string fileName = g_resources["unitTestDataPath"] + "/vesselset.isxd";
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

    // line endpoints
    isx::SpVesselLine_t lineEndpoints = std::make_shared<isx::VesselLine>(isx::Contour_t{
        isx::PointInPixels_t(0,0), isx::PointInPixels_t(1,1), isx::PointInPixels_t(2,2), isx::PointInPixels_t(3,3)
    });

    isx::SpFTrace_t originalDirection = std::make_shared<isx::Trace<float>>(timingInfo);
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

    isx::CoreInitialize();

    SECTION("Write constructor")
    {
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);

        REQUIRE(vesselSet->isValid());
        REQUIRE(vesselSet->getFileName() == fileName);
        REQUIRE(vesselSet->getTimingInfo() == timingInfo);
        REQUIRE(vesselSet->getSpacingInfo() == spacingInfo);
        REQUIRE(vesselSet->getOriginalSpacingInfo() == isx::SpacingInfo::getDefault());
        REQUIRE(vesselSet->getNumVessels() == 0);
        vesselSet->closeForWriting();
    }

    SECTION("Write nVista 3 vessel set")
    {
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);

        vesselSet->setExtraProperties("{\"microscope\":\"nVista3\"}");
        REQUIRE(vesselSet->getSpacingInfo() == spacingInfo);
        REQUIRE(vesselSet->getOriginalSpacingInfo() == isx::SpacingInfo::getDefaultForNVista3());
        vesselSet->closeForWriting();
    }

    SECTION("Read constructor")
    {
        {
            isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            vesselSet->closeForWriting();
        }
        isx::SpVesselSet_t vesselSet = isx::readVesselSet(fileName);

        REQUIRE(vesselSet->isValid());
        REQUIRE(vesselSet->getFileName() == fileName);
        REQUIRE(vesselSet->getTimingInfo() == timingInfo);
        REQUIRE(vesselSet->getExtraProperties() == "{\"idps\":{\"vesselset\":{\"type\":\"red blood cell velocity\"}}}");
        REQUIRE(vesselSet->getSpacingInfo() == spacingInfo);
        REQUIRE(vesselSet->getNumVessels() == 0);
    }

    SECTION("Read nVista 3 extra properties")
    {
        std::string extraProperties = "{\"microscope\":\"nVista3\"}";

        {
            isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            vesselSet->setExtraProperties(extraProperties);
            vesselSet->closeForWriting();
        }

        isx::SpVesselSet_t vesselSet = isx::readVesselSet(fileName);

        // vessel set type is saved to metadata on closeForWriting
        extraProperties = "{\"idps\":{\"vesselset\":{\"type\":\"red blood cell velocity\"}},\"microscope\":\"nVista3\"}";

        REQUIRE(vesselSet->isValid());
        REQUIRE(vesselSet->getFileName() == fileName);
        REQUIRE(vesselSet->getTimingInfo() == timingInfo);
        REQUIRE(vesselSet->getSpacingInfo() == spacingInfo);
        REQUIRE(vesselSet->getExtraProperties() == extraProperties);
        REQUIRE(vesselSet->getNumVessels() == 0);
    }

    SECTION("Get vessel set type")
    {
        {
            isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            vesselSet->closeForWriting();
        }

        isx::SpVesselSet_t vesselSet = isx::readVesselSet(fileName);
        REQUIRE(vesselSet->getVesselSetType() == isx::VesselSetType_t::RBC_VELOCITY);
    }

    SECTION("Set data for one vessel and check values are correct")
    {
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        vesselSet->writeImage(originalImage);
        vesselSet->writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection);
        vesselSet->closeForWriting();

        REQUIRE(vesselSet->getNumVessels() == 1);
        REQUIRE(vesselSet->getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        requireEqualImages(vesselSet->getImage(0), originalImage);
        requireEqualTraces(vesselSet->getTrace(0), originalTrace);
    }

    SECTION("Set data for one vessel and check read values are correct")
    {
        {
            isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            vesselSet->writeImage(originalImage);
            vesselSet->writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection, originalCorrTriptychs, "myvessel");
            vesselSet->closeForWriting();
        }
        isx::SpVesselSet_t vesselSet = isx::readVesselSet(fileName);

        REQUIRE(vesselSet->getNumVessels() == 1);
        REQUIRE(vesselSet->getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(vesselSet->getVesselName(0).compare("myvessel") == 0);
        requireEqualImages(vesselSet->getImage(0), originalImage);
        requireEqualTraces(vesselSet->getTrace(0), originalTrace);
        requireEqualVesselLines(vesselSet->getLineEndpoints(0), lineEndpoints);
        requireEqualTraces(vesselSet->getDirectionTrace(0), originalDirection);
        for (size_t t = 0; t < timingInfo.getNumTimes(); t++)
        {
            requireEqualVesselCorrelations(vesselSet->getCorrelations(0, t), originalCorrTriptychs->getValue(t));
        }
    }

    SECTION("Set/Get vessel name")
    {
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        vesselSet->writeImage(originalImage);
        vesselSet->writeVesselVelocityData(0, lineEndpoints, originalTrace, originalDirection);
        REQUIRE(vesselSet->getNumVessels() == 1);
        REQUIRE(vesselSet->getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(vesselSet->getVesselName(0).compare("") == 0);

        vesselSet->setVesselName(0, "newVesselName");
        REQUIRE(vesselSet->getVesselName(0).compare("newVesselName") == 0);
        vesselSet->closeForWriting();
    }

    SECTION("Set data for 3 vessels and check values are correct")
    {
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        vesselSet->writeImage(originalImage);
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeVesselVelocityData(i, lineEndpoints, originalTrace, originalDirection, originalCorrTriptychs);
        }
        vesselSet->setVesselStatus(0, isx::VesselSet::VesselStatus::ACCEPTED);
        vesselSet->setVesselStatus(1, isx::VesselSet::VesselStatus::UNDECIDED);
        vesselSet->setVesselStatus(2, isx::VesselSet::VesselStatus::REJECTED);
        vesselSet->closeForWriting();

        REQUIRE(vesselSet->getNumVessels() == 3);
        REQUIRE(vesselSet->getVesselStatus(0) == isx::VesselSet::VesselStatus::ACCEPTED);
        REQUIRE(vesselSet->getVesselStatus(1) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(vesselSet->getVesselStatus(2) == isx::VesselSet::VesselStatus::REJECTED);

        for (size_t i = 0; i < 3; ++i)
        {
            requireEqualImages(vesselSet->getImage(i), originalImage);
            requireEqualTraces(vesselSet->getTrace(i), originalTrace);
            requireEqualVesselLines(vesselSet->getLineEndpoints(i), lineEndpoints);
            requireEqualTraces(vesselSet->getDirectionTrace(i), originalDirection);
            for (size_t t = 0; t < timingInfo.getNumTimes(); t++)
            {
                requireEqualVesselCorrelations(vesselSet->getCorrelations(i, t), originalCorrTriptychs->getValue(t));
            }
        }
    }

    SECTION("Set data for 3 vessels and check read values are correct")
    {
        {
            isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            vesselSet->writeImage(originalImage);
            for (size_t i = 0; i < 3; ++i)
            {
                vesselSet->writeVesselVelocityData(i, lineEndpoints, originalTrace, originalDirection, originalCorrTriptychs);
            }
            vesselSet->setVesselStatus(0, isx::VesselSet::VesselStatus::ACCEPTED);
            vesselSet->setVesselStatus(1, isx::VesselSet::VesselStatus::UNDECIDED);
            vesselSet->setVesselStatus(2, isx::VesselSet::VesselStatus::REJECTED);
            vesselSet->closeForWriting();
        }
        isx::SpVesselSet_t vesselSet = isx::readVesselSet(fileName);

        REQUIRE(vesselSet->getNumVessels() == 3);
        REQUIRE(vesselSet->getVesselStatus(0) == isx::VesselSet::VesselStatus::ACCEPTED);
        REQUIRE(vesselSet->getVesselStatus(1) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(vesselSet->getVesselStatus(2) == isx::VesselSet::VesselStatus::REJECTED);

        for (size_t i = 0; i < 3; ++i)
        {
            requireEqualImages(vesselSet->getImage(i), originalImage);
            requireEqualTraces(vesselSet->getTrace(i), originalTrace);
            requireEqualVesselLines(vesselSet->getLineEndpoints(i), lineEndpoints);
            requireEqualTraces(vesselSet->getDirectionTrace(i), originalDirection);
            for (size_t t = 0; t < timingInfo.getNumTimes(); t++)
            {
                requireEqualVesselCorrelations(vesselSet->getCorrelations(i, t), originalCorrTriptychs->getValue(t));
            }
        }
    }

    SECTION("Read trace data for 3 vessels asynchronously")
    {
        std::atomic_int doneCount(0);
        size_t numVessels = 3;
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        vesselSet->writeImage(originalImage);
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeVesselVelocityData(i, lineEndpoints, originalTrace, originalDirection);
        }
        vesselSet->closeForWriting();

        isx::VesselSet::VesselSetGetTraceCB_t callBack = [originalTrace, &doneCount](isx::AsyncTaskResult<isx::SpFTrace_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            requireEqualTraces(inAsyncTaskResult.get(), originalTrace);
            ++doneCount;

        };
        for (size_t i = 0; i < numVessels; ++i)
        {
            vesselSet->getTraceAsync(i, callBack);
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numVessels))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == int(numVessels));
    }

    SECTION("Read image data for 3 vessels asynchronously")
    {
        std::atomic_int doneCount(0);
        size_t numVessels = 3;

        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        vesselSet->writeImage(originalImage);
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeVesselVelocityData(i, lineEndpoints, originalTrace, originalDirection);
        }
        vesselSet->closeForWriting();

        isx::VesselSet::VesselSetGetImageCB_t callBack = [originalImage, &doneCount](isx::AsyncTaskResult<isx::SpImage_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            requireEqualImages(inAsyncTaskResult.get(), originalImage);
            ++doneCount;
        };
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->getImageAsync(i, callBack);
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numVessels))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == int(numVessels));
    }

    SECTION("Read line endpoints data for 3 vessels asynchronously")
    {
        std::atomic_int doneCount(0);
        size_t numVessels = 3;

        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        vesselSet->writeImage(originalImage);
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeVesselVelocityData(i, lineEndpoints, originalTrace, originalDirection);
        }
        vesselSet->closeForWriting();

        isx::VesselSet::VesselSetGetLineEndpointsCB_t callBack = [lineEndpoints, &doneCount](isx::AsyncTaskResult<isx::SpVesselLine_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            requireEqualVesselLines(inAsyncTaskResult.get(), lineEndpoints);
            ++doneCount;
        };
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->getLineEndpointsAsync(i, callBack);
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numVessels))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == int(numVessels));
    }

    SECTION("Read direction data for 3 vessels asynchronously")
    {
        std::atomic_int doneCount(0);
        size_t numVessels = 3;

        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        vesselSet->writeImage(originalImage);
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeVesselVelocityData(i, lineEndpoints, originalTrace, originalDirection);
        }
        vesselSet->closeForWriting();

        isx::VesselSet::VesselSetGetTraceCB_t callBack = [originalDirection, &doneCount](isx::AsyncTaskResult<isx::SpFTrace_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            requireEqualTraces(inAsyncTaskResult.get(), originalDirection);
            ++doneCount;
        };
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->getDirectionTraceAsync(i, callBack);
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numVessels))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == int(numVessels));
    }

    SECTION("Read correlation triptych data for 3 vessels asynchronously")
    {
        std::atomic_int doneCount(0);
        size_t numVessels = 3;

        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        vesselSet->writeImage(originalImage);
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeVesselVelocityData(i, lineEndpoints, originalTrace, originalDirection, originalCorrTriptychs);
        }
        vesselSet->closeForWriting();

        size_t frameIndex = 0;
        isx::VesselSet::VesselSetGetCorrelationsCB_t callBack = [originalCorrTriptychs, &doneCount, frameIndex](isx::AsyncTaskResult<isx::SpVesselCorrelations_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            requireEqualVesselCorrelations(inAsyncTaskResult.get(), originalCorrTriptychs->getValue(frameIndex));
            ++doneCount;
        };
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->getCorrelationsAsync(i, frameIndex, callBack);
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numVessels))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == int(numVessels));
    }

    SECTION("Get max velocity")
    {
        size_t numVessels = 3;
        const isx::Contour_t contour = {
            isx::PointInPixels_t(0, 0),
            isx::PointInPixels_t(0, 4),
            isx::PointInPixels_t(4, 4),
            isx::PointInPixels_t(4, 0),
        };
        isx::SpVesselLine_t lineEndpoints = std::make_shared<isx::VesselLine>(contour);

        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        vesselSet->writeImage(originalImage);
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeVesselVelocityData(i, lineEndpoints, originalTrace, originalDirection);
        }

        using json = nlohmann::json;
        json extraProps;
        extraProps["idps"] = json::object();
        extraProps["idps"]["vesselset"]["inputMovieFps"] = 100.0f;
        std::string extraProperties = extraProps.dump();
        vesselSet->setExtraProperties(extraProperties);

        vesselSet->closeForWriting();

        for (size_t i = 0; i < 3; ++i)
        {
            const float maxVelocity = vesselSet->getMaxVelocity(i);
            const float expMaxVelocity = 4.0f / 2.0f * 100.0f;
            REQUIRE(maxVelocity == expMaxVelocity);
        }
    }

    isx::CoreShutdown();
}

TEST_CASE("VesselLineTest-computeMaxVelocity", "[core]")
{
    isx::CoreInitialize();
    SECTION("Simple upright rectangle bounding box")
    {
        const isx::Contour_t contour = {
            isx::PointInPixels_t(0, 0),
            isx::PointInPixels_t(0, 4),
            isx::PointInPixels_t(4, 4),
            isx::PointInPixels_t(4, 0),
        };
        isx::VesselLine vesselRoi(contour);
        const float fps = 100.0f;

        float maxVelocity = vesselRoi.computeMaxVelocity(fps);
        const float expMaxVelocity = 4.0f / 2.0f * 100.0f;

        REQUIRE(maxVelocity == expMaxVelocity);
    }

    SECTION("Diagonal rectangle bounding box")
    {
        const isx::Contour_t contour = {
            isx::PointInPixels_t(4, 0),
            isx::PointInPixels_t(0, 4),
            isx::PointInPixels_t(2, 6),
            isx::PointInPixels_t(6, 2),
        };
        const size_t numRows = 7;
        const size_t numCols = 7;
        const float fps = 100.0f;
        isx::VesselLine vesselRoi(contour);

        float maxVelocity = vesselRoi.computeMaxVelocity(fps);

        const float expMaxVelocity = sqrt(32.0f) / 2.0f * 100.0f;

        REQUIRE(maxVelocity == expMaxVelocity);
    }

    SECTION("Line length of 0")
    {
        const isx::Contour_t contour = {
            isx::PointInPixels_t(0, 0),
            isx::PointInPixels_t(0, 0),
            isx::PointInPixels_t(0, 0),
            isx::PointInPixels_t(0, 0),
        };
        const size_t numRows = 7;
        const size_t numCols = 7;
        const float fps = 100.0f;
        isx::VesselLine vesselRoi(contour);

        float maxVelocity = vesselRoi.computeMaxVelocity(fps);

        const float expMaxVelocity = std::numeric_limits<float>::quiet_NaN();

        REQUIRE(std::isnan(maxVelocity) == std::isnan(expMaxVelocity));
    }

    isx::CoreShutdown();
}
