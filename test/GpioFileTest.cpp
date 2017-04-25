#include "isxTest.h"
#include "isxGpioFile.h"
#include "isxPathUtils.h"
#include "catch.hpp"

TEST_CASE("GpioFileTest", "[core]")
{
    isx::CoreInitialize();

    SECTION("Invalid file object") 
    {
        isx::GpioFile file; 
        REQUIRE(!file.isValid());
    }

    SECTION("Analog data file")
    {
        std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/test_gpio_analog.isxd");
        isx::GpioFile file(fileName); 
        REQUIRE(file.isValid());
        REQUIRE(file.isAnalog());
        REQUIRE(file.getFileName() == fileName);
        REQUIRE(file.numberOfChannels() == 1);
        const std::vector<std::string> channels = file.getChannelList();
        REQUIRE(channels.size() == 1);
        REQUIRE(channels.at(0) == "GPIO4_AI");

        isx::SpDTrace_t trace = file.getAnalogData();
        REQUIRE(trace);
        double val = trace->getValue(0);
        REQUIRE(val == 2.6556396484375);
    }

    SECTION("Logical data file")
    {
        std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/test_gpio_events.isxd");
        isx::GpioFile file(fileName); 
        REQUIRE(file.isValid());
        REQUIRE(!file.isAnalog());
        REQUIRE(file.getFileName() == fileName);
        REQUIRE(file.numberOfChannels() == 3);
        const std::vector<std::string> channels = file.getChannelList();
        REQUIRE(channels.size() == 3);
        REQUIRE(channels.at(0) == "EX_LED");
        REQUIRE(channels.at(1) == "SYNC");
        REQUIRE(channels.at(2) == "TRIG");

        isx::SpDTrace_t trace = file.getAnalogData();
        REQUIRE(trace == nullptr);
        
        isx::SpLogicalTrace_t ledTrace = file.getLogicalData("EX_LED");
        REQUIRE(ledTrace != nullptr);

        const std::map<isx::Time, double> ledVals = ledTrace->getValues();
        REQUIRE(ledVals.size() == 2);
    }

    isx::CoreShutdown();
}