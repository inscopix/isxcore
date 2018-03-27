#include "isxCsvTraceImporter.h"
#include "isxTest.h"
#include "catch.hpp"
#include "isxPathUtils.h"
#include "isxGpio.h"
#include "isxTime.h"
#include "isxCore.h"

#include <vector>
#include <cstring>

namespace
{

/// Require that a logical trace has the expected number of samples
/// and that some of those samples have the expected values.
void
requireEqualValues(
        const isx::SpLogicalTrace_t & inTrace,
        const size_t inExpSize,
        const std::map<isx::Time, float> & inSomeExpValues)
{
    REQUIRE(inTrace != nullptr);
    const std::map<isx::Time, float> & values = inTrace->getValues();
    REQUIRE(values.size() == inExpSize);

    for (const auto expValue : inSomeExpValues)
    {
        const float value = values.at(expValue.first);
        if (std::isnan(expValue.second))
        {
            REQUIRE(std::isnan(value));
        }
        else
        {
            REQUIRE(value == expValue.second);
        }
    }
}

/// Require that the same channels are in each vector, though not
/// necessarily in the same order.
void
requireEqualChannels(
        const std::vector<std::string> & inActual,
        const std::vector<std::string> & inExpected)
{
    std::vector<std::string> actual(inActual);
    std::sort(actual.begin(), actual.end());
    std::vector<std::string> expected(inExpected);
    std::sort(expected.begin(), expected.end());
    REQUIRE(actual == expected);
}

} // namespace

namespace std
{

ostream &
operator<<(ostream & inStream, const set<size_t> & inSet)
{
    inStream << "{";
    for (const auto & element : inSet)
    {
        inStream << element << ", ";
    }
    inStream << "}";
    return inStream;
}

}

TEST_CASE("importCsvTraces-Bonsai", "[core][importCsvTraces]")
{
    const std::string inputDataDir = g_resources["unitTestDataPath"] + "/CSV_traces/Bonsai";
    const std::string outputDataDir = inputDataDir + "/output";
    isx::makeDirectory(outputDataDir);

    isx::CoreInitialize();

    const std::string dataName = "272-05Min-pasted-titled";
    const size_t numTimes = 7500;

    isx::CsvTraceImporterParams params;
    params.m_inputFile = inputDataDir + "/" + dataName + ".csv";
    params.m_outputFile = outputDataDir + "/" + dataName + ".isxd";
    params.m_startTime = isx::Time(2018, 1, 18, 15, 23, 4);
    params.m_timeUnit = isx::DurationInSeconds(1, 1000);

    const std::map<isx::Time, float> expX =
    {
        {params.m_startTime + isx::DurationInSeconds(125707789, 1000000), 25.55296f},
        {params.m_startTime + isx::DurationInSeconds(129547443, 1000000), 34.12868f},
        {params.m_startTime + isx::DurationInSeconds(425699507, 1000000), 615.0051f},
    };

    const std::map<isx::Time, float> expY =
    {
        {params.m_startTime + isx::DurationInSeconds(125707789, 1000000), 14.29496f},
        {params.m_startTime + isx::DurationInSeconds(129547443, 1000000), 21.1843f},
        {params.m_startTime + isx::DurationInSeconds(425699507, 1000000), 16.23495f},
    };

    const isx::TimingInfo expTi(
            params.m_startTime,
            isx::DurationInSeconds(40004, 1000000),
            numTimes
    );

    SECTION("All channels")
    {
        const std::vector<std::string> expChannelList = {"X", "Y"};

        SECTION("DOS line endings")
        {
            REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

            const isx::SpGpio_t traces = isx::readGpio(params.m_outputFile);
            REQUIRE(traces->getChannelList() == expChannelList);
            REQUIRE(traces->isAnalog("X"));
            REQUIRE(traces->isAnalog("Y"));
            REQUIRE(traces->numberOfChannels() == 2);

            requireEqualChannels(traces->getChannelList(), expChannelList);
            requireEqualValues(traces->getLogicalData("X"), numTimes, expX);
            requireEqualValues(traces->getLogicalData("Y"), numTimes, expY);
        }

        SECTION("Unix line endings")
        {
            params.m_inputFile += ".unix";

            REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

            const isx::SpGpio_t traces = isx::readGpio(params.m_outputFile);
            REQUIRE(traces->getChannelList() == expChannelList);
            REQUIRE(traces->isAnalog("X"));
            REQUIRE(traces->isAnalog("Y"));
            REQUIRE(traces->numberOfChannels() == 2);

            requireEqualChannels(traces->getChannelList(), expChannelList);
            requireEqualValues(traces->getLogicalData("X"), numTimes, expX);
            requireEqualValues(traces->getLogicalData("Y"), numTimes, expY);
        }
    }

    SECTION("One channel")
    {
        params.m_colsToImport = {1};
        const std::vector<std::string> expChannelList = {"X"};

        SECTION("DOS line endings")
        {
            REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

            const isx::SpGpio_t traces = isx::readGpio(params.m_outputFile);
            REQUIRE(traces->getChannelList() == expChannelList);
            REQUIRE(traces->isAnalog("X"));
            REQUIRE(traces->numberOfChannels() == 1);

            requireEqualChannels(traces->getChannelList(), expChannelList);
            requireEqualValues(traces->getLogicalData("X"), numTimes, expX);
        }

        SECTION("Unix line endings")
        {
            params.m_inputFile += ".unix";

            REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

            const isx::SpGpio_t traces = isx::readGpio(params.m_outputFile);
            REQUIRE(traces->getChannelList() == expChannelList);
            REQUIRE(traces->isAnalog("X"));
            REQUIRE(traces->numberOfChannels() == 1);

            requireEqualChannels(traces->getChannelList(), expChannelList);
            requireEqualValues(traces->getLogicalData("X"), numTimes, expX);
        }
    }

    isx::CoreShutdown();
    isx::removeDirectory(outputDataDir);
}

TEST_CASE("importCsvTrace-Anymaze", "[core][importCsvTraces]")
{
    const std::string inputDataDir = g_resources["unitTestDataPath"] + "/CSV_traces/Anymaze";
    const std::string outputDataDir = inputDataDir + "/output";
    isx::makeDirectory(outputDataDir);

    isx::CoreInitialize();

    const std::string dataName = "Lingjie_OFA";
    const size_t numTimes = 13710;

    isx::CsvTraceImporterParams params;
    params.m_inputFile = inputDataDir + "/" + dataName + ".csv";
    params.m_outputFile = outputDataDir + "/" + dataName + ".isxd";
    params.m_startTime = isx::Time(2018, 1, 19, 11, 10, 25);

    const std::map<isx::Time, float> expX =
    {
        {params.m_startTime, std::numeric_limits<float>::quiet_NaN()},
        {params.m_startTime + isx::DurationInSeconds(2690000, 1000000), 379.f},
        {params.m_startTime + isx::DurationInSeconds(1199980000, 1000000), 163.f},
    };

    const std::map<isx::Time, float> expY =
    {
        {params.m_startTime, std::numeric_limits<float>::quiet_NaN()},
        {params.m_startTime + isx::DurationInSeconds(2690000, 1000000), 189.f},
        {params.m_startTime + isx::DurationInSeconds(1199980000, 1000000), 354.f},
    };

    const std::map<isx::Time, float> expSpeed =
    {
        {params.m_startTime, std::numeric_limits<float>::quiet_NaN()},
        {params.m_startTime + isx::DurationInSeconds(2690000, 1000000), 0.123f},
        {params.m_startTime + isx::DurationInSeconds(1199980000, 1000000), 0.017f},
    };

    const std::map<isx::Time, float> expOfa =
    {
        {params.m_startTime, 1.f},
        {params.m_startTime + isx::DurationInSeconds(2690000, 1000000), 1.f},
        {params.m_startTime + isx::DurationInSeconds(1199980000, 1000000), 1.f},
    };

    const std::map<isx::Time, float> expCenter =
    {
        {params.m_startTime, 0.f},
        {params.m_startTime + isx::DurationInSeconds(2690000, 1000000), 1.f},
        {params.m_startTime + isx::DurationInSeconds(1199980000, 1000000), 0.f},
    };

    const isx::TimingInfo expTi(
            params.m_startTime,
            isx::DurationInSeconds(87532, 1000000),
            numTimes
    );

    SECTION("All channels")
    {
        const std::vector<std::string> expChannelList =
        {
            "Centre posn X",
            "Centre posn Y",
            "Speed",
            "In OFA",
            "In center",
        };

        SECTION("DOS line endings")
        {
            REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

            isx::SpGpio_t traces = isx::readGpio(params.m_outputFile);
            requireEqualChannels(traces->getChannelList(), expChannelList);
            REQUIRE(traces->isAnalog("Centre posn X"));
            REQUIRE(traces->isAnalog("Centre posn Y"));
            REQUIRE(traces->isAnalog("Speed"));
            REQUIRE(!traces->isAnalog("In OFA"));
            REQUIRE(!traces->isAnalog("In center"));
            REQUIRE(traces->numberOfChannels() == 5);


            requireEqualValues(traces->getLogicalData("Centre posn X"), numTimes, expX);
            requireEqualValues(traces->getLogicalData("Centre posn Y"), numTimes, expY);
            requireEqualValues(traces->getLogicalData("Speed"), numTimes, expSpeed);
            requireEqualValues(traces->getLogicalData("In OFA"), numTimes, expOfa);
            requireEqualValues(traces->getLogicalData("In center"), numTimes, expCenter);
        }

        SECTION("Unix line endings")
        {
            params.m_inputFile += ".unix";

            REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

            isx::SpGpio_t traces = isx::readGpio(params.m_outputFile);
            requireEqualChannels(traces->getChannelList(), expChannelList);
            REQUIRE(traces->isAnalog("Centre posn X"));
            REQUIRE(traces->isAnalog("Centre posn Y"));
            REQUIRE(traces->isAnalog("Speed"));
            REQUIRE(!traces->isAnalog("In OFA"));
            REQUIRE(!traces->isAnalog("In center"));
            REQUIRE(traces->numberOfChannels() == 5);


            requireEqualValues(traces->getLogicalData("Centre posn X"), numTimes, expX);
            requireEqualValues(traces->getLogicalData("Centre posn Y"), numTimes, expY);
            requireEqualValues(traces->getLogicalData("Speed"), numTimes, expSpeed);
            requireEqualValues(traces->getLogicalData("In OFA"), numTimes, expOfa);
            requireEqualValues(traces->getLogicalData("In center"), numTimes, expCenter);
        }
    }

    SECTION("Some channels")
    {
        params.m_colsToImport = {1, 3, 4};
        const std::vector<std::string> expChannelList =
        {
            "Centre posn X",
            "Speed",
            "In OFA",
        };

        SECTION("DOS line endings")
        {
            REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

            isx::SpGpio_t traces = isx::readGpio(params.m_outputFile);
            requireEqualChannels(traces->getChannelList(), expChannelList);
            REQUIRE(traces->isAnalog("Centre posn X"));
            REQUIRE(traces->isAnalog("Speed"));
            REQUIRE(!traces->isAnalog("In OFA"));

            REQUIRE(traces->numberOfChannels() == 3);

            requireEqualValues(traces->getLogicalData("Centre posn X"), numTimes, expX);
            requireEqualValues(traces->getLogicalData("Speed"), numTimes, expSpeed);
            requireEqualValues(traces->getLogicalData("In OFA"), numTimes, expOfa);
        }

        SECTION("Unix line endings")
        {
            params.m_inputFile += ".unix";

            REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

            isx::SpGpio_t traces = isx::readGpio(params.m_outputFile);
            requireEqualChannels(traces->getChannelList(), expChannelList);
            REQUIRE(traces->isAnalog("Centre posn X"));
            REQUIRE(traces->isAnalog("Speed"));
            REQUIRE(!traces->isAnalog("In OFA"));
            REQUIRE(traces->numberOfChannels() == 3);

            requireEqualValues(traces->getLogicalData("Centre posn X"), numTimes, expX);
            requireEqualValues(traces->getLogicalData("Speed"), numTimes, expSpeed);
            requireEqualValues(traces->getLogicalData("In OFA"), numTimes, expOfa);
        }
    }

    isx::CoreShutdown();
    isx::removeDirectory(outputDataDir);
}

TEST_CASE("importCsvTrace-Ethovision", "[core][importCsvTraces]")
{
    const std::string inputDataDir = g_resources["unitTestDataPath"] + "/CSV_traces/Ethovision";
    const std::string outputDataDir = inputDataDir + "/output";
    isx::makeDirectory(outputDataDir);

    isx::CoreInitialize();

    const std::string dataName = "Raw data-P38-Trial    10-track";
    const size_t numTimes = 15001;

    isx::CsvTraceImporterParams params;
    params.m_inputFile = inputDataDir + "/" + dataName + ".csv";
    params.m_outputFile = outputDataDir + "/" + dataName + ".isxd";
    params.m_startRow = 33;
    params.m_titleRow = 31;
    params.m_startTime = isx::Time(2018, 1, 19, 13, 50, 22);

    const std::map<isx::Time, float> expRecTime =
    {
        {params.m_startTime, 0.f},
        {params.m_startTime + isx::DurationInSeconds(28, 100), 0.28f},
        {params.m_startTime + isx::DurationInSeconds(600, 1), 600.f},
    };

    const std::map<isx::Time, float> expXCenter =
    {
        {params.m_startTime, 14.4354f},
        {params.m_startTime + isx::DurationInSeconds(28, 100), 14.949f},
        {params.m_startTime + isx::DurationInSeconds(600, 1), 8.75563f},
    };

    const std::map<isx::Time, float> expYNose =
    {
        {params.m_startTime, -6.4062f},
        {params.m_startTime + isx::DurationInSeconds(28, 100), -5.70016f},
        {params.m_startTime + isx::DurationInSeconds(600, 1), 14.8922f},
    };

    const std::map<isx::Time, float> expGelPad =
    {
        {params.m_startTime, 0.f},
        {params.m_startTime + isx::DurationInSeconds(28, 100), 0.f},
        {params.m_startTime + isx::DurationInSeconds(600, 1), 0.f},
    };

    // There are many more channels, but I'm too lazy to verify all of them...

    const isx::TimingInfo expTi(
            params.m_startTime,
            isx::DurationInSeconds(40000, 1000000),
            numTimes
    );

    SECTION("All columns")
    {
        const std::vector<std::string> expChannelList =
        {
            "Recording time",
            "X center",
            "Y center",
            "X nose",
            "Y nose",
            "X tail",
            "Y tail",
            "Area",
            "Areachange",
            "Elongation",
            "Direction",
            "Distance moved",
            "Velocity",
            "In zone(wood block / Center-point)",
            "In zone(gel pad / Center-point)",
            "Mobility state(Highly mobile)",
            "Mobility state(Mobile)",
            "Mobility state(Immobile)",
            "Head directed to zone",
            "Result 1",
        };

        SECTION("DOS line endings")
        {
            REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

            const isx::SpGpio_t traces = isx::readGpio(params.m_outputFile);
            requireEqualChannels(traces->getChannelList(), expChannelList);
            REQUIRE(traces->numberOfChannels() == 20);

            requireEqualValues(traces->getLogicalData("Recording time"), numTimes, expRecTime);
            requireEqualValues(traces->getLogicalData("X center"), numTimes, expXCenter);
            requireEqualValues(traces->getLogicalData("Y nose"), numTimes, expYNose);
            requireEqualValues(traces->getLogicalData("In zone(gel pad / Center-point)"), numTimes, expGelPad);
        }

        SECTION("Unix line endings")
        {
            params.m_inputFile += ".unix";

            REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

            const isx::SpGpio_t traces = isx::readGpio(params.m_outputFile);
            REQUIRE(traces->numberOfChannels() == 20);

            requireEqualChannels(traces->getChannelList(), expChannelList);
            requireEqualValues(traces->getLogicalData("Recording time"), numTimes, expRecTime);
            requireEqualValues(traces->getLogicalData("X center"), numTimes, expXCenter);
            requireEqualValues(traces->getLogicalData("Y nose"), numTimes, expYNose);
            requireEqualValues(traces->getLogicalData("In zone(gel pad / Center-point)"), numTimes, expGelPad);
        }

        SECTION("Break things by using defaults")
        {
            params.m_startRow = 2;
            params.m_titleRow = 1;

            ISX_REQUIRE_EXCEPTION(
                    isx::runCsvTraceImporter(params, nullptr, [](float){return false;}),
                    isx::ExceptionDataIO,
                    ""
            );
        }
    }

    SECTION("Some columns")
    {
        params.m_colsToImport = {2, 5, 15};
        const std::vector<std::string> expChannelList =
        {
            "X center",
            "Y nose",
            "In zone(gel pad / Center-point)",
        };

        SECTION("DOS line endings")
        {
            REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

            const isx::SpGpio_t traces = isx::readGpio(params.m_outputFile);
            requireEqualChannels(traces->getChannelList(), expChannelList);
            REQUIRE(traces->isAnalog("X center"));
            REQUIRE(traces->isAnalog("Y nose"));
            REQUIRE(!traces->isAnalog("In zone(gel pad / Center-point)"));
            REQUIRE(traces->numberOfChannels() == 3);


            requireEqualValues(traces->getLogicalData("X center"), numTimes, expXCenter);
            requireEqualValues(traces->getLogicalData("Y nose"), numTimes, expYNose);
            requireEqualValues(traces->getLogicalData("In zone(gel pad / Center-point)"), numTimes, expGelPad);
        }

        SECTION("Unix line endings")
        {
            params.m_inputFile += ".unix";

            REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

            const isx::SpGpio_t traces = isx::readGpio(params.m_outputFile);
            requireEqualChannels(traces->getChannelList(), expChannelList);
            REQUIRE(traces->isAnalog("X center"));
            REQUIRE(traces->isAnalog("Y nose"));
            REQUIRE(!traces->isAnalog("In zone(gel pad / Center-point)"));
            REQUIRE(traces->numberOfChannels() == 3);

            requireEqualValues(traces->getLogicalData("X center"), numTimes, expXCenter);
            requireEqualValues(traces->getLogicalData("Y nose"), numTimes, expYNose);
            requireEqualValues(traces->getLogicalData("In zone(gel pad / Center-point)"), numTimes, expGelPad);
        }
    }

    isx::CoreShutdown();
    isx::removeDirectory(outputDataDir);
}

TEST_CASE("convertExcelIndexToIndex", "[core][importCsvTraces]")
{
    SECTION("One letter")
    {
        REQUIRE(isx::convertExcelIndexToIndex("A") == 0);
        REQUIRE(isx::convertExcelIndexToIndex("F") == 5);
    }

    SECTION("Two letters")
    {
        REQUIRE(isx::convertExcelIndexToIndex("AA") == 26);
        REQUIRE(isx::convertExcelIndexToIndex("BF") == 2*26 + 5);
    }

    SECTION("Three letters")
    {
        REQUIRE(isx::convertExcelIndexToIndex("ABC") == std::pow(26, 2) + 2*26 + 2);
    }

    SECTION("Four letters")
    {
        REQUIRE(isx::convertExcelIndexToIndex("DUDE") == size_t(4*std::pow(26, 3) + 21*std::pow(26, 2) + 4*26 + 4));
    }

    SECTION("One digit")
    {
        REQUIRE(isx::convertExcelIndexToIndex("1") == 0);
    }

    SECTION("Two digits")
    {
        REQUIRE(isx::convertExcelIndexToIndex("27") == 26);
    }

    SECTION("Three digits")
    {
        REQUIRE(isx::convertExcelIndexToIndex("419") == 418);
    }
}

TEST_CASE("convertExcelIndicesToIndices", "[core][importCsvTraces]")
{
    SECTION("Empty string")
    {
        REQUIRE(isx::convertExcelIndicesToIndices("") == std::set<size_t>());
    }

    SECTION("One alphabetical index")
    {
        REQUIRE(isx::convertExcelIndicesToIndices("A") == std::set<size_t>({0}));
    }

    SECTION("Two alphabetical indices")
    {
        REQUIRE(isx::convertExcelIndicesToIndices("A,G") == std::set<size_t>({0, 6}));
    }

    SECTION("Two alphabetical indices with spaces")
    {
        REQUIRE(isx::convertExcelIndicesToIndices(" A, G ") == std::set<size_t>({0, 6}));
    }

    SECTION("One numerical index")
    {
        REQUIRE(isx::convertExcelIndicesToIndices("1") == std::set<size_t>({0}));
    }

    SECTION("Mix of alphabetical and numerical indices")
    {
        REQUIRE(isx::convertExcelIndicesToIndices("B, 7, AE") == std::set<size_t>({1, 6, 30}));
    }

    SECTION("One range of alphabetical indices")
    {
        REQUIRE(isx::convertExcelIndicesToIndices("A-D") == std::set<size_t>({0, 1, 2, 3}));
    }

    SECTION("One range of numerical indices")
    {
        REQUIRE(isx::convertExcelIndicesToIndices("1-3") == std::set<size_t>({0, 1, 2}));
    }

    SECTION("Mix of everything")
    {
        REQUIRE(isx::convertExcelIndicesToIndices("1, C, 4-5, Z-AC") == std::set<size_t>({0, 2, 3, 4, 25, 26, 27, 28}));
    }
}
