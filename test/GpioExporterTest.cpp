#include "catch.hpp"
#include "isxTest.h"

#include "isxGpio.h"
#include "isxGpioExporter.h"
#include "isxGpioImporter.h"

#include <cstring>
#include <fstream>

TEST_CASE("GpioExport", "[core]")
{
    const std::string outputFileName = g_resources["unitTestDataPath"] + "/GpioExport-output.csv";
    std::remove(outputFileName.c_str());

    isx::CoreInitialize();

    SECTION("Export one analog GPIO trace")
    {
        const std::string inputFileName = g_resources["unitTestDataPath"] + "/test_gpio_analog_2.isxd";
        const isx::SpGpio_t gpio = isx::readGpio(inputFileName);

        isx::GpioExporterParams params({gpio}, outputFileName, isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);

        std::vector<std::string> expLines;

        SECTION("using first time")
        {
            params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::FIRST_DATA_ITEM;

            expLines =
            {
                "Time (s), GPIO4_AI",
                "0, 2.65564",
                "0.001, 2.532349",
                "0.002, 2.403412",
            };
        }

        SECTION("using unix time")
        {
            params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::UNIX_EPOCH;

            expLines =
            {
                "Time (s), GPIO4_AI",
                "1478277469.294107, 2.65564",
                "1478277469.295107, 2.532349",
                "1478277469.296107, 2.403412",
            };
        }

        isx::runGpioExporter(params);

        std::ifstream outputFile(outputFileName);
        std::string line;
        for (const auto & expLine : expLines)
        {
            getline(outputFile, line);
            REQUIRE(line == expLine);
        }
    }

    SECTION("Export one logical GPIO trace set")
    {
        const std::string inputFileName = g_resources["unitTestDataPath"] + "/test_gpio_events_2.isxd";
        const isx::SpGpio_t gpio = isx::readGpio(inputFileName);

        isx::GpioExporterParams params({gpio}, outputFileName, isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);
        isx::runGpioExporter(params);

        const std::map<size_t, std::string> expLines =
        {
            {0, "Time (s), Channel Name, Value"},
            {1, "3.077753, EX_LED, 1.5"},
            {2, "9.152908, EX_LED, 0"},
            {3, "3.875367, SYNC, 1"},
            {176, "8.198840000000001, SYNC, 0"},
            {177, "0, TRIG, 1"},
            {180, "0.300178, TRIG, 0"},
        };

        size_t lineIndex = 0;
        std::string line;
        std::ifstream outputFile(outputFileName);
        while (getline(outputFile, line))
        {
            if (expLines.count(lineIndex) > 0)
            {
                REQUIRE(line == expLines.at(lineIndex));
            }
            ++lineIndex;
        }
    }

    SECTION("Export two logical GPIO trace sets")
    {
        const std::string inputDir = g_resources["unitTestDataPath"] + "/nVokeGpio";

        const std::vector<std::string> gpioFileBases = {
            inputDir + "/recording_20160919_095413_gpio",
            inputDir + "/recording_20160919_100153_gpio",
        };

        std::vector<isx::SpGpio_t> gpios;
        std::vector<std::string> gpioFiles;
        for (const auto & f : gpioFileBases)
        {
            auto outputParams = std::make_shared<isx::GpioDataOutputParams>();
            runGpioDataImporter(isx::GpioDataParams(inputDir, f + ".raw"), outputParams, [](float){return false;});
            gpios.push_back(isx::readGpio(f + "_events.isxd"));
            gpioFiles.insert(gpioFiles.end(), outputParams->filenames.begin(), outputParams->filenames.end());
        }

        isx::GpioExporterParams params(gpios, outputFileName, isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);
        isx::runGpioExporter(params);

        const std::map<size_t, std::string> expLines =
        {
            {0, "Time (s), Channel Name, Value"},
            {1, "0.009131999999999999, EX_LED, 0.5"},
            {2, "459.58963, EX_LED, 0.5"},
            {3, "0.114279, SYNC, 1"},
            {34229, "2259.368452, SYNC, 1"},
            {34230, "0, TRIG, 1"},
            {34231, "459.562282, TRIG, 1"},
        };

        size_t lineIndex = 0;
        std::string line;
        std::ifstream outputFile(outputFileName);
        while (getline(outputFile, line))
        {
            if (expLines.count(lineIndex) > 0)
            {
                REQUIRE(line == expLines.at(lineIndex));
            }
            ++lineIndex;
        }

        for (const auto & f : gpioFiles)
        {
            std::remove(f.c_str());
        }
    }

    isx::CoreShutdown();

    std::remove(outputFileName.c_str());
}
