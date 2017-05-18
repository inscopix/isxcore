#include "isxNVokeGpioFile.h"
#include "isxTest.h"
#include "isxPathUtils.h"
#include "isxJsonUtils.h"
#include "isxException.h"
#include "isxDataSet.h"

#include "catch.hpp"

#include <fstream>
#include <algorithm>

void testParsing(
    const std::string & inFileName,
    const std::string & inOutputDir,
    const std::vector<std::string> & inExpectedSuffixes,
    const std::vector<isx::json> inFileJsonHeaders, 
    const std::vector<std::vector<isx::json>> inChannelJsonHeaders,
    const std::vector<std::vector<uint64_t>> & inMicroSecs, 
    const std::vector<std::vector<bool>> & inStates,
    const std::vector<std::vector<float>> & inPowerLevel
    )
{
    std::string dir = isx::getDirName(inFileName);
    std::string base = isx::getBaseName(inFileName);

    for(auto & expected : inExpectedSuffixes)
    {
        std::string fullName = dir + "/" + base + expected + ".isxd";
        std::remove(fullName.c_str());
    }

    isx::NVokeGpioFile raw(inFileName, inOutputDir);
    try
    {
        raw.parse();
    }
    catch(const isx::ExceptionDataIO & error)
    {
        FAIL("There was a DataIO exception when parsing file: ", error.what());
    }
    catch(const isx::ExceptionFileIO & error)
    {
        FAIL("There was a FileIO exception when parsing file: ", error.what());
    }

    std::vector<std::string> filenames;
    raw.getOutputFileNames(filenames);

    REQUIRE(filenames.size() == inExpectedSuffixes.size());

    isx::isize_t idx = 0;
    for(auto & expected : inExpectedSuffixes)
    {
        // Make sure the expected file was produced
        std::string fullName = dir + "/" + base + expected + ".isxd";
        auto result = std::find(filenames.begin(), filenames.end(), fullName);
        bool found = result != std::end(filenames);
        REQUIRE(found);

        
        if(found)
        {
            // Compare file json headers
            std::fstream file(fullName, std::ios::binary | std::ios_base::in);
            if (!file.good() || !file.is_open())
            {
                ISX_THROW(isx::ExceptionFileIO,
                    "Failed to open gpio file for reading: ", fullName);
            }

            std::ios::pos_type headerPos;
            isx::json j = isx::readJsonHeaderAtEnd(file, headerPos);
            auto & expectedHdr = inFileJsonHeaders.at(idx);
            REQUIRE(j == expectedHdr);

            auto & channelHeaders = inChannelJsonHeaders.at(idx);
            auto & timeMicroSecs = inMicroSecs.at(idx);
            auto & states = inStates.at(idx);
            auto & powerLevels = inPowerLevel.at(idx);
            auto offsets = j["channel offsets"];

            isx::isize_t channelIdx = 0;
            for(auto & expectedHeader : channelHeaders)
            {
                /// Read channel header and compare
                std::string channel = expectedHeader.at("channel");
                file.seekg(offsets.at(channel), std::ios_base::beg);
                isx::json header = isx::readJson(file);
                REQUIRE(header == expectedHeader);

                /// Read first data value for channel
                isx::GpioFile::DataPkt pkt;

                file.read((char *) &pkt, sizeof(pkt));                
                if (!file.good())
                {
                    ISX_THROW(isx::ExceptionFileIO,
                        "Failed to read values from  gpio file: ", fullName);
                }

                REQUIRE(pkt.m_timeStampUSec == timeMicroSecs.at(channelIdx));
                REQUIRE(pkt.getState() == states.at(channelIdx));
                REQUIRE(pkt.getValue() == powerLevels.at(channelIdx));

                ++channelIdx;
            }      

        }
        else
        {
            std::cout << "Exiting prematurely" << std::endl;
            return;
        }

        idx++;
    }    

    for(auto & fn : filenames)
    {
        std::remove(fn.c_str());
    }
}

// TODO: MOS-584 merge fix
TEST_CASE("GpioDataTest", "[core]")
{

    isx::CoreInitialize();
    


    SECTION("Parse a GPIO file - ANALOG and SYNC") 
    {
        std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/recording_20161104_093749_gpio.raw");
        std::string outputDir = isx::getAbsolutePath(g_resources["unitTestDataPath"]);

        ///////////////////////////////////////////////////////////////
        // Expected values *********************************************
        std::vector<std::string> expectedSuffixes {"_analog", "_events"};        
        
        isx::Time start(isx::DurationInSeconds(1478277469, 1) + isx::DurationInSeconds(294107, 1000000));
        isx::DurationInSeconds step(1, 1000);
        isx::isize_t numTimes = 3640;
        isx::TimingInfo ti(start, step, numTimes);

        isx::json analogHeader;
        std::map<std::string, int> analogChannelOffsets{{"GPIO4_AI", 0}};
        analogHeader["type"] = size_t(isx::DataSet::Type::GPIO);
        analogHeader["channel offsets"] = analogChannelOffsets;
        analogHeader["timing info"] = convertTimingInfoToJson(ti);
        analogHeader["producer"] = isx::getProducerAsJson();
        analogHeader["fileVersion"] = 0;

        isx::json eventsHeader;
        std::map<std::string, int> eventsChannelOffsets{{"SYNC", 0}};
        eventsHeader["type"] = size_t(isx::DataSet::Type::GPIO);
        eventsHeader["channel offsets"] = eventsChannelOffsets;
        eventsHeader["timing info"] = convertTimingInfoToJson(ti);
        eventsHeader["producer"] = isx::getProducerAsJson();
        eventsHeader["fileVersion"] = 0;

        std::vector<isx::json> fileHeaders{analogHeader, eventsHeader};

        isx::json analog;
        analog["channel"] = "GPIO4_AI";
        analog["mode"] = "";
        analog["GPIO Trigger/Follow"] = "";
        analog["Number of Packets"] = 0;

        isx::json sync;
        sync["channel"] = "SYNC";
        sync["mode"] = "Output Manual Mode";
        sync["GPIO Trigger/Follow"] = "";
        sync["Number of Packets"] = 145;

        std::vector<std::vector<isx::json>> channelHeaders{std::vector<isx::json>{analog}, std::vector<isx::json>{sync}};

        std::vector<std::vector<uint64_t>> usecsFromUnix{ std::vector<uint64_t>{(uint64_t)1478277469 * 1000000 + 294107}, std::vector<uint64_t>{(uint64_t)1478277469 * 1000000 + 311800}};
        std::vector<std::vector<bool>> states{std::vector<bool>{false}, std::vector<bool>{false}};
        std::vector<std::vector<float>> power{std::vector<float>{float(2.6556396484375)}, std::vector<float>{0.0f} };

        // End of expected values ********************************************
        //////////////////////////////////////////////////////////////////////

        testParsing(
            fileName,
            outputDir,
            expectedSuffixes,
            fileHeaders,
            channelHeaders, 
            usecsFromUnix,
            states,
            power);
    }

    SECTION("Parse a GPIO file - EX_LED, SYNC and TRIG") 
    {
        std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/recording_20170126_143728_gpio.raw");
        std::string outputDir = isx::getAbsolutePath(g_resources["unitTestDataPath"]);

        ///////////////////////////////////////////////////////////////
        // Expected values *********************************************
        std::vector<std::string> expectedSuffixes {"_events"};        
        
        isx::Time start(isx::DurationInSeconds(1485470243, 1) + isx::DurationInSeconds(163233, 1000000));
        isx::DurationInSeconds step(1, 1000);
        isx::isize_t numTimes = 9153;
        isx::TimingInfo ti(start, step, numTimes);

        isx::json eventsHeader;
        std::map<std::string, int> channelOffsets{{"EX_LED", 0}, {"SYNC", 151}, {"TRIG", 3056}};
        eventsHeader["type"] = size_t(isx::DataSet::Type::GPIO);
        eventsHeader["channel offsets"] = channelOffsets;
        eventsHeader["timing info"] = convertTimingInfoToJson(ti);
        eventsHeader["producer"] = isx::getProducerAsJson();
        eventsHeader["fileVersion"] = 0;

        std::vector<isx::json> fileHeaders{eventsHeader};

        isx::json exled;
        exled["channel"] = "EX_LED";
        exled["mode"] = "Manual Mode";
        exled["GPIO Trigger/Follow"] = "GPIO1";
        exled["Number of Packets"] = 2;

        isx::json sync;
        sync["channel"] = "SYNC";
        sync["mode"] = "Output Manual Mode";
        sync["GPIO Trigger/Follow"] = "";
        sync["Number of Packets"] = 174;

        isx::json trig;
        trig["channel"] = "TRIG";
        trig["mode"] = "Output Manual Mode";
        trig["GPIO Trigger/Follow"] = "";
        trig["Number of Packets"] = 4;

        std::vector<std::vector<isx::json>> channelHeaders{std::vector<isx::json>{exled, sync, trig}};

        std::vector<std::vector<uint64_t>> usecsFromUnix{ std::vector<uint64_t>{(uint64_t)1485470246 * 1000000 + 240986, (uint64_t)1485470247 * 1000000 + 38600, (uint64_t)1485470243 * 1000000 + 163233}};
        std::vector<std::vector<bool>> states{std::vector<bool>{true, true, true}};
        std::vector<std::vector<float>> power{ std::vector<float>{1.5f, 0.0f, 0.0f} };

        // End of expected values ********************************************
        //////////////////////////////////////////////////////////////////////
        
        testParsing(
            fileName,
            outputDir,
            expectedSuffixes,
            fileHeaders,
            channelHeaders, 
            usecsFromUnix,
            states,
            power);

        
    }

    isx::CoreShutdown();
}
