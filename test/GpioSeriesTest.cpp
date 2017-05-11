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
    std::vector<std::string> eventsFilePaths;
    std::vector<std::string> analogFilePaths;

    isx::CoreInitialize();

    SECTION("Empty constructor")
    {
        const auto gpio = std::make_shared<isx::GpioSeries>();
        REQUIRE(!gpio->isValid());
    }

    SECTION("Two compatiable logical GPIO sets")
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
            eventsFilePaths.push_back(outputFilePaths.front());
        }

        const isx::SpGpio_t gpios = isx::readGpioSeries(eventsFilePaths);
    }

    SECTION("Logical and analog GPIO sets")
    {
        const std::array<std::string, 2> inputFilePaths =
        { {
            dirPath + "/recording_20160919_095413_gpio.raw",
            g_resources["unitTestDataPath"] + "/recording_20161104_093749_gpio.raw"
        } };

        for (const auto f : inputFilePaths)
        {
            const std::vector<std::string> outputFilePaths = convertNVokeGpio(f, dirPath);
            eventsFilePaths.push_back(outputFilePaths.front());
            if (outputFilePaths.size() > 1)
            {
                analogFilePaths.push_back(outputFilePaths.back());
            }
        }
        const std::vector<std::string> filePaths = {eventsFilePaths[0], analogFilePaths[0]};

        ISX_REQUIRE_EXCEPTION(isx::readGpioSeries(filePaths), isx::ExceptionSeries,
                "GPIO series member with mismatching analog/digital data: " + filePaths.back());
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
            eventsFilePaths.push_back(outputFilePaths.front());
        }

        ISX_REQUIRE_EXCEPTION(isx::readGpioSeries(eventsFilePaths), isx::ExceptionSeries,
                "GPIO series member with mismatching number of logical channels: " + eventsFilePaths.back());
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
            eventsFilePaths.push_back(outputFilePaths.front());
        }

        ISX_REQUIRE_EXCEPTION(isx::readGpioSeries(eventsFilePaths), isx::ExceptionSeries,
                "The timing info temporally overlaps with the reference.");
    }

    for (const auto & f : eventsFilePaths)
    {
        std::remove(f.c_str());
    }
    for (const auto & f : analogFilePaths)
    {
        std::remove(f.c_str());
    }

    isx::CoreShutdown();
}
