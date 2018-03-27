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

    SECTION("Export one analog GPIO trace - V1")
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
                "Time (s), Channel Name, Value",
                "0, GPIO4_AI, 2.65564",
                "0.001, GPIO4_AI, 2.532349",
                "0.002, GPIO4_AI, 2.403412",
            };
        }

        SECTION("using unix time")
        {
            params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::UNIX_EPOCH;

            expLines =
            {
                "Time (s), Channel Name, Value",
                "1478277469.294107, GPIO4_AI, 2.65564",
                "1478277469.295107, GPIO4_AI, 2.532349",
                "1478277469.296107, GPIO4_AI, 2.403412",
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

    SECTION("Export one logical GPIO trace set - V1")
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

    SECTION("Export one analog and one logical GPIO trace from isxd V2")
    {
        // This file comes from running the import step on this nVoke GPIO file:
        // g_resources["unitTestDataPath"] + "/recording_20161104_093749_gpio.raw"
        // If the events file changes in the future, you will need to re-run the importer
        // to generate a new input file.
        const std::string nVokeDir = "/nVokeGpio";
        const std::string inputFileName = g_resources["unitTestDataPath"] + nVokeDir + "/test_nvoke_gpio_V2-2.isxd";
        const isx::SpGpio_t gpio = isx::readGpio(inputFileName);

        isx::GpioExporterParams params({gpio}, outputFileName, isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);

        std::map<size_t, std::string> expLines;

        SECTION("using first time")
        {
            params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::FIRST_DATA_ITEM;

            expLines =
            {
                {0, "Time (s), Channel Name, Value"},
                {1, "0, GPIO4_AI, 2.65564" },
                {2, "0.001, GPIO4_AI, 2.532349" },
                {3, "0.002, GPIO4_AI, 2.403412"},
                {3783, "3.56642, SYNC, 0"},
                {3784, "3.591411, SYNC, 1"},
                {3785, "3.616402, SYNC, 0"}
            };
        }

        SECTION("using unix time")
        {
            params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::UNIX_EPOCH;

            expLines =
            {
                {0, "Time (s), Channel Name, Value"},
                {1, "1478277469.294107, GPIO4_AI, 2.65564" },
                {2, "1478277469.295107, GPIO4_AI, 2.532349" },
                {3, "1478277469.296107, GPIO4_AI, 2.403412"},
                {3783, "1478277472.860527, SYNC, 0"},
                {3784, "1478277472.885518, SYNC, 1"},
                {3785, "1478277472.910509, SYNC, 0"}
            };
        }

        isx::runGpioExporter(params);

        std::ifstream outputFile(outputFileName);
        size_t lineIndex = 0;
        std::string line;
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
            gpios.push_back(isx::readGpio(f + "_gpio.isxd"));
            gpioFiles.insert(gpioFiles.end(), outputParams->filenames.begin(), outputParams->filenames.end());
        }

        isx::GpioExporterParams params(gpios, outputFileName, isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);
        isx::runGpioExporter(params);

        const std::map<size_t, std::string> expLines =
        {
            {0, "Time (s), Channel Name, Value"},
            {1, "0, TRIG, 1" },
            {2, "459.562282, TRIG, 1" },
            {3, "0.009131999999999999, EX_LED, 0.5"},
            {4, "459.58963, EX_LED, 0.5"},
            {5, "0.114279, SYNC, 1"},
            {34231, "2259.368452, SYNC, 1"}
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
