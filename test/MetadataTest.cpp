#include "catch.hpp"
#include "isxTest.h"

#include "isxMetadata.h"
#include "isxDataSet.h"
#include "isxMovieFactory.h"
#include "isxPathUtils.h"

// TODO: test case fails on linux teamcity but passes locally
// TEST_CASE("IntegratedBasePlate-Get")
// {
//     isx::CoreInitialize();
    
//     SECTION("From IDAS metadata")
//     {
//         SECTION("Integrated base plate")
//         {
//             isx::IntegratedBasePlateType_t probeType;
//             const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/2021-06-14-13-30-29_video_green.isxd";

//             SECTION("DataSet")
//             {
//                 isx::DataSet* ds = new isx::DataSet("movie", isx::DataSet::Type::MOVIE, filePath, isx::HistoricalDetails());
//                 probeType = isx::getIntegratedBasePlateType(ds);
//                 delete ds;
//             }

//             SECTION("Movie")
//             {
//                 isx::SpMovie_t movie = isx::readMovie(filePath);
//                 probeType = isx::getIntegratedBasePlateType(movie);
//             }

//             REQUIRE(probeType == isx::IntegratedBasePlateType_t::IBP5);
//         }

//         SECTION("Custom base plate")
//         {
//             isx::IntegratedBasePlateType_t probeType;
//             const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/2021-06-28-23-45-49_video_sched_0_probe_custom.isxd";

//             SECTION("DataSet")
//             {
//                 isx::DataSet* ds = new isx::DataSet("movie", isx::DataSet::Type::MOVIE, filePath, isx::HistoricalDetails());
//                 probeType = isx::getIntegratedBasePlateType(ds);
//                 delete ds;
//             }

//             SECTION("Movie")
//             {
//                 isx::SpMovie_t movie = isx::readMovie(filePath);
//                 probeType = isx::getIntegratedBasePlateType(movie);
//             }

//             REQUIRE(probeType == isx::IntegratedBasePlateType_t::CUSTOM);
//         }

//         SECTION("No base plate")
//         {
//             isx::IntegratedBasePlateType_t probeType;
//             const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/2021-06-28-23-34-09_video_sched_0_probe_none.isxd";

//             SECTION("DataSet")
//             {
//                 isx::DataSet* ds = new isx::DataSet("movie", isx::DataSet::Type::MOVIE, filePath, isx::HistoricalDetails());
//                 probeType = isx::getIntegratedBasePlateType(ds);
//                 delete ds;
//             }

//             SECTION("Movie")
//             {
//                 isx::SpMovie_t movie = isx::readMovie(filePath);
//                 probeType = isx::getIntegratedBasePlateType(movie);
//             }

//             REQUIRE(probeType == isx::IntegratedBasePlateType_t::UNAVAILABLE); 
//         }   

//         SECTION("Unrecognizable base plate")
//         {
//             isx::IntegratedBasePlateType_t probeType;
//             const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/video-efocus_0500-Part1.isxd";

//             SECTION("DataSet")
//             {
//                 isx::DataSet* ds = new isx::DataSet("movie", isx::DataSet::Type::MOVIE, filePath, isx::HistoricalDetails());
//                 probeType = isx::getIntegratedBasePlateType(ds);
//                 delete ds;
//             }

//             SECTION("Movie")
//             {
//                 isx::SpMovie_t movie = isx::readMovie(filePath);
//                 probeType = isx::getIntegratedBasePlateType(movie);
//             }

//             REQUIRE(probeType == isx::IntegratedBasePlateType_t::UNAVAILABLE);
//         }
//     }

//     SECTION("Read from IDPS metadata")
//     {
//         isx::IntegratedBasePlateType_t probeType;
//         const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/2021-06-14-13-30-29_video_green_probe_change_idps.isxd";

//         SECTION("DataSet")
//         {
//             isx::DataSet* ds = new isx::DataSet("movie", isx::DataSet::Type::MOVIE, filePath, isx::HistoricalDetails());
//             probeType = isx::getIntegratedBasePlateType(ds);
//             delete ds;
//         }

//         SECTION("Movie")
//         {
//             isx::SpMovie_t movie = isx::readMovie(filePath);
//             probeType = isx::getIntegratedBasePlateType(movie);
//         }

//         REQUIRE(probeType == isx::IntegratedBasePlateType_t::IBP15);
//     }

//     SECTION("No metadata from IDPS or IDAS")
//     {
//         isx::IntegratedBasePlateType_t probeType;
//         const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/blood_flow_movie.isxd";

//         SECTION("DataSet")
//         {
//             isx::DataSet* ds = new isx::DataSet("movie", isx::DataSet::Type::MOVIE, filePath, isx::HistoricalDetails());
//             probeType = isx::getIntegratedBasePlateType(ds);
//             delete ds;
//         }

//         SECTION("Movie")
//         {
//             isx::SpMovie_t movie = isx::readMovie(filePath);
//             probeType = isx::getIntegratedBasePlateType(movie);
//         }

//         REQUIRE(probeType == isx::IntegratedBasePlateType_t::UNAVAILABLE);    
//     }

//     isx::CoreShutdown();
// }

TEST_CASE("IntegratedBasePlate-Set", "[core]")
{
    isx::CoreInitialize();

    const std::string tmpFilePath = g_resources["unitTestDataPath"] + "/baseplate/tmp.isxd";

    SECTION("Initial metadata from IDAS")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/2021-06-14-13-30-29_video_green.isxd";
        REQUIRE(isx::copyFile(filePath, tmpFilePath));

        isx::DataSet* ds = new isx::DataSet("movie", isx::DataSet::Type::MOVIE, tmpFilePath, isx::HistoricalDetails());
        
        isx::IntegratedBasePlateType_t probeTypeInitial = isx::getIntegratedBasePlateType(ds);
        REQUIRE(probeTypeInitial == isx::IntegratedBasePlateType_t::IBP5);

        isx::setIntegratedBasePlateType(ds, isx::IntegratedBasePlateType_t::IBP12);
        isx::IntegratedBasePlateType_t probeTypeFinal = isx::getIntegratedBasePlateType(ds);
        REQUIRE(probeTypeFinal == isx::IntegratedBasePlateType_t::IBP12);

        delete ds;
    }

    SECTION("Initial metadata from IDPS")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/2021-06-14-13-30-29_video_green_probe_change_idps.isxd";
        REQUIRE(isx::copyFile(filePath, tmpFilePath));

        isx::DataSet* ds = new isx::DataSet("movie", isx::DataSet::Type::MOVIE, tmpFilePath, isx::HistoricalDetails());
        
        isx::IntegratedBasePlateType_t probeTypeInitial = isx::getIntegratedBasePlateType(ds);
        REQUIRE(probeTypeInitial == isx::IntegratedBasePlateType_t::IBP15);

        isx::setIntegratedBasePlateType(ds, isx::IntegratedBasePlateType_t::IBP14);
        isx::IntegratedBasePlateType_t probeTypeFinal = isx::getIntegratedBasePlateType(ds);
        REQUIRE(probeTypeFinal == isx::IntegratedBasePlateType_t::IBP14);

        delete ds;
    }
    
    SECTION("No initial metadata from IDPS or IDAS")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/baseplate/blood_flow_movie.isxd";
        REQUIRE(isx::copyFile(filePath, tmpFilePath));

        isx::DataSet* ds = new isx::DataSet("movie", isx::DataSet::Type::MOVIE, tmpFilePath, isx::HistoricalDetails());
        
        isx::IntegratedBasePlateType_t probeTypeInitial = isx::getIntegratedBasePlateType(ds);
        REQUIRE(probeTypeInitial == isx::IntegratedBasePlateType_t::UNAVAILABLE);

        isx::setIntegratedBasePlateType(ds, isx::IntegratedBasePlateType_t::IBP11);
        isx::IntegratedBasePlateType_t probeTypeFinal = isx::getIntegratedBasePlateType(ds);
        REQUIRE(probeTypeFinal == isx::IntegratedBasePlateType_t::IBP11);

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
        const std::string inputFilename = g_resources["unitTestDataPath"] + "/baseplate/blood_flow_movie.isxd";
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
