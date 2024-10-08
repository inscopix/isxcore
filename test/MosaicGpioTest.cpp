#include "isxTest.h"
#include "isxMosaicGpio.h"
#include "isxPathUtils.h"

#include "catch.hpp"

#include <atomic>
#include <chrono>
#include <thread>

TEST_CASE("MosaicGpioTest", "[core]")
{
    isx::CoreInitialize();

    SECTION("Invalid GPIO object")
    {
        isx::MosaicGpio gpio;
        REQUIRE(!gpio.isValid());
    }

    SECTION("Analog data")
    {
        std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/test_gpio_analog_2.isxd");
        std::shared_ptr<isx::MosaicGpio> gpio = std::make_shared<isx::MosaicGpio>(fileName);
        REQUIRE(gpio->isValid());
        const std::vector<std::string> channels = gpio->getChannelList();
        REQUIRE(channels.size() == 1);
        REQUIRE(channels.at(0) == "GPIO4_AI");
        REQUIRE(gpio->isAnalog("GPIO4_AI"));
        REQUIRE(gpio->getFileName() == fileName);
        REQUIRE(gpio->numberOfChannels() == 1);

        isx::SpFTrace_t trace = gpio->getAnalogData("GPIO4_AI");
        REQUIRE(trace);
        float val = trace->getValue(0);
        REQUIRE(val == 2.6556396484375);
    }

    SECTION("Analog data - async")
    {
        std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/test_gpio_analog_2.isxd");
        std::shared_ptr<isx::MosaicGpio> gpio = std::make_shared<isx::MosaicGpio>(fileName);

        std::atomic_int doneCount(0);
        size_t numTraces = 1;

        isx::MosaicGpio::GpioGetAnalogDataCB_t callBack = [&doneCount](isx::AsyncTaskResult<isx::SpFTrace_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            auto trace = inAsyncTaskResult.get();
            REQUIRE(trace);
            REQUIRE(trace->getValue(0) == 2.6556396484375);
            ++doneCount;
        };

        std::string channel("GPIO4_AI");

        gpio->getAnalogDataAsync(channel, callBack);


        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numTraces))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == int(numTraces));
    }

    SECTION("Logical data")
    {
        std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/test_gpio_events_2.isxd");
        std::shared_ptr<isx::MosaicGpio> gpio = std::make_shared<isx::MosaicGpio>(fileName);
        REQUIRE(gpio->isValid());
        const std::vector<std::string> channels = gpio->getChannelList();
        REQUIRE(channels.size() == 3);
        REQUIRE(channels.at(0) == "EX_LED");
        REQUIRE(channels.at(1) == "SYNC");
        REQUIRE(channels.at(2) == "TRIG");

        REQUIRE(!gpio->isAnalog("EX_LED"));
        REQUIRE(!gpio->isAnalog("SYNC"));
        REQUIRE(!gpio->isAnalog("TRIG"));
        REQUIRE(gpio->getFileName() == fileName);
        REQUIRE(gpio->numberOfChannels() == 3);

        isx::SpFTrace_t trace = gpio->getAnalogData("GPIO4_AI");
        REQUIRE(trace == nullptr);

        isx::SpLogicalTrace_t ledTrace = gpio->getLogicalData("EX_LED");
        REQUIRE(ledTrace != nullptr);

        const std::map<isx::Time, float> ledVals = ledTrace->getValues();
        REQUIRE(ledVals.size() == 2);
    }

    SECTION("Logical data - async")
    {
        std::atomic_int doneCount(0);
        size_t numTraces = 3;
        std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/test_gpio_events_2.isxd");
        std::shared_ptr<isx::MosaicGpio> gpio = std::make_shared<isx::MosaicGpio>(fileName);
        std::map<std::string, double> values{{"EX_LED", 1.5}, {"SYNC", 1.0}, {"TRIG", 1.0}};

        isx::MosaicGpio::GpioGetLogicalDataCB_t callBack = [&values, &doneCount](isx::AsyncTaskResult<isx::SpLogicalTrace_t> inAsyncTaskResult)
        {
            REQUIRE(!inAsyncTaskResult.getException());
            auto trace = inAsyncTaskResult.get();
            auto readValues = trace->getValues();
            REQUIRE(readValues.begin()->second == values.at(trace->getName()));
            ++doneCount;
        };

        const std::vector<std::string> channels = gpio->getChannelList();
        REQUIRE(channels.size() == numTraces);

        for (size_t i = 0; i < numTraces; ++i)
        {
            gpio->getLogicalDataAsync(channels.at(i), callBack);
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numTraces))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == int(numTraces));
    }

    isx::CoreShutdown();
}
