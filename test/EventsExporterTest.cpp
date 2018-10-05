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
    std::vector<std::string> names;
    for (const auto c : inPackets)
    {
        names.push_back(c.first);
    }

    const std::vector<isx::DurationInSeconds> steps(names.size(), inTi.getStep());
    isx::SpWritableEvents_t events = isx::writeEvents(inFilePath, names, steps);
    uint64_t idx = 0;
    for (const auto c : inPackets)
    {
        for (const auto e : c.second)
        {
            auto offset = convertTimeToUSecsSinceEpoch(e.first) - convertTimeToUSecsSinceEpoch(inTi.getStart());
            events->writeDataPkt(idx, offset, e.second);
        }
        idx++;
    }
    events->setTimingInfo(inTi);
    events->closeForWriting();
    return isx::readEvents(inFilePath);
}

} // namespace

TEST_CASE("EventsExport", "[core][event_export]")
{
    isx::CoreInitialize();

    const std::string inputDir = g_resources["unitTestDataPath"] + "/events-export";
    const std::string outputDir = inputDir + "/output";

    makeCleanDirectory(outputDir);

    const std::string inputBase = outputDir + "/events";
    std::string inputFilePath = inputBase + ".isxd";
    const std::string outputFilePath = inputBase + ".csv";

    isx::EventsExporterParams params;
    std::vector<std::string> expLines;

    SECTION("Using latest events file format")
    {
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

        params = isx::EventsExporterParams({events}, outputFilePath, isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);

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
    }

    SECTION("Using legacy events file format - V1")
    {
        inputFilePath = g_resources["unitTestDataPath"] + "/test_events-V1.isxd";
        const isx::SpEvents_t events = isx::readEvents(inputFilePath);

        params = isx::EventsExporterParams({events}, outputFilePath, isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);

        SECTION("using first time")
        {
            params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::FIRST_DATA_ITEM;

            expLines =
            {
                "Time (s), Cell Name, Value",
                "3.05, C0, 7.846115",
                "36.7, C0, 7.842131",
                "41.35, C0, 6.825201",
                "48.65, C0, 7.83252",
                "61.45, C0, 7.828137",
                "71.34999999999999, C0, 7.8383",
                "96.15000000000001, C0, 7.840545",
                "28.45, C1, 7.091612",
                "45.1, C1, 7.062166",
                "54.95, C1, 7.112681",
                "106, C1, 7.077516",
                "113.65, C1, 7.080116",
                "186.4, C1, 6.971137",
                "210.75, C1, 7.087188",
                "225.3, C1, 7.094207",
                "241.3, C1, 7.079885",
                "285.8, C1, 7.126414",
                "287.95, C1, 4.777596",
                "293, C1, 7.05116",
                "3.7, C2, 6.634831"
            };
        }

        SECTION("using unix epoch")
        {
            params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::UNIX_EPOCH;

            expLines =
            {
                "Time (s), Cell Name, Value",
                "1504696263.05, C0, 7.846115",
                "1504696296.7, C0, 7.842131",
                "1504696301.35, C0, 6.825201",
                "1504696308.65, C0, 7.83252",
                "1504696321.45, C0, 7.828137",
                "1504696331.35, C0, 7.8383",
                "1504696356.15, C0, 7.840545",
                "1504696288.45, C1, 7.091612",
                "1504696305.1, C1, 7.062166",
                "1504696314.95, C1, 7.112681",
                "1504696366, C1, 7.077516",
                "1504696373.65, C1, 7.080116",
                "1504696446.4, C1, 6.971137",
                "1504696470.75, C1, 7.087188",
                "1504696485.3, C1, 7.094207",
                "1504696501.3, C1, 7.079885",
                "1504696545.8, C1, 7.126414",
                "1504696547.95, C1, 4.777596",
                "1504696553, C1, 7.05116",
                "1504696263.7, C2, 6.634831"
            };
        }
    }

    isx::runEventsExporter(params);

    requireEqualLines(outputFilePath, expLines);

    isx::removeDirectory(outputDir);

    isx::CoreShutdown();
}

TEST_CASE("EventsExport-series", "[core][event_export]")
{
    isx::CoreInitialize();

    const std::string inputDir = g_resources["unitTestDataPath"] + "/events-export";
    const std::string outputDir = inputDir + "/output";
    makeCleanDirectory(outputDir);

    const std::string inputBase = outputDir + "/events";
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

    isx::removeDirectory(outputDir);

    isx::CoreShutdown();
}

TEST_CASE("EventsExport-properties-longitudinal", "[core][event_export]")
{
    isx::CoreInitialize();

    const std::string inputDir = g_resources["unitTestDataPath"] + "/events-export";
    const std::string outputDir = inputDir + "/output";
    makeCleanDirectory(outputDir);

    std::vector<isx::SpEvents_t> events;
    for (size_t i = 1; i <= 3; ++i)
    {
        events.push_back(isx::readEvents(inputDir + "/50fr10_l" + std::to_string(i) + "-3cells_he-ROI-LCR-ED.isxd"));
    }

    const std::string outputFilePath = outputDir + "/events.csv";
    const std::string outputPropsFile = outputDir + "/props.csv";

    const isx::EventsExporterParams params(events, outputFilePath, isx::WriteTimeRelativeTo::FIRST_DATA_ITEM, outputPropsFile);
    REQUIRE(isx::runEventsExporter(params) == isx::AsyncTaskStatus::COMPLETE);

    const std::vector<std::string> lines = getLinesFromFile(outputPropsFile);
    REQUIRE(lines.size() == 4);
    REQUIRE(lines[0] == "Name,EventRate(Hz),SNR");
    REQUIRE(lines[1] == "C0,0.8,2.20437");
    REQUIRE(lines[2] == "C1,1.2,2.57958");
    REQUIRE(lines[3] == "C2,0.666667,1.5594");

    isx::removeDirectory(outputDir);
    isx::CoreShutdown();
}

TEST_CASE("EventsExport-properties-no_metrics", "[core][event_export]")
{
    isx::CoreInitialize();

    const std::string inputDir = g_resources["unitTestDataPath"] + "/events-export";
    const std::string outputDir = inputDir + "/output";
    makeCleanDirectory(outputDir);

    const std::vector<isx::SpEvents_t> events = {isx::readEvents(inputDir + "/eventset_no_metrics.isxd")};

    const std::string outputFilePath = outputDir + "/events.csv";
    const std::string outputPropsFile = outputDir + "/props.csv";

    const isx::EventsExporterParams params(events, outputFilePath, isx::WriteTimeRelativeTo::FIRST_DATA_ITEM, outputPropsFile);
    REQUIRE(isx::runEventsExporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

    const std::vector<std::string> lines = getLinesFromFile(outputPropsFile);
    REQUIRE(lines.size() == 4);
    REQUIRE(lines[0] == "Name");
    REQUIRE(lines[1] == "C0");
    REQUIRE(lines[2] == "C1");
    REQUIRE(lines[3] == "C2");

    isx::removeDirectory(outputDir);
    isx::CoreShutdown();
}
