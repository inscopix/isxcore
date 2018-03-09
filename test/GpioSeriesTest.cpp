#include "isxCore.h"
#include "isxGpioSeries.h"
#include "isxGpioImporter.h"
#include "isxPathUtils.h"

#include "catch.hpp"
#include "isxTest.h"
#include <array>

namespace
{

std::vector<std::string>
convertNVokeGpio(const std::string & inFilePath, const std::string & inOutDirPath)
{
    const isx::GpioDataParams inputParams(inOutDirPath, inFilePath);
    auto outputParams = std::make_shared<isx::GpioDataOutputParams>();
    runGpioDataImporter(inputParams, outputParams, [](float){return false;});
    return outputParams->filenames;
}

} // namespace

TEST_CASE("GpioSeries-GpioSeries", "[core-internal]")
{
    const std::string dirPath = g_resources["unitTestDataPath"] + "/nVokeGpio";
    std::vector<std::string> filePaths;

    isx::CoreInitialize();

    SECTION("Empty constructor")
    {
        const auto gpio = std::make_shared<isx::GpioSeries>();
        REQUIRE(!gpio->isValid());
    }

    SECTION("Two compatible logical GPIO sets")
    {
        const std::array<const char *, 2> names =
        { {
            "recording_20160919_095413_gpio.raw",
            "recording_20160919_100153_gpio.raw"
        } };

        for (const auto n : names)
        {
            const std::string inputFilePath = dirPath + "/" + n;
            const std::vector<std::string> outputFilePaths = convertNVokeGpio(inputFilePath, dirPath);
            filePaths.push_back(outputFilePaths.front());
        }

        const isx::SpGpio_t gpios = isx::readGpioSeries(filePaths);

        const isx::SpLogicalTrace_t exLedTrace = gpios->getLogicalData("EX_LED");
        const std::map<isx::Time, float> & exLedValues = exLedTrace->getValues();
        for (const auto & kv : exLedValues)
        {
            REQUIRE(kv.second == 0.5f);
        }

        const isx::SpLogicalTrace_t syncTrace = gpios->getLogicalData("SYNC");
        const std::map<isx::Time, float> & syncValues = syncTrace->getValues();
        size_t counter = 0;
        for (const auto & kv : syncValues)
        {
            if (counter == 0)
            {
                REQUIRE(kv.second == 1.f);
            }
            else if (counter == 34226)
            {
                REQUIRE(kv.second == 1.f);
            }
            ++counter;
        }

        const isx::SpLogicalTrace_t trigTrace = gpios->getLogicalData("TRIG");
        const std::map<isx::Time, float> & trigValues = trigTrace->getValues();
        for (const auto & kv : trigValues)
        {
            REQUIRE(kv.second == 1.f);
        }
    }

    SECTION("Two logical GPIO sets with a different number channels")
    {
        const std::array<std::string, 2> inputFilePaths =
        { {
            dirPath + "/recording_20160919_095413_gpio.raw",
            g_resources["unitTestDataPath"] + "/recording_20161104_093749_gpio.raw"
        } };

        for (const auto f : inputFilePaths)
        {
            const std::vector<std::string> outputFilePaths = convertNVokeGpio(f, dirPath);
            filePaths.push_back(outputFilePaths.front());
        }
        
        ISX_REQUIRE_EXCEPTION(isx::readGpioSeries(filePaths), isx::ExceptionSeries,
                "GPIO series member with mismatching channels.");
    }

    SECTION("Two logical GPIO sets with temporal overlap (they're actually the same)")
    {
        const std::array<const char *, 2> names =
        { {
            "recording_20160919_095413_gpio.raw",
            "recording_20160919_095413_gpio-copy.raw"
        } };

        for (const auto n : names)
        {
            const std::string inputFilePath = dirPath + "/" + n;
            const std::vector<std::string> outputFilePaths = convertNVokeGpio(inputFilePath, dirPath);
            filePaths.push_back(outputFilePaths.front());
        }

        ISX_REQUIRE_EXCEPTION(isx::readGpioSeries(filePaths), isx::ExceptionSeries,
                "Unable to insert data that temporally overlaps with other parts of the series. Data sets in a series must all be non-overlapping.");
    }

    for (const auto & f : filePaths)
    {
        std::remove(f.c_str());
    }


    isx::CoreShutdown();
}
