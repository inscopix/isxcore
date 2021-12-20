#include "catch.hpp"
#include "isxTest.h"

#include "isxMetadata.h"
#include "isxDataSet.h"
#include "isxMovieFactory.h"
#include "isxPathUtils.h"

TEST_CASE("IntegratedBasePlate-Get", "[core]")
{
    isx::CoreInitialize();

    SECTION("IDAS metadata - Integrated base plate")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/2021-06-14-13-30-29_video_green.isxd";
        isx::SpMovie_t movie = isx::readMovie(filePath);
        isx::BasePlateType_t probeType = isx::getBasePlateType(movie);

        isx::BasePlateType_t expectedProbeType = isx::BasePlateType_t::BP31;
        REQUIRE(probeType == expectedProbeType);
    }

    SECTION("IDAS metadata - Custom base plate")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/2021-06-28-23-45-49_video_sched_0_probe_custom.isxd";
        isx::SpMovie_t movie = isx::readMovie(filePath);
        isx::BasePlateType_t probeType = isx::getBasePlateType(movie);

        isx::BasePlateType_t expectedProbeType = isx::BasePlateType_t::CUSTOM;
        REQUIRE(probeType == expectedProbeType);
    }

    SECTION("IDAS metadata - No base plate")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/2021-06-28-23-34-09_video_sched_0_probe_none.isxd";
        isx::SpMovie_t movie = isx::readMovie(filePath);
        isx::BasePlateType_t probeType = isx::getBasePlateType(movie);

        isx::BasePlateType_t expectedProbeType = isx::BasePlateType_t::UNAVAILABLE;
        REQUIRE(probeType == expectedProbeType);
    }   

    SECTION("IDAS metadata - Cranial window base plate")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/movie_plate_cranial.isxd";
        isx::SpMovie_t movie = isx::readMovie(filePath);
        isx::BasePlateType_t probeType = isx::getBasePlateType(movie);

        isx::BasePlateType_t expectedProbeType = isx::BasePlateType_t::CRANIAL_WINDOW;
        REQUIRE(probeType == expectedProbeType);
    }

    SECTION("IDAS metadata - Unrecognizable base plate")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/movie_plate_unknown.isxd";
        isx::SpMovie_t movie = isx::readMovie(filePath);
        isx::BasePlateType_t probeType = isx::getBasePlateType(movie);

        isx::BasePlateType_t expectedProbeType = isx::BasePlateType_t::UNAVAILABLE;
        REQUIRE(probeType == expectedProbeType);
    }

    SECTION("IDPS metadata")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/2021-06-14-13-30-29_video_green_probe_change_idps.isxd";
        isx::SpMovie_t movie = isx::readMovie(filePath);
        isx::BasePlateType_t probeType = isx::getBasePlateType(movie);

        isx::BasePlateType_t expectedProbeType = isx::BasePlateType_t::BP32;
        REQUIRE(probeType == expectedProbeType);
    }

    SECTION("No metadata from IDPS or IDAS")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/cnmfe-cpp/movie_128x128x1000.isxd";
        isx::SpMovie_t movie = isx::readMovie(filePath);
        isx::BasePlateType_t probeType = isx::getBasePlateType(movie);

        isx::BasePlateType_t expectedProbeType = isx::BasePlateType_t::UNAVAILABLE;
        REQUIRE(probeType == expectedProbeType);
    }

    isx::CoreShutdown();
}

TEST_CASE("IntegratedBasePlate-Set", "[core]")
{
    isx::CoreInitialize();

    const std::string tmpFilePath = g_resources["unitTestDataPath"] + "/baseplate/tmp.isxd";

    SECTION("Initial metadata from IDAS")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/2021-06-14-13-30-29_video_green.isxd";
        REQUIRE(isx::copyFile(filePath, tmpFilePath));

        isx::DataSet* ds = new isx::DataSet("movie", isx::DataSet::Type::MOVIE, tmpFilePath, isx::HistoricalDetails());
        
        isx::BasePlateType_t probeTypeInitial = isx::getBasePlateType(ds);
        REQUIRE(probeTypeInitial == isx::BasePlateType_t::BP31);

        isx::setBasePlateType(ds, isx::BasePlateType_t::BP12);
        isx::BasePlateType_t probeTypeFinal = isx::getBasePlateType(ds);
        REQUIRE(probeTypeFinal == isx::BasePlateType_t::BP12);

        delete ds;
    }

    SECTION("Initial metadata from IDPS")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/2021-06-14-13-30-29_video_green_probe_change_idps.isxd";
        REQUIRE(isx::copyFile(filePath, tmpFilePath));

        isx::DataSet* ds = new isx::DataSet("movie", isx::DataSet::Type::MOVIE, tmpFilePath, isx::HistoricalDetails());
        
        isx::BasePlateType_t probeTypeInitial = isx::getBasePlateType(ds);
        REQUIRE(probeTypeInitial == isx::BasePlateType_t::BP32);

        isx::setBasePlateType(ds, isx::BasePlateType_t::BP14);
        isx::BasePlateType_t probeTypeFinal = isx::getBasePlateType(ds);
        REQUIRE(probeTypeFinal == isx::BasePlateType_t::BP14);

        delete ds;
    }
    
    SECTION("No initial metadata from IDPS or IDAS")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/cnmfe-cpp/movie_128x128x1000.isxd";
        REQUIRE(isx::copyFile(filePath, tmpFilePath));

        isx::DataSet* ds = new isx::DataSet("movie", isx::DataSet::Type::MOVIE, tmpFilePath, isx::HistoricalDetails());
        
        isx::BasePlateType_t probeTypeInitial = isx::getBasePlateType(ds);
        REQUIRE(probeTypeInitial == isx::BasePlateType_t::UNAVAILABLE);

        isx::setBasePlateType(ds, isx::BasePlateType_t::BP11);
        isx::BasePlateType_t probeTypeFinal = isx::getBasePlateType(ds);
        REQUIRE(probeTypeFinal == isx::BasePlateType_t::BP11);

        delete ds;
    }

    std::remove(tmpFilePath.c_str());
    isx::CoreShutdown();
}

TEST_CASE("SpatialDownSamplingFactor", "[core]")
{
    isx::CoreInitialize();

    SECTION("No Downsampling")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/metadata/spatialDownSamplingFactor_none.isxd";
        const isx::SpMovie_t movie = isx::readMovie(filePath);
        
        size_t spatialDs = isx::getSpatialDownSamplingFactor(movie);
        REQUIRE(spatialDs == 1);
    }

    SECTION("IDPS Downsampling Factor of 2")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/metadata/spatialDownSamplingFactor_idps2.isxd";
        const isx::SpMovie_t movie = isx::readMovie(filePath);
        
        size_t spatialDs = isx::getSpatialDownSamplingFactor(movie);
        REQUIRE(spatialDs == 2);
    }

    SECTION("IDAS Downsampling Factor of 2")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/metadata/spatialDownSamplingFactor_idas2.isxd";
        const isx::SpMovie_t movie = isx::readMovie(filePath);
        
        size_t spatialDs = isx::getSpatialDownSamplingFactor(movie);
        REQUIRE(spatialDs == 2);
    }

    SECTION("IDAS and IDPS Downsampling Factors of 2")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/metadata/spatialDownSamplingFactor_idas2_idps2.isxd";
        const isx::SpMovie_t movie = isx::readMovie(filePath);
        
        size_t spatialDs = isx::getSpatialDownSamplingFactor(movie);
        REQUIRE(spatialDs == 4);
    }

    isx::CoreShutdown();
}

TEST_CASE("getEfocus", "[core]")
{
    using json = nlohmann::json;
    isx::CoreInitialize();

    SECTION("Movie with no metadata")
    {
        const std::string inputFilename = g_resources["unitTestDataPath"] + "/cnmfe-cpp/movie_128x128x1000.isxd";
        const isx::SpMovie_t movie = isx::readMovie(inputFilename);

        json expectedExtraProps;
        expectedExtraProps["idps"] = nullptr;
        json extraProps = isx::getExtraPropertiesJSON(movie);
        REQUIRE(extraProps == expectedExtraProps);

        uint16_t expectedEfocus = 0;
        uint16_t efocus = isx::getEfocus(movie);
        REQUIRE(efocus == expectedEfocus);
    }

    SECTION("Movie with idps metadata")
    {
        const std::string inputFilename = g_resources["unitTestDataPath"] + "/baseplate/video-efocus_0500-Part1.isxd";
        const isx::SpMovie_t movie = isx::readMovie(inputFilename);

        uint16_t expectedEfocus = 500;
        json extraProps = isx::getExtraPropertiesJSON(movie);
        REQUIRE(extraProps["idps"]["efocus"].get<uint16_t>() == expectedEfocus);

        uint16_t efocus = isx::getEfocus(movie);
        REQUIRE(efocus == expectedEfocus);
    }

    SECTION("Movie with idas metadata")
    {
        const std::string inputFilename = g_resources["unitTestDataPath"] + "/baseplate/2021-06-28-23-34-09_video_sched_0_probe_none.isxd";
        const isx::SpMovie_t movie = isx::readMovie(inputFilename);

        uint16_t expectedEfocus = 760;
        json extraProps = isx::getExtraPropertiesJSON(movie);
        REQUIRE(extraProps["idps"] == nullptr);
        REQUIRE(extraProps["microscope"]["focus"].get<uint16_t>() == expectedEfocus);

        uint16_t efocus = isx::getEfocus(movie);
        REQUIRE(efocus == expectedEfocus);
    }

    isx::CoreShutdown();
}
