#include "isxTest.h"
#include "isxLogicalTrace.h"
#include "isxEvents.h"
#include "catch.hpp"
#include "isxPathUtils.h"
#include "isxEventBasedFileV2.h"
#include "isxGpio.h"
#include "isxCsvTraceImporter.h"

template <typename T>
void
requireApproxEqual(
        const std::vector<std::vector<T>> & inActual,
        const std::vector<std::vector<T>> & inExpected,
        const double inPrecision = double(std::numeric_limits<float>::epsilon() * 100))
{
    REQUIRE((inActual.size() == inExpected.size()));
    for (size_t i = 0; i < inActual.size(); ++i)
    {
        REQUIRE(inActual[i].size() == inExpected[i].size());
        for (size_t j = 0; j < inActual[i].size(); ++j)
        {
            REQUIRE(inActual[i][j] == Approx(inExpected[i][j]).epsilon(inPrecision));
        }
    }
}

const double g_xPrecision = std::numeric_limits<double>::epsilon() * 100;

TEST_CASE("getCoordinatesFromLogicalTrace", "[core][logicaltrace]")
{
    const std::string inputDir = g_resources["unitTestDataPath"] + "/logical_trace";
    const std::string outputDir = inputDir + "/output";
    isx::makeDirectory(outputDir);

    isx::CoreInitialize();

    SECTION("Series of 3 synthetic event sets")
    {
        const std::vector<std::string> filePaths =
        {
            inputDir + "/50fr10_l1-3cells_he-PCA-ICA-ED.isxd",
            inputDir + "/50fr10_l2-3cells_he-PCA-ICA-ED.isxd",
            inputDir + "/50fr10_l3-3cells_he-PCA-ICA-ED.isxd",
        };

        const isx::SpEvents_t series = isx::readEventsSeries(filePaths);
        const isx::TimingInfos_t tis = series->getTimingInfosForSeries();

        std::vector<std::vector<double>> xActual;
        std::vector<std::vector<double>> yActual;

        const isx::SpLogicalTrace_t trace0 = series->getLogicalData("C0");
        getCoordinatesFromLogicalTrace(trace0, tis, false, xActual, yActual);
        requireApproxEqual(xActual, std::vector<std::vector<double>>(
        {
            {0.2, 0.5, 1.5, 2.2, 3.6},
            {5.2, 5.5, 6.5, 7.2, 8.6},
            {10.2, 10.5, 11.5, 12.2, 13.6},
        }), g_xPrecision);
        requireApproxEqual(yActual, std::vector<std::vector<double>>(
        {
            {1873949, 1850383, 4889011, 1828613, 1911710},
            {1873949, 1850383, 4889011, 1828613, 1911710},
            {1873949, 1850383, 4889011, 1828613, 1911710},
        }));

        const isx::SpLogicalTrace_t trace1 = series->getLogicalData("C1");
        getCoordinatesFromLogicalTrace(trace1, tis, false, xActual, yActual);

        requireApproxEqual(xActual, std::vector<std::vector<double>>(
            {
                {2.8, 3.7},
                {7.8, 8.7},
                {12.8, 13.7},
        }), g_xPrecision);
        requireApproxEqual(yActual, std::vector<std::vector<double>>(
        {
            {5177436, 2654317},
            {5177436, 2654317},
            {5177436, 2654317},
        }));

        const isx::SpLogicalTrace_t trace2 = series->getLogicalData("C2");
        getCoordinatesFromLogicalTrace(trace2, tis, false, xActual, yActual);
        requireApproxEqual(xActual, std::vector<std::vector<double>>(
        {
            {3.3, 3.7},
            {8.3, 8.7},
            {13.3, 13.7},
        }));
        requireApproxEqual(yActual, std::vector<std::vector<double>>(
        {
            {2503654, 2633966},
            {2503654, 2633966},
            {2503654, 2633966},
        }));
    }

    SECTION("Series of two synthetic GPIO sets")
    {
        const std::vector<std::string> eventFilePaths =
        {
            outputDir + "/output_0.isxd",
            outputDir + "/output_1.isxd",
        };
        const std::vector<isx::Time> startTimes =
        {
            isx::Time(2018, 7, 26, 11, 41, 0),
            isx::Time(2018, 7, 26, 11, 41, 1),
        };
        const auto step = isx::DurationInSeconds::fromMicroseconds(1);
        const std::string channelName = "C0";

        for (size_t i = 0; i < 2; ++i)
        {
            isx::EventBasedFileV2 eventFile(eventFilePaths[i], isx::DataSet::Type::GPIO,
                    {channelName}, {step}, {isx::SignalType::SPARSE});
            eventFile.writeDataPkt(isx::EventBasedFileV2::DataPkt(1, 1, 0));
            eventFile.setTimingInfo(startTimes[i], startTimes[i] + isx::DurationInSeconds::fromMicroseconds(4));
            eventFile.closeFileForWriting();
        }

        const isx::SpGpio_t series = isx::readGpioSeries(eventFilePaths);
        const isx::TimingInfos_t tis = series->getTimingInfosForSeries();

        std::vector<std::vector<double>> x, y;
        const isx::SpLogicalTrace_t trace = series->getLogicalData(channelName);
        getCoordinatesFromLogicalTrace(trace, tis, true, x, y);
        requireApproxEqual(x, std::vector<std::vector<double>>({
                {0.000001, 0.000003},
                {0.000005, 0.000007},
        }), g_xPrecision);
        REQUIRE(y == std::vector<std::vector<double>>({
                {1, 1},
                {1, 1},
        }));
    }

    SECTION("Series of two imported CSV traces found during QA of MOS-1602")
    {
        const std::string inputDir = g_resources["unitTestDataPath"] + "/CSV_traces/Bonsai";
        const std::string fileBase = "272-05Min-pasted-titled";
        const std::string outputFileBase = outputDir + "/" + fileBase + "-";
        const std::vector<std::string> outputFiles =
        {
            outputFileBase + "0.isxd",
            outputFileBase + "1.isxd",
        };
        const std::vector<isx::Time> startTimes =
        {
            isx::Time(2018, 8, 8, 9, 45, 0),
            isx::Time(2018, 8, 8, 10, 45, 0),
        };

        isx::CsvTraceImporterParams params;
        params.m_inputFile = inputDir + "/" + fileBase + ".csv";
        params.m_outputFile = outputFiles[0];
        params.m_startTime = startTimes[0];
        params.m_timeUnit = isx::DurationInSeconds::fromMicroseconds(1);
        REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

        params.m_outputFile = outputFiles[1];
        params.m_startTime = startTimes[1];
        REQUIRE(isx::runCsvTraceImporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

        const isx::SpGpio_t gpio = isx::readGpioSeries(outputFiles);
        const isx::TimingInfos_t tis = gpio->getTimingInfosForSeries();

        std::vector<std::vector<double>> x, y;
        const std::string channelName = "X";
        const isx::SpLogicalTrace_t trace = gpio->getLogicalData(channelName);
        getCoordinatesFromLogicalTrace(trace, tis, !gpio->isAnalog(channelName), x, y);

        REQUIRE(x.size() == 2);

            const size_t numSamples = 7500;

        REQUIRE(x[0].size() == numSamples);
        REQUIRE(y[0].size() == numSamples);

        REQUIRE(x[0][0] == Approx(0.1257077888));
        REQUIRE(y[0][0] == Approx(25.55296));

        REQUIRE(x[0][7499] == Approx(0.4256995072));
        REQUIRE(y[0][7499] == Approx(615.0051));

        REQUIRE(x[1].size() == numSamples);
        REQUIRE(y[1].size() == numSamples);

        REQUIRE(x[1][0] == Approx(tis[0].getDuration().toDouble() + 0.1257077888));
        REQUIRE(y[1][0] == Approx(25.55296));
    }

    isx::CoreShutdown();
    isx::removeDirectory(outputDir);
}
