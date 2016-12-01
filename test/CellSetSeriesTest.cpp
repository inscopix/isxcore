#include "isxCore.h"
#include "isxCellSetFactory.h"
#include "isxCellSetSeries.h"
#include "catch.hpp"
#include "isxTest.h"
#include <algorithm>
#include <array>
#include <cstring>

TEST_CASE("CellSetSeries", "[core-internal]")
{
    std::array<const char *, 3> names = 
    { {
        "seriesCellSet0.isxd",
        "seriesCellSet1.isxd",
        "seriesCellSet2.isxd"
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

    isx::SpImage_t cellImage = std::make_shared<isx::Image>(spacingInfo, spacingInfo.getNumColumns()*sizeof(float), 1, isx::DataType::F32);
    float* v = cellImage->getPixelsAsF32();
    float cellImageData[] = {       
        1.f, 0.f, 1.f, 
        0.f, 0.f, 0.f, 
        0.f, 0.f, 0.f, 
        1.f, 0.f, 1.f};

    std::memcpy((char *)v, (char *)cellImageData, spacingInfo.getTotalNumPixels()*sizeof(float));

    

    isx::CoreInitialize();


    SECTION("Empty constructor")
    {
        auto cs = std::make_shared<isx::CellSetSeries>();
        REQUIRE(!cs->isValid());
    }

    SECTION("Compatible set of cellsets")
    {        
        // Write simple cell sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpCellSet_t cs = isx::writeCellSet(filenames[i], timingInfos[i], spacingInfo);            
        }

        isx::SpCellSet_t css = isx::readCellSetSeries(filenames);

        isx::TimingInfo expectedTimingInfo(isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 2405);
        REQUIRE(css->isValid());
        REQUIRE(css->getSpacingInfo() == spacingInfo);
        REQUIRE(css->getTimingInfo() == expectedTimingInfo);
        REQUIRE(css->getFileName() == "**CellSetSeries");
        REQUIRE(css->getNumCells() == 0);

    }

    SECTION("Compatible set of cellsets - not sequential in time ")
    {
        std::vector<isx::TimingInfo> tis(timingInfos.begin(), timingInfos.end());
        std::swap(tis[1], tis[2]);

        // Write simple cell sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpCellSet_t cs = isx::writeCellSet(filenames[i], tis[i], spacingInfo);            
        }

        isx::SpCellSet_t css = isx::readCellSetSeries(filenames);

        isx::TimingInfo expectedTimingInfo(isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 2405);
        REQUIRE(css->isValid());
        REQUIRE(css->getSpacingInfo() == spacingInfo);
        REQUIRE(css->getTimingInfo() == expectedTimingInfo);
        REQUIRE(css->getFileName() == "**CellSetSeries");
        REQUIRE(css->getNumCells() == 0);

    }

    SECTION("Non-compatible set of cellsets - spacing info")
    {
        isx::SizeInPixels_t incompatibleSizePixels(3, 4);
        std::array<isx::SpacingInfo, 3> spacingInfos =
        { {
            spacingInfo,
            isx::SpacingInfo(incompatibleSizePixels, pixelSize, topLeft),
            spacingInfo
        } };

        // Write simple cell sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpCellSet_t cs = isx::writeCellSet(filenames[i], timingInfos[i], spacingInfos[i]);            
        }

        ISX_REQUIRE_EXCEPTION(
            isx::SpCellSet_t css = isx::readCellSetSeries(filenames),
            isx::ExceptionSeries,
            "The spacing info is different than the reference.");
    }

    SECTION("Non-compatible set of cellsets - number of cells")
    {
        // Write simple cell sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpCellSet_t cs = isx::writeCellSet(filenames[i], timingInfos[i], spacingInfo); 

            if( i == 2)
            {
                isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
                float * values = trace->getValues();
                std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
                trace->setValue(i, float(i));                
                cs->writeImageAndTrace(0, cellImage, trace);
            }           
        }

        ISX_REQUIRE_EXCEPTION(
            isx::SpCellSet_t css = isx::readCellSetSeries(filenames),
            isx::ExceptionSeries,
            "CellSetSeries with mismatching number of cells: " + filenames.at(2));
    }

    SECTION("Non-compatible set of cellsets - non-overlapping time windows")
    {
        std::vector<isx::TimingInfo> tis(timingInfos.begin(), timingInfos.end());
        tis[1] = tis[2];

        // Write simple cell sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpCellSet_t cs = isx::writeCellSet(filenames[i], tis[i], spacingInfo);            
        }

        ISX_REQUIRE_EXCEPTION(
            isx::SpCellSet_t css = isx::readCellSetSeries(filenames),
            isx::ExceptionSeries,
            "The timing info temporally overlaps with the reference.");
    }

    SECTION("Non-compatible set of cellsets - framerate")
    {
        std::array<isx::TimingInfo, 3> tis =
        { {
            { isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 3 },
            { isx::Time(2222, 4, 1, 3, 1, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 30), 4 },
            { isx::Time(2222, 4, 1, 3, 2, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 40), 5 }
        } };

        // Write simple cell sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpCellSet_t cs = isx::writeCellSet(filenames[i], tis[i], spacingInfo);            
        }

        ISX_REQUIRE_EXCEPTION(
            isx::SpCellSet_t css = isx::readCellSetSeries(filenames),
            isx::ExceptionSeries,
            "The timing info has a different frame rate than the reference.");
    }

    SECTION("Get image")
    {
        // Write simple cell sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpCellSet_t cs = isx::writeCellSet(filenames[i], timingInfos[i], spacingInfo);
            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * values = trace->getValues();
            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
            trace->setValue(i, float(i));                
            cs->writeImageAndTrace(0, cellImage, trace);
        }

        isx::SpCellSet_t css = isx::readCellSetSeries(filenames);

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

        // Write simple cell sets
        for(isx::isize_t i(0); i < filenames.size(); ++i)
        {
            isx::SpCellSet_t cs = isx::writeCellSet(filenames[i], timingInfos[i], spacingInfo);
            isx::SpFTrace_t trace = std::make_shared<isx::Trace<float>>(timingInfos[i]);
            float * values = trace->getValues();
            std::memset(values, 0, sizeof(float)*timingInfos[i].getNumTimes());
            totalNumSamples += timingInfos[i].getNumTimes();
            trace->setValue(i, float(i));                
            cs->writeImageAndTrace(0, cellImage, trace);
        }

        isx::SpCellSet_t css = isx::readCellSetSeries(filenames);

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
    


    for (const auto & fn: filenames)
    {
        std::remove(fn.c_str());
    }


    isx::CoreShutdown();

}
