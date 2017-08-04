#include "catch.hpp"
#include "isxTest.h"

#include "isxWritableEvents.h"
#include "isxEventsExporter.h"
#include "isxPathUtils.h"

#include <cstring>
#include <fstream>
#include <array>

namespace
{

uint64_t
convertTimeToUSecsSinceEpoch(const isx::Time & inTime)
{
    return uint64_t(inTime.getSecsSinceEpoch().toDouble() * 1E6);
}

isx::SpEvents_t
writeNamedPacketsAsEvents(
        const std::string & inFilePath,
        const isx::TimingInfo & inTi,
        const std::map<std::string, std::map<isx::Time, float>> & inPackets)
{
    isx::SpWritableEvents_t events = isx::writeEvents(inFilePath);
    for (const auto c : inPackets)
    {
        events->writeCellHeader(c.first, c.second.size());
        for (const auto e : c.second)
        {
            events->writeDataPkt(convertTimeToUSecsSinceEpoch(e.first), e.second);
        }
    }
    events->setTimingInfo(inTi);
    events->closeForWriting();
    return isx::readEvents(inFilePath);
}

void
requireEqualLines(
        const std::string inFilePath,
        const std::vector<std::string> inExpLines)
{
    std::ifstream outputFile(inFilePath);
    std::string line;
    for (const auto & expLine : inExpLines)
    {
        getline(outputFile, line);
        REQUIRE(line == expLine);
    }
}

} // namespace

TEST_CASE("EventsExport", "[core]")
{
    isx::CoreInitialize();

    const std::string inputDir = g_resources["unitTestDataPath"] + "/events-export";
    isx::makeDirectory(inputDir);

    const std::string inputBase = inputDir + "/events";
    const std::string inputFilePath = inputBase + ".isxd";
    const std::string outputFilePath = inputBase + ".csv";

    const isx::Time start(2017, 7, 26, 9, 51, 23, isx::DurationInSeconds(0, 1000));

    const isx::TimingInfo ti(start, isx::DurationInSeconds(50, 1000), 5);

    const std::map<std::string, std::map<isx::Time, float>> eventPackets =
    {
        {"C0",
            {
                {start + isx::DurationInSeconds(0, 1000), 1.0f},
                {start + isx::DurationInSeconds(50, 1000), 3.0f},
                {start + isx::DurationInSeconds(200, 1000), 0.8f},
            }
        },
        {"C1",
            {
                {start + isx::DurationInSeconds(50, 1000), 2.5f},
            }
        },
        {"C2",
            {
                {start + isx::DurationInSeconds(150, 1000), 2.9f},
                {start + isx::DurationInSeconds(200, 1000), 1.7f},
            }
        },
    };

    const isx::SpEvents_t events = writeNamedPacketsAsEvents(inputFilePath, ti, eventPackets);

    isx::EventsExporterParams params({events}, outputFilePath, isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);

    std::vector<std::string> expLines;

    SECTION("using first time")
    {
        params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::FIRST_DATA_ITEM;

        expLines =
        {
            "Time (s), Cell Name, Value",
            "0, C0, 1",
            "0.05, C0, 3",
            "0.2, C0, 0.8",
            "0.05, C1, 2.5",
            "0.15, C2, 2.9",
            "0.2, C2, 1.7",
        };
    }

    SECTION("using unix epoch")
    {
        params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::UNIX_EPOCH;

        expLines =
        {
            "Time (s), Cell Name, Value",
            "1501062683, C0, 1",
            "1501062683.05, C0, 3",
            "1501062683.2, C0, 0.8",
            "1501062683.05, C1, 2.5",
            "1501062683.15, C2, 2.9",
            "1501062683.2, C2, 1.7",
        };
    }

    isx::runEventsExporter(params);

    requireEqualLines(outputFilePath, expLines);

    isx::removeDirectory(inputDir);

    isx::CoreShutdown();
}

TEST_CASE("EventsExport-series", "[core]")
{
    isx::CoreInitialize();

    const std::string inputDir = g_resources["unitTestDataPath"] + "/events-export";
    isx::makeDirectory(inputDir);

    const std::string inputBase = inputDir + "/events";
    const std::array<std::string, 2> inputFilePaths =
    {{
        inputBase + "_0.isxd",
        inputBase + "_1.isxd",
    }};
    const std::string outputFilePath = inputBase + ".csv";

    const std::array<isx::Time, 2> starts =
    {{
        isx::Time(2017, 7, 26, 9, 51, 23, isx::DurationInSeconds(0, 1000)),
        isx::Time(2017, 7, 26, 9, 51, 28, isx::DurationInSeconds(0, 1000)),
    }};

    const std::array<isx::TimingInfo, 2> tis =
    {{
        isx::TimingInfo(starts[0], isx::DurationInSeconds(50, 1000), 5),
        isx::TimingInfo(starts[1], isx::DurationInSeconds(50, 1000), 4),
    }};

    const std::array<std::map<std::string, std::map<isx::Time, float>>, 2> eventPackets =
    {{
        {
            {"C0",
                {
                    {starts[0] + isx::DurationInSeconds(0, 1000), 1.0f},
                    {starts[0] + isx::DurationInSeconds(50, 1000), 3.0f},
                    {starts[0] + isx::DurationInSeconds(200, 1000), 0.8f},
                }
            },
            {"C1",
                {
                    {starts[0] + isx::DurationInSeconds(50, 1000), 2.5f},
                }
            },
            {"C2",
                {
                    {starts[0] + isx::DurationInSeconds(150, 1000), 2.9f},
                    {starts[0] + isx::DurationInSeconds(200, 1000), 1.7f},
                }
            },
        },
        {
            {"C0",
                {
                    {starts[1] + isx::DurationInSeconds(50, 1000), 2.2f},
                }
            },
            {"C1",
                {
                    {starts[1] + isx::DurationInSeconds(50, 1000), 1.5f},
                    {starts[1] + isx::DurationInSeconds(100, 1000), 3.5f},
                }
            },
            {"C2",
                {
                    {starts[1] + isx::DurationInSeconds(0, 1000), 1.4f},
                    {starts[1] + isx::DurationInSeconds(100, 1000), 1.3f},
                }
            },
        },
    }};

    const std::vector<isx::SpEvents_t> events =
    {
        writeNamedPacketsAsEvents(inputFilePaths[0], tis[0], eventPackets[0]),
        writeNamedPacketsAsEvents(inputFilePaths[1], tis[1], eventPackets[1]),
    };

    isx::EventsExporterParams params({events}, outputFilePath, isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);

    std::vector<std::string> expLines;

    SECTION("using first time")
    {
        params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::FIRST_DATA_ITEM;

        expLines =
        {
            "Time (s), Cell Name, Value",
            "0, C0, 1",
            "0.05, C0, 3",
            "0.2, C0, 0.8",
            "5.05, C0, 2.2",
            "0.05, C1, 2.5",
            "5.05, C1, 1.5",
            "5.1, C1, 3.5",
            "0.15, C2, 2.9",
            "0.2, C2, 1.7",
            "5, C2, 1.4",
            "5.1, C2, 1.3",
        };

    }

    SECTION("using unix time")
    {
        params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::UNIX_EPOCH;

        expLines =
        {
            "Time (s), C0, C1, C2",
            "1501062683, 1, 0, 0",
            "1501062683.05, 3, 2.5, 0",
            "1501062683.1, 0, 0, 0",
            "1501062683.15, 0, 0, 2.9",
            "1501062683.2, 0.8, 0, 1.7",
            "1501062688, 0, 0, 1.4",
            "1501062688.05, 2.2, 1.5, 0",
            "1501062688.1, 0, 3.5, 1.3",
            "1501062688.15, 0, 0, 0",
        };

        expLines =
        {
            "Time (s), Cell Name, Value",
            "1501062683, C0, 1",
            "1501062683.05, C0, 3",
            "1501062683.2, C0, 0.8",
            "1501062688.05, C0, 2.2",
            "1501062683.05, C1, 2.5",
            "1501062688.05, C1, 1.5",
            "1501062688.1, C1, 3.5",
            "1501062683.15, C2, 2.9",
            "1501062683.2, C2, 1.7",
            "1501062688, C2, 1.4",
            "1501062688.1, C2, 1.3",
        };
    }

    isx::runEventsExporter(params);

    requireEqualLines(outputFilePath, expLines);

    isx::removeDirectory(inputDir);

    isx::CoreShutdown();
}
