#include "isxCore.h"
#include "isxVesselSetFactory.h"
#include "isxVesselSetSeries.h"
#include "catch.hpp"
#include "isxTest.h"
#include <algorithm>
#include <array>
#include <cstring>

TEST_CASE("VesselSetSeries", "[core-internal]")
{
    std::array<const char *, 3> names = 
    { {
        "seriesVesselSet0.isxd",
        "seriesVesselSet1.isxd",
        "seriesVesselSet2.isxd"
    } };
    std::vector<std::string> filenames;

    for (auto n: names)
    {
        filenames.push_back(g_resources["unitTestDataPath"] + "/" + n);
    }

    std::array<isx::TimingInfo, 3> timingInfos =
    { {
        { isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 3 },
        { isx::Time(2222, 4, 1, 3, 1, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 4 },
        { isx::Time(2222, 4, 1, 3, 2, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 5 }
    } };

    isx::SizeInPixels_t sizePixels(4, 3);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(sizePixels, pixelSize, topLeft);

    isx::SpImage_t vesselImage = std::make_shared<isx::Image>(spacingInfo, spacingInfo.getNumColumns()*sizeof(float), 1, isx::DataType::F32);
    float* v = vesselImage->getPixelsAsF32();
    float vesselImageData[] = {       
        1.f, 0.f, 1.f, 
        0.f, 0.f, 0.f, 
        0.f, 0.f, 0.f, 
        1.f, 0.f, 1.f};

    std::memcpy((char *)v, (char *)vesselImageData, spacingInfo.getTotalNumPixels()*sizeof(float));

    // line endpoints
    const std::pair<isx::PointInPixels_t, isx::PointInPixels_t> lineEndpoints = std::make_pair(
        isx::PointInPixels_t(0,0), isx::PointInPixels_t(1,1));

    isx::CoreInitialize();
    
    SECTION("Empty constructor")
    {
        auto vs = std::make_shared<isx::VesselSetSeries>();
        REQUIRE(!vs->isValid());
    }

    SECTION("Compatible set of vesselsets")
    {        
        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo);
            cs->closeForWriting();
        }

        isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames);

        isx::TimingInfo expectedTimingInfo(isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 12);
        REQUIRE(css->isValid());
        REQUIRE(css->getSpacingInfo() == spacingInfo);
        REQUIRE(css->getTimingInfo() == expectedTimingInfo);
        REQUIRE(css->getNumVessels() == 0);
    }

    SECTION("Compatible set of vesselsets - not sequential in time ")
    {
        std::vector<isx::TimingInfo> tis(timingInfos.begin(), timingInfos.end());
        std::swap(tis[1], tis[2]);

        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], tis[i], spacingInfo);
            cs->closeForWriting();
        }

        isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames);

        isx::TimingInfo expectedTimingInfo(isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 12);
        REQUIRE(css->isValid());
        REQUIRE(css->getSpacingInfo() == spacingInfo);
        REQUIRE(css->getTimingInfo() == expectedTimingInfo);
        REQUIRE(css->getNumVessels() == 0);

    }

    SECTION("Non-compatible set of vesselsets - spacing info")
    {
        isx::SizeInPixels_t incompatibleSizePixels(3, 4);
        std::array<isx::SpacingInfo, 3> spacingInfos =
        { {
            spacingInfo,
            isx::SpacingInfo(incompatibleSizePixels, pixelSize, topLeft),
            spacingInfo
        } };

        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfos[i]);
            cs->closeForWriting();
        }

        ISX_REQUIRE_EXCEPTION(
            isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames),
            isx::ExceptionSeries,
            "The new data set has different spacing information than the rest of the series. Spacing information must be equal among series' components.");
    }

    SECTION("Non-compatible set of vesselsets - number of vessels")
    {
        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo); 

            if( i == 2)
            {
                isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
                float * values = trace->getValues();
                std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
                trace->setValue(i, float(i));                
                cs->writeImageAndLineAndTrace(0, vesselImage, lineEndpoints, trace);
            }           
            cs->closeForWriting();
        }

        ISX_REQUIRE_EXCEPTION(
            isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames),
            isx::ExceptionSeries,
            "VesselSet series member with mismatching number of vessels.");
    }

    SECTION("Non-compatible set of vesselsets - non-overlapping time windows")
    {
        std::vector<isx::TimingInfo> tis(timingInfos.begin(), timingInfos.end());
        tis[1] = tis[2];

        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], tis[i], spacingInfo);            
            cs->closeForWriting();
        }

        ISX_REQUIRE_EXCEPTION(
            isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames),
            isx::ExceptionSeries,
            "Unable to insert data that temporally overlaps with other parts of the series. Data sets in a series must all be non-overlapping.");
    }

    SECTION("Compatible set of vesselsets with different framerates")
    {
        std::array<isx::TimingInfo, 3> tis =
        { {
            { isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 3 },
            { isx::Time(2222, 4, 1, 3, 1, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 30), 4 },
            { isx::Time(2222, 4, 1, 3, 2, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 40), 5 }
        } };

        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], tis[i], spacingInfo);            
            cs->closeForWriting();
        }

        const isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames);
        const isx::TimingInfo expectedTi(isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 12);
        REQUIRE(css->isValid());
        REQUIRE(css->getSpacingInfo() == spacingInfo);
        REQUIRE(css->getTimingInfo() == expectedTi);
        REQUIRE(css->getNumVessels() == 0);
    }

    SECTION("Get image")
    {
        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo);
            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * values = trace->getValues();
            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
            trace->setValue(i, float(i));
            cs->writeImageAndLineAndTrace(0, vesselImage, lineEndpoints, trace);
            cs->closeForWriting();
        }

        isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames);

        isx::SpImage_t seriesImage = css->getImage(0);
        float * imVals = seriesImage->getPixelsAsF32();

        for (isx::isize_t i(0); i < seriesImage->getSpacingInfo().getTotalNumPixels(); ++i)
        {
            REQUIRE(*imVals == *v);
            imVals++;
            v++;
        }
    }

    SECTION("Get trace")
    {
        isx::isize_t totalNumSamples = 0;

        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo);
            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * values = trace->getValues();
            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
            totalNumSamples += timingInfos[i].getNumTimes();
            trace->setValue(i, float(i));
            cs->writeImageAndLineAndTrace(0, vesselImage, lineEndpoints, trace);
            cs->closeForWriting();
        }

        isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames);

        isx::SpFTrace_t trace = css->getTrace(0);

        float * traceVals = trace->getValues();

        for (isx::isize_t i(0); i < totalNumSamples; ++i)
        {
            
            if (i == 4) 
            {
                REQUIRE(*traceVals == 1.f);
            }
            else if (i == 9)
            {
                REQUIRE(*traceVals == 2.0);
            }
            else 
            {
                REQUIRE(*traceVals == 0.f);                
            }

            traceVals++;
        }
    }

//    SECTION("Get line endpoints")
//    {
//        isx::isize_t totalNumSamples = 0;
//
//        // Write simple vessel sets
//        for(isx::isize_t i(0); i < filenames.size(); ++i)
//        {
//            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo);
//            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
//            float * values = trace->getValues();
//            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
//            totalNumSamples += timingInfos[i].getNumTimes();
//            trace->setValue(i, float(i));
//            cs->writeImageAndLineAndTrace(0, vesselImage, lineEndpoints, trace);
//            cs->closeForWriting();
//        }
//
//        isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames);
//        // TODO: the following line stalls and doesn't seem to complete for some reason
//        std::pair<isx::PointInPixels_t, isx::PointInPixels_t> vesselLineEndpoints = css->getLineEndpoints(0);
//        REQUIRE(vesselLineEndpoints.first == lineEndpoints.first);
//        REQUIRE(vesselLineEndpoints.second == lineEndpoints.second);
//    }

    for (const auto & fn: filenames)
    {
        std::remove(fn.c_str());
    }

    isx::CoreShutdown();
}
