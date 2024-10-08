#include "isxCore.h"
#include "isxVesselSetFactory.h"
#include "isxVesselSetSeries.h"
#include "catch.hpp"
#include "isxTest.h"
#include "json.hpp"
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
    isx::SpVesselLine_t lineEndpoints = std::make_shared<isx::VesselLine>(isx::Contour_t{isx::PointInPixels_t(0,0), isx::PointInPixels_t(1,1)});

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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], tis[i], spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfos[i], isx::VesselSetType_t::VESSEL_DIAMETER); 
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER); 

            if( i == 2)
            {
                isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
                float * values = trace->getValues();
                std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
                trace->setValue(i, float(i));

                isx::SpFTrace_t centerTrace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
                float * centerValues = centerTrace->getValues();
                std::memset(centerValues, 0, sizeof(float)*timingInfos[i].getNumTimes());
                centerTrace->setValue(i, float(i));
                
                cs->writeImage(vesselImage);
                cs->writeVesselDiameterData(0, lineEndpoints, trace, centerTrace);
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], tis[i], spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);            
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], tis[i], spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);            
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * values = trace->getValues();
            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
            trace->setValue(i, float(i));

            isx::SpFTrace_t centerTrace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * centerValues = centerTrace->getValues();
            std::memset(centerValues, 0, sizeof(float)*timingInfos[i].getNumTimes());
            centerTrace->setValue(i, float(i));
                        
            cs->writeImage(vesselImage);
            cs->writeVesselDiameterData(0, lineEndpoints, trace, centerTrace);
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * values = trace->getValues();
            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
            totalNumSamples += timingInfos[i].getNumTimes();
            trace->setValue(i, float(i));

            isx::SpFTrace_t centerTrace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * centerValues = centerTrace->getValues();
            std::memset(centerValues, 0, sizeof(float)*timingInfos[i].getNumTimes());
            centerTrace->setValue(i, float(i));
            
            cs->writeImage(vesselImage);
            cs->writeVesselDiameterData(0, lineEndpoints, trace, centerTrace);
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

    SECTION("Get line endpoints")
    {
        isx::isize_t totalNumSamples = 0;

        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo, isx::VesselSetType_t::VESSEL_DIAMETER);
            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * values = trace->getValues();
            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
            totalNumSamples += timingInfos[i].getNumTimes();
            trace->setValue(i, float(i));

            isx::SpFTrace_t centerTrace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * centerValues = centerTrace->getValues();
            std::memset(centerValues, 0, sizeof(float)*timingInfos[i].getNumTimes());
            centerTrace->setValue(i, float(i));
            
            cs->writeImage(vesselImage);
            cs->writeVesselDiameterData(0, lineEndpoints, trace, centerTrace);
            cs->closeForWriting();
        }

        isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames);
        isx::SpVesselLine_t vesselLineEndpoints = css->getLineEndpoints(0);
        requireEqualVesselLines(vesselLineEndpoints, lineEndpoints);
    }

    for (const auto & fn: filenames)
    {
        std::remove(fn.c_str());
    }

    isx::CoreShutdown();
}

TEST_CASE("VesselSetSeries-RbcVelocity", "[core-internal]")
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
    isx::SpVesselLine_t lineEndpoints = std::make_shared<isx::VesselLine>(isx::Contour_t{
        isx::PointInPixels_t(0,0), isx::PointInPixels_t(1,1), isx::PointInPixels_t(2,2), isx::PointInPixels_t(3,3)
    });

    isx::CoreInitialize();

    SECTION("Compatible set of vesselsets")
    {        
        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], tis[i], spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfos[i], isx::VesselSetType_t::RBC_VELOCITY); 
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo, isx::VesselSetType_t::RBC_VELOCITY); 

            if( i == 2)
            {
                isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
                float * values = trace->getValues();
                std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
                trace->setValue(i, float(i)); 
                isx::SpFTrace_t direction = std::make_shared<isx::Trace<float>>(timingInfos[i]);
                isx::SizeInPixels_t correlationSize(10, 20);
                isx::SpVesselCorrelationsTrace_t correlations = std::make_shared<isx::VesselCorrelationsTrace>(timingInfos[i], correlationSize);
                cs->writeImage(vesselImage);
                cs->writeVesselVelocityData(0, lineEndpoints, trace, direction, correlations);
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], tis[i], spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);            
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], tis[i], spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);            
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * values = trace->getValues();
            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
            trace->setValue(i, float(i));
            isx::SpFTrace_t direction = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            isx::SizeInPixels_t correlationSize(10, 20);
            isx::SpVesselCorrelationsTrace_t correlations = std::make_shared<isx::VesselCorrelationsTrace>(timingInfos[i], correlationSize);
            cs->writeImage(vesselImage);
            cs->writeVesselVelocityData(0, lineEndpoints, trace, direction, correlations);
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
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * values = trace->getValues();
            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
            totalNumSamples += timingInfos[i].getNumTimes();
            trace->setValue(i, float(i));
            isx::SpFTrace_t direction = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            isx::SizeInPixels_t correlationSize(10, 20);
            isx::SpVesselCorrelationsTrace_t correlations = std::make_shared<isx::VesselCorrelationsTrace>(timingInfos[i], correlationSize);
            cs->writeImage(vesselImage);
            cs->writeVesselVelocityData(0, lineEndpoints, trace, direction, correlations);
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

    SECTION("Get line endpoints")
    {
        isx::isize_t totalNumSamples = 0;

        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * values = trace->getValues();
            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
            totalNumSamples += timingInfos[i].getNumTimes();
            trace->setValue(i, float(i));
            isx::SpFTrace_t direction = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            isx::SizeInPixels_t correlationSize(10, 20);
            isx::SpVesselCorrelationsTrace_t correlations = std::make_shared<isx::VesselCorrelationsTrace>(timingInfos[i], correlationSize);
            cs->writeImage(vesselImage);
            cs->writeVesselVelocityData(0, lineEndpoints, trace, direction, correlations);
            cs->closeForWriting();
        }

        isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames);
        isx::SpVesselLine_t vesselLineEndpoints = css->getLineEndpoints(0);
        requireEqualVesselLines(vesselLineEndpoints, lineEndpoints);
    }

    SECTION("Get direction")
    {
        isx::isize_t totalNumSamples = 0;

        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * values = trace->getValues();
            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
            totalNumSamples += timingInfos[i].getNumTimes();
            trace->setValue(i, float(i));
            
            isx::SpFTrace_t direction = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            direction->setValue(i, float(i) / 100);
            
            isx::SizeInPixels_t correlationSize(10, 20);
            isx::SpVesselCorrelationsTrace_t correlations = std::make_shared<isx::VesselCorrelationsTrace>(timingInfos[i], correlationSize);
            cs->writeImage(vesselImage);
            cs->writeVesselVelocityData(0, lineEndpoints, trace, direction, correlations);
            cs->closeForWriting();
        }

        isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames);

        isx::SpFTrace_t direction = css->getDirectionTrace(0);

        float * d = direction->getValues();

        for (isx::isize_t i(0); i < totalNumSamples; ++i)
        {
            if (i == 4) 
            {
                REQUIRE(*d == 1e-2f);
            }
            else if (i == 9)
            {
                REQUIRE(*d == 2e-2f);
            }
            else 
            {
                REQUIRE(*d == 0.0f);
            }

            d++;
        }
    }

    SECTION("Get correlations")
    {
        isx::isize_t totalNumSamples = 0;
        isx::SizeInPixels_t correlationSize(10, 20);
        size_t corrNumPixels = correlationSize.getWidth() * correlationSize.getHeight();
        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * values = trace->getValues();
            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
            totalNumSamples += timingInfos[i].getNumTimes();
            trace->setValue(i, float(i));
            isx::SpFTrace_t direction = std::make_shared<isx::Trace<float>>(timingInfos[i]);

            isx::SpVesselCorrelationsTrace_t correlations = std::make_shared<isx::VesselCorrelationsTrace>(timingInfos[i], correlationSize);
            for (size_t j = 0; j <  timingInfos[i].getNumTimes(); j++)
            {
                isx::SpVesselCorrelations_t triptych = correlations->getValue(j);
                for (int offset = -1; offset <= 1; offset++)
                {
                    float * data = triptych->getValues(offset);
                    for (size_t k = 0; k < corrNumPixels; k++)
                    {
                        data[k] = float(i + j) / float(offset + 2);
                    }
                }
            }
            cs->writeImage(vesselImage);
            cs->writeVesselVelocityData(0, lineEndpoints, trace, direction, correlations);
            cs->closeForWriting();
        }

        isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames);
        for (isx::isize_t j(0); j < totalNumSamples; ++j)
        {
            isx::SpVesselCorrelations_t triptych = css->getCorrelations(0, j);
            for (int offset = -1; offset <= 1; offset++)
            {
                float * data = triptych->getValues(offset);
                for (size_t k = 0; k < corrNumPixels; k++)
                {
                    if (j < 3) 
                    {
                        REQUIRE(data[k] == float(j) / float(offset + 2));
                    }
                    else if (j < 7)
                    {
                        REQUIRE(data[k] == float(1 + (j - 3)) / float(offset + 2));
                    }
                    else
                    {
                        REQUIRE(data[k] == float(2 + (j - 7)) / float(offset + 2));
                    }
                }

            }
            
        }
    }

    SECTION("Get max velocity")
    {
        isx::isize_t totalNumSamples = 0;
        const isx::Contour_t contour = {
            isx::PointInPixels_t(0, 0),
            isx::PointInPixels_t(0, 4),
            isx::PointInPixels_t(4, 4),
            isx::PointInPixels_t(4, 0),
        };
        isx::SpVesselLine_t lineEndpoints = std::make_shared<isx::VesselLine>(contour);

        using json = nlohmann::json;
        json extraProps;
        extraProps["idps"] = json::object();
        extraProps["idps"]["vesselset"]["inputMovieFps"] = 100.0f;

        // Write simple vessel sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            std::string extraProperties = extraProps.dump();

            isx::SpVesselSet_t cs = isx::writeVesselSet(filenames[i], timingInfos[i], spacingInfo, isx::VesselSetType_t::RBC_VELOCITY);
            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * values = trace->getValues();
            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
            totalNumSamples += timingInfos[i].getNumTimes();
            trace->setValue(i, float(i));
            isx::SpFTrace_t direction = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            isx::SizeInPixels_t correlationSize(10, 20);
            isx::SpVesselCorrelationsTrace_t correlations = std::make_shared<isx::VesselCorrelationsTrace>(timingInfos[i], correlationSize);
            cs->writeImage(vesselImage);
            cs->writeVesselVelocityData(0, lineEndpoints, trace, direction, correlations);
            cs->setExtraProperties(extraProperties);
            cs->closeForWriting();
        }

        isx::SpVesselSet_t css = isx::readVesselSetSeries(filenames);
        float maxVelocity = css->getMaxVelocity(0);

        const float expMaxVelocity = 4.0f / 2.0f * 100.0f;
        REQUIRE(maxVelocity == expMaxVelocity);
    }

    for (const auto & fn: filenames)
    {
        std::remove(fn.c_str());
    }

    isx::CoreShutdown();
}
