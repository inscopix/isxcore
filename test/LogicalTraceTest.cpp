#include "isxTest.h"
#include "isxLogicalTrace.h"
#include "isxEvents.h"
#include "catch.hpp"

template <typename T>
void
requireApproxEqual(
        const std::vector<std::vector<T>> & inActual,
        const std::vector<std::vector<T>> & inExpected)
{
    REQUIRE((inActual.size() == inExpected.size()));
    for (size_t i = 0; i < inActual.size(); ++i)
    {
        REQUIRE(inActual[i].size() == inExpected[i].size());
        for (size_t j = 0; j < inActual[i].size(); ++j)
        {
            REQUIRE(inActual[i][j] == Approx(inExpected[i][j]));
        }
    }
}

TEST_CASE("getCoordinatesFromLogicalTrace", "[core][logicaltrace]")
{
    isx::CoreInitialize();

    const std::string inputDir = g_resources["unitTestDataPath"] + "/logical_trace";

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
        }));
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
        }));
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

    isx::CoreShutdown();
}
