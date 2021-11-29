#include "isxVesselSetFactory.h"
#include "isxVesselSetSimple.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"
#include "isxMovieFactory.h"
#include "isxProject.h"
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
    float val = 0.0f;
    for (isx::isize_t i(0); i < timingInfo.getNumTimes(); ++i)
    {
        originalValues[i] = val;
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
        vesselSet->writeImageAndLineAndTrace(0, originalImage, lineEndpoints, originalTrace);
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
            vesselSet->writeImageAndLineAndTrace(0, originalImage, lineEndpoints, originalTrace, "myvessel");
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
        vesselSet->writeImageAndLineAndTrace(0, originalImage, lineEndpoints, originalTrace);
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
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeImageAndLineAndTrace(i, originalImage, lineEndpoints, originalTrace);
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
            for (size_t i = 0; i < 3; ++i)
            {
                vesselSet->writeImageAndLineAndTrace(i, originalImage, lineEndpoints, originalTrace);
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
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeImageAndLineAndTrace(i, originalImage, lineEndpoints, originalTrace);
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
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeImageAndLineAndTrace(i, originalImage, lineEndpoints, originalTrace);
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
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeImageAndLineAndTrace(i, originalImage, lineEndpoints, originalTrace);
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

    isx::SpVesselDirectionTrace_t originalDirection = std::make_shared<isx::VesselDirectionTrace>(timingInfo);
    float * originalX = originalDirection->m_x->getValues();
    float * originalY = originalDirection->m_y->getValues();
    float x = 0.0f;
    float y = 0.0f;
    for (isx::isize_t i(0); i < timingInfo.getNumTimes(); ++i)
    {
        originalX[i] = x;
        originalY[i] = y;
        x += 0.001f;
        y += 0.002f;
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
        vesselSet->writeImageAndLineAndTrace(0, originalImage, lineEndpoints, originalTrace, "", originalDirection);
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
            vesselSet->writeImageAndLineAndTrace(0, originalImage, lineEndpoints, originalTrace, "myvessel", originalDirection);
            vesselSet->closeForWriting();
        }
        isx::SpVesselSet_t vesselSet = isx::readVesselSet(fileName);

        REQUIRE(vesselSet->getNumVessels() == 1);
        REQUIRE(vesselSet->getVesselStatus(0) == isx::VesselSet::VesselStatus::UNDECIDED);
        REQUIRE(vesselSet->getVesselName(0).compare("myvessel") == 0);
        requireEqualImages(vesselSet->getImage(0), originalImage);
        requireEqualTraces(vesselSet->getTrace(0), originalTrace);
        requireEqualVesselLines(vesselSet->getLineEndpoints(0), lineEndpoints);
        requireEqualVesselDirections(vesselSet->getDirection(0), originalDirection);
    }

    SECTION("Set/Get vessel name")
    {
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        vesselSet->writeImageAndLineAndTrace(0, originalImage, lineEndpoints, originalTrace, "", originalDirection);
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
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeImageAndLineAndTrace(i, originalImage, lineEndpoints, originalTrace, "", originalDirection);
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
            requireEqualVesselDirections(vesselSet->getDirection(i), originalDirection);
        }
    }

    SECTION("Set data for 3 vessels and check read values are correct")
    {
        {
            isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            for (size_t i = 0; i < 3; ++i)
            {
                vesselSet->writeImageAndLineAndTrace(i, originalImage, lineEndpoints, originalTrace, "", originalDirection);
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
            requireEqualVesselDirections(vesselSet->getDirection(i), originalDirection);
        }
    }

    SECTION("Read trace data for 3 vessels asynchronously")
    {
        std::atomic_int doneCount(0);
        size_t numVessels = 3;
        isx::SpVesselSet_t vesselSet = isx::writeVesselSet(fileName, timingInfo, spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeImageAndLineAndTrace(i, originalImage, lineEndpoints, originalTrace, "", originalDirection);
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
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeImageAndLineAndTrace(i, originalImage, lineEndpoints, originalTrace, "", originalDirection);
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
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeImageAndLineAndTrace(i, originalImage, lineEndpoints, originalTrace, "", originalDirection);
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
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->writeImageAndLineAndTrace(i, originalImage, lineEndpoints, originalTrace, "", originalDirection);
        }
        vesselSet->closeForWriting();

        isx::VesselSet::VesselSetGetDirectionTraceCB_t callBack = [originalDirection, &doneCount](isx::AsyncTaskResult<isx::SpVesselDirectionTrace_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            requireEqualVesselDirections(inAsyncTaskResult.get(), originalDirection);
            ++doneCount;
        };
        for (size_t i = 0; i < 3; ++i)
        {
            vesselSet->getDirectionAsync(i, callBack);
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
