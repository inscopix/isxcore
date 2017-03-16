#include "isxGpioDataFile.h"
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
    const std::vector<isx::json> inJsonHeaders, 
    const std::vector<uint32_t> & inSecs,
    const std::vector<double> & inMicroSecs, 
    const std::vector<char> & inStates,
    const std::vector<double> & inPowerLevel
    )
{
    std::string dir = isx::getDirName(inFileName);
    std::string base = isx::getBaseName(inFileName);

    for(auto & expected : inExpectedSuffixes)
    {
        std::string fullName = dir + "/" + base + expected + ".isxd";
        std::remove(fullName.c_str());
    }

    isx::GpioDataFile raw(inFileName, inOutputDir);
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
            // Compare json headers
            std::fstream file(fullName, std::ios::binary | std::ios_base::in);
            if (!file.good() || !file.is_open())
            {
                ISX_THROW(isx::ExceptionFileIO,
                    "Failed to open gpio file for reading: ", fullName);
            }

            std::ios::pos_type headerPos;
            isx::json j = isx::readJsonHeaderAtEnd(file, headerPos);
            auto & expectedHdr = inJsonHeaders.at(idx);
            REQUIRE(j == expectedHdr);

            // Compare first values
            uint32_t timeStampSec, timeStampUSec;
            char state;
            double powerLevel;

            file.seekg(0, std::ios_base::beg);

            file.read((char *) &timeStampSec, sizeof(uint32_t));
            file.read((char *) &timeStampUSec, sizeof(uint32_t));
            file.read((char *) &state, sizeof(char));
            file.read((char *) &powerLevel, sizeof(double));
            if (!file.good())
            {
                ISX_THROW(isx::ExceptionFileIO,
                    "Failed to read values from  gpio file: ", fullName);
            }            

            REQUIRE(timeStampSec == inSecs.at(idx));

            double dTimeStampUSec = ((double)timeStampUSec) / 1000000.0;
            REQUIRE(std::abs(dTimeStampUSec - inMicroSecs.at(idx)) < 0.001);
            REQUIRE(state == inStates.at(idx));
            REQUIRE(std::abs(powerLevel - inPowerLevel.at(idx)) < 0.01);

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
        std::vector<std::string> expectedSuffixes {
            "_GPIO4_AI",
            "_SYNC"
        };        
        
        isx::json gpio4; 
        gpio4["type"] = size_t(isx::DataSet::Type::GPIO);
        gpio4["signal"] = "GPIO4_AI";
        gpio4["mode"] = "";
        gpio4["GPIO Trigger/Follow"] = "";

        isx::json sync; 
        sync["type"] = size_t(isx::DataSet::Type::GPIO);
        sync["signal"] = "SYNC";
        sync["mode"] = "Output Manual Mode";
        sync["GPIO Trigger/Follow"] = "";


        std::vector<isx::json> headers{gpio4, sync};

        std::vector<uint32_t> secsFromUnix{ 1478277469, 1478277469};
        std::vector<double> microSecs{.2941, .3118};
        std::vector<char> states{0, 0};
        std::vector<double> power{ 2.6556396484375, 0.0 };

        // End of expected values ********************************************
        //////////////////////////////////////////////////////////////////////

        testParsing(
            fileName,
            outputDir,
            expectedSuffixes,
            headers, 
            secsFromUnix,
            microSecs, 
            states,
            power);
    }

    SECTION("Parse a GPIO file - EX_LED, SYNC and TRIG") 
    {
        std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/recording_20170126_143728_gpio.raw");
        std::string outputDir = isx::getAbsolutePath(g_resources["unitTestDataPath"]);

        ///////////////////////////////////////////////////////////////
        // Expected values *********************************************
        std::vector<std::string> expectedSuffixes {
            "_EX_LED",
            "_SYNC",
            "_TRIG"
        };
    
        isx::json exled; 
        exled["type"] = size_t(isx::DataSet::Type::GPIO);
        exled["signal"] = "EX_LED";
        exled["mode"] = "Manual Mode";
        exled["GPIO Trigger/Follow"] = "GPIO1";

        isx::json sync; 
        sync["type"] = size_t(isx::DataSet::Type::GPIO);
        sync["signal"] = "SYNC";
        sync["mode"] = "Output Manual Mode";
        sync["GPIO Trigger/Follow"] = "";

        isx::json trig; 
        trig["type"] = size_t(isx::DataSet::Type::GPIO);
        trig["signal"] = "TRIG";
        trig["mode"] = "Output Manual Mode";
        trig["GPIO Trigger/Follow"] = "";


        std::vector<isx::json> headers{exled, sync, trig};

        std::vector<uint32_t> secsFromUnix{ 1485470246, 1485470247, 1485470243};
        std::vector<double> microSecs{.24098, .0386, .16323};
        std::vector<char> states{1, 1, 1};
        std::vector<double> power{ 1.5, 0.0, 0.0 };

        // End of expected values ********************************************
        //////////////////////////////////////////////////////////////////////
        
        testParsing(
            fileName,
            outputDir,
            expectedSuffixes,
            headers, 
            secsFromUnix,
            microSecs, 
            states,
            power);

        
    }

    isx::CoreShutdown();
}