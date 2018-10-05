#include "isxCellSetFactory.h"
#include "isxCellSetSimple.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"
#include "isxMovieFactory.h"
#include "isxProject.h"
#include "isxPathUtils.h"

#include "isxCellSetExporter.h"

#include <cstring>
#include <fstream>
#include <memory>
#include <array>

#include <tiffio.h>

TEST_CASE("CellSetExportTest", "[core][cellset_export]")
{
    std::string fileName = g_resources["unitTestDataPath"] + "/cellset.isxd";
    std::remove(fileName.c_str());

    std::string exportedTraceFileName = g_resources["unitTestDataPath"] + "/exportedTrace.csv";
    std::remove(exportedTraceFileName.c_str());

    std::string exportedImageFileName = g_resources["unitTestDataPath"] + "/exportedImage.tiff";
    std::remove(exportedImageFileName.c_str());

    isx::Time start;
    isx::DurationInSeconds step(50, 1000);
    isx::isize_t numFrames = 5;
    isx::TimingInfo timingInfo(start, step, numFrames);

    isx::isize_t numRows = 4;
    isx::isize_t numCols = 3;
    isx::SizeInPixels_t numPixels(numCols, numRows);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

    isx::SpImage_t originalImage = std::make_shared<isx::Image>(
            spacingInfo,
            sizeof(float) * spacingInfo.getNumColumns(),
            1,
            isx::DataType::F32);
    float * originalPixels = originalImage->getPixelsAsF32();
    std::memset(originalPixels, 0, sizeof(float) * spacingInfo.getTotalNumPixels());

    constexpr isx::isize_t numCells = 5;
    std::vector<isx::SpFTrace_t> originalTraces(numCells);
    std::vector<float *> originalValues(numCells);
    float val = 0.0f;
    for (isx::isize_t cell = 0; cell < numCells; ++cell)
    {
        originalTraces[cell] = std::make_shared<isx::Trace<float>>(timingInfo);
        originalValues[cell] = originalTraces[cell]->getValues();

        val = float(cell) * 42.f;

        for (isx::isize_t i(0); i < timingInfo.getNumTimes(); ++i)
        {
            originalValues[cell][i] = val;
            val += 0.01f;
        }
    }

    isx::CoreInitialize();

    SECTION("Verify exported trace data")
    {
        // write sample data
        {
            isx::SpCellSet_t cellSet = isx::writeCellSet(
                fileName, timingInfo, spacingInfo);

            cellSet->writeImageAndTrace(0, originalImage, originalTraces[0], "Kunal");
            cellSet->writeImageAndTrace(1, originalImage, originalTraces[1], "Mark");
            cellSet->writeImageAndTrace(2, originalImage, originalTraces[2], "Abbas");
            cellSet->writeImageAndTrace(3, originalImage, originalTraces[3]);
            cellSet->writeImageAndTrace(4, originalImage, originalTraces[4]);
            cellSet->closeForWriting();
        }

        // export and then verify
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);
        isx::CellSetExporterParams params(
            std::vector<isx::SpCellSet_t>{cellSet},
            exportedTraceFileName,
            std::string(),
            isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);
        isx::runCellSetExporter(params, nullptr, [](float){return false;});

        const std::string expected =
            " , Kunal, Mark, Abbas, C3, C4\n"
            "Time(s)/Cell Status, undecided, undecided, undecided, undecided, undecided\n"
            "0, 0, 42, 84, 126, 168\n"
            "0.05, 0.01, 42.01, 84.01, 126.01, 168.01\n"
            "0.1, 0.02, 42.02, 84.02, 126.02, 168.02\n"
            "0.15, 0.03, 42.02999, 84.03001, 126.03, 168.03\n"
            "0.2, 0.04, 42.03999, 84.04001, 126.04, 168.04\n";

        std::ifstream strm(exportedTraceFileName);
        std::unique_ptr<char[]> buf(new char[expected.length() + 1]);   // account for null termination
        strm.get(buf.get(), expected.length() + 1, '$');                // get reads count - 1 chars
        std::string actual(buf.get());
        REQUIRE(actual == expected);
    }

    SECTION("Export CellSet with single cell to CSV")
    {
        // write sample data
        {
            isx::SpCellSet_t cellSet = isx::writeCellSet(
                fileName, timingInfo, spacingInfo);

            cellSet->writeImageAndTrace(0, originalImage, originalTraces[2], "Lonely1");
            cellSet->closeForWriting();
        }

        // export and then verify
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);
        isx::CellSetExporterParams params(
            std::vector<isx::SpCellSet_t>{cellSet},
            exportedTraceFileName,
            std::string(),
            isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);
        isx::runCellSetExporter(params, nullptr, [](float){return false;});

        const std::string expected =
            " , Lonely1\n"
            "Time(s)/Cell Status, undecided\n"
            "0, 84\n"
            "0.05, 84.01\n"
            "0.1, 84.02\n"
            "0.15, 84.03001\n"
            "0.2, 84.04001\n";

        std::ifstream strm(exportedTraceFileName);
        std::unique_ptr<char[]> buf(new char[expected.length() + 1]);   // account for null termination
        strm.get(buf.get(), expected.length() + 1, '$');                // get reads count - 1 chars
        std::string actual(buf.get());
        REQUIRE(actual == expected);
    }


    SECTION("Export CellSet with single cell to TIFF")
    {
        // write sample data
        {
            isx::SpCellSet_t cellSet = isx::writeCellSet(
                fileName, timingInfo, spacingInfo);

            // Make the original image have alternating rows of 1's and 0's
            float * pixelsF32 = originalImage->getPixelsAsF32();
            for(isx::isize_t i = 0; i < numRows; i+=2)
            {
                for(isx::isize_t j = 0; j < numCols; ++j)
                {
                    pixelsF32[numCols * i + j] = 1.0f;
                }
            }

            cellSet->writeImageAndTrace(0, originalImage, originalTraces[2], "Lonely1");
            cellSet->setCellStatus(0, isx::CellSet::CellStatus::ACCEPTED);
            cellSet->closeForWriting();
        }

        // export
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);
        isx::CellSetExporterParams params(
            std::vector<isx::SpCellSet_t>{cellSet},
            std::string(),
            exportedImageFileName,
            isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);
        isx::runCellSetExporter(params, nullptr, [](float){return false;});

        // read output TIFF and verify
        std::string cellname = cellSet->getCellName(0);
        std::string fn = isx::getDirName(exportedImageFileName) + "/" +
                isx::getBaseName(exportedImageFileName) + "_" + cellname +
                "." + isx::getExtension(exportedImageFileName);

        TIFF * tif = TIFFOpen(fn.c_str(), "r");
        if(!tif)
        {
            FAIL("Could not open the TIFF file.");
        }
        else
        {
            uint32_t width, height;
            TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
            TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

            REQUIRE(width == uint32_t(numCols));
            REQUIRE(height == uint32_t(numRows));

            float * readRow = (float *) _TIFFmalloc(width*sizeof(float));
            for (uint32_t r = 0; r < height; ++r)
            {
                if(TIFFReadScanline(tif, readRow, r, 0) < 0)
                {
                    FAIL("Failed to read TIFF image");
                }
                float expectedVal = 0.0f;
                if(r % 2 == 0)
                {
                    expectedVal = 1.0f;
                }

                for (isx::isize_t c = 0; c < numCols; ++c)
                {
                    REQUIRE(readRow[c] == expectedVal);
                }
            }

            if(readRow)
            {
                _TIFFfree(readRow);
            }

            TIFFClose(tif);
        }

        std::remove(fn.c_str());

        // read output TIFF map and verify
        fn = isx::getDirName(exportedImageFileName) + "/" +
            isx::getBaseName(exportedImageFileName) + "_accepted-cells-map." +
            isx::getExtension(exportedImageFileName);

        tif = TIFFOpen(fn.c_str(), "r");
        if (!tif)
        {
            FAIL("Could not open the TIFF file.");
        }
        else
        {
            uint32_t width, height;
            TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
            TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

            REQUIRE(width == uint32_t(numCols));
            REQUIRE(height == uint32_t(numRows));

            float * readRow = (float *)_TIFFmalloc(width * sizeof(float));
            for (uint32_t r = 0; r < height; ++r)
            {
                if (TIFFReadScanline(tif, readRow, r, 0) < 0)
                {
                    FAIL("Failed to read TIFF image");
                }
                float expectedVal = 0.0f;
                if (r % 2 == 0)
                {
                    expectedVal = 1.0f / 6.0f;
                }

                for (isx::isize_t c = 0; c < numCols; ++c)
                {
                    REQUIRE(readRow[c] == expectedVal);
                }
            }

            if (readRow)
            {
                _TIFFfree(readRow);
            }

            TIFFClose(tif);
        }

        std::remove(fn.c_str());
    }

    SECTION("Test for overflow when getting sample times")
    {
        // write sample data
        {
            isx::Time start2(2016, 1, 1, 0, 0, 0, isx::DurationInSeconds(67, 1000));
            isx::DurationInSeconds step2(100, 2000);
            isx::TimingInfo timingInfo2(start2, step2, numFrames);
            isx::SpCellSet_t cellSet = isx::writeCellSet(fileName, timingInfo2, spacingInfo);
            cellSet->writeImageAndTrace(0, originalImage, originalTraces[0], "Kunal");
            cellSet->writeImageAndTrace(1, originalImage, originalTraces[1], "Mark");
            cellSet->writeImageAndTrace(2, originalImage, originalTraces[2], "Abbas");
            cellSet->writeImageAndTrace(3, originalImage, originalTraces[3]);
            cellSet->writeImageAndTrace(4, originalImage, originalTraces[4]);
            cellSet->closeForWriting();
        }
        // export
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);
        isx::CellSetExporterParams params(
            std::vector<isx::SpCellSet_t>{cellSet},
            exportedTraceFileName,
            "",
            isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);
        isx::runCellSetExporter(params, nullptr, [](float){return false;});
        // check that sample times are as expected
        const std::string expected =
            " , Kunal, Mark, Abbas, C3, C4\n"
            "Time(s)/Cell Status, undecided, undecided, undecided, undecided, undecided\n"
            "0, 0, 42, 84, 126, 168\n"
            "0.05, 0.01, 42.01, 84.01, 126.01, 168.01\n"
            "0.1, 0.02, 42.02, 84.02, 126.02, 168.02\n"
            "0.15, 0.03, 42.02999, 84.03001, 126.03, 168.03\n"
            "0.2, 0.04, 42.03999, 84.04001, 126.04, 168.04\n";

        std::ifstream strm(exportedTraceFileName);
        std::unique_ptr<char[]> buf(new char[expected.length() + 1]);   // account for null termination
        strm.get(buf.get(), expected.length() + 1, '$');                // get reads count - 1 chars
        std::string actual(buf.get());
        REQUIRE(actual == expected);
    }

    SECTION("Test to reproduce error seen in python")
    {
        int inNumInputFiles = 1;

        std::string test_file = g_resources["unitTestDataPath"] + "/eventDetectionCellSet.isxd";
        std::string output_trace = g_resources["unitTestDataPath"] + "/output/trace_output.csv";
        std::string output_image = g_resources["unitTestDataPath"] + "/output/image_output.tiff";

        const char * inInputFileNames[] = { test_file.c_str() };
        const char * inTraceFilename = output_trace.c_str();
        const char * inImagesFilename = output_image.c_str();

        int inWriteTimeRelativeTo = 0;

        std::vector<isx::SpCellSet_t> inputCellSets;
        for (int64_t i = 0; i < int64_t(inNumInputFiles); ++i)
        {
            inputCellSets.push_back(isx::readCellSet(inInputFileNames[i]));
        }

        isx::CellSetExporterParams params(
            inputCellSets,
            std::string(inTraceFilename),
            std::string(inImagesFilename),
            isx::WriteTimeRelativeTo(inWriteTimeRelativeTo));

        auto outputParams = std::make_shared<isx::CellSetExporterOutputParams>();
        auto res = isx::runCellSetExporter(params, outputParams, [](float) {return false; });

        REQUIRE(res == isx::AsyncTaskStatus::COMPLETE);
    }

    isx::CoreShutdown();
    std::remove(fileName.c_str());
    std::remove(exportedTraceFileName.c_str());
}

TEST_CASE("CellSetExport-generatePythonTestData", "[!hide]")
{
    isx::CoreInitialize();

    const std::string inputFile = g_resources["unitTestDataPath"] + "/eventDetectionCellSet.isxd";

    const std::string version = "v2";

    SECTION("Typical settings")
    {
        const std::string traceOutputFile = g_resources["unitTestDataPath"] + "/guilded/exp_mosaicCellSetExporter_TraceOutput-" + version + ".csv";
        const std::string imageOutputFile = g_resources["unitTestDataPath"] + "/guilded/exp_mosaicCellSetExporter_ImageOutput-" + version + ".tiff";

        const isx::SpCellSet_t inputCellSet = isx::readCellSet(inputFile);
        const isx::CellSetExporterParams params({inputCellSet}, traceOutputFile, imageOutputFile, isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);
        isx::runCellSetExporter(params, nullptr, [](float){return false;});
    }

    isx::CoreShutdown();
}

TEST_CASE("CellSetExport-properties-longitudinal", "[core][cellset_export]")
{
    isx::CoreInitialize();

    const std::string inputDir = g_resources["unitTestDataPath"] + "/cellset_exporter";
    const std::string outputDir = inputDir + "/output";
    makeCleanDirectory(outputDir);

    std::vector<isx::SpCellSet_t> inputCellSets;
    for (size_t i = 1; i <= 3; ++i)
    {
        inputCellSets.push_back(isx::readCellSet(inputDir + "/50fr10_l" + std::to_string(i) + "-3cells_he-ROI-LCR.isxd"));
    }

    const std::string outputTraceFile = outputDir + "/traces.csv";
    const std::string autoPropsFile = outputDir + "/traces-props.csv";
    const std::string manualPropsFile = outputDir + "/props.csv";

    isx::CellSetExporterParams params(
            inputCellSets,
            outputTraceFile,
            "",
            isx::WriteTimeRelativeTo::FIRST_DATA_ITEM,
            true,
            "",
            false);

    std::string outputPropsFile;

    SECTION("No properties")
    {
        REQUIRE(isx::runCellSetExporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);
        REQUIRE(!isx::pathExists(autoPropsFile));
        REQUIRE(!isx::pathExists(manualPropsFile));
    }

    SECTION("Auto properties")
    {
        outputPropsFile = autoPropsFile;
        params.m_autoOutputProps = true;

        REQUIRE(isx::runCellSetExporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);
        REQUIRE(isx::pathExists(autoPropsFile));
        REQUIRE(!isx::pathExists(manualPropsFile));
    }

    SECTION("Manually specify properties (auto off)")
    {
        outputPropsFile = manualPropsFile;
        params.m_propertiesFilename = outputPropsFile;
        params.m_autoOutputProps = false;

        REQUIRE(isx::runCellSetExporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);
        REQUIRE(!isx::pathExists(autoPropsFile));
        REQUIRE(isx::pathExists(manualPropsFile));
    }

    SECTION("Manually specify properties (auto on, but ignored)")
    {
        outputPropsFile = manualPropsFile;
        params.m_propertiesFilename = outputPropsFile;
        params.m_autoOutputProps = true;

        REQUIRE(isx::runCellSetExporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);
        REQUIRE(!isx::pathExists(autoPropsFile));
        REQUIRE(isx::pathExists(manualPropsFile));
    }

    if (!outputPropsFile.empty())
    {
        const std::vector<std::string> lines = getLinesFromFile(outputPropsFile);
        REQUIRE(lines.size() == 4);

        REQUIRE(lines[0] == "Name,Status,Color(R),Color(G),Color(B),Centroid(X),Centroid(Y),NumComponents,Size,Active(0),Active(1),Active(2)");
        REQUIRE(lines[1] == "C0,accepted,244,44,82,17,30,1,36.2353,1,1,0");
        REQUIRE(lines[2] == "C1,undecided,221,168,36,147,28,1,10.0499,1,1,1");
        REQUIRE(lines[3] == "C2,rejected,0,188,165,148,129,1,10.4403,1,0,1");
    }

    isx::removeDirectory(outputDir);
    isx::CoreShutdown();
}

TEST_CASE("CellSetExport-properties-no_metrics", "[core][cellset_export]")
{
    isx::CoreInitialize();

    const std::string inputDir = g_resources["unitTestDataPath"] + "/cellset_exporter";
    const std::string outputDir = inputDir + "/output";
    makeCleanDirectory(outputDir);

    const std::vector<isx::SpCellSet_t> inputCellSets = {isx::readCellSet(inputDir + "/cellset_no_metrics.isxd")};
    const std::string outputPropsFile = outputDir + "/props.csv";

    const isx::CellSetExporterParams params(
            inputCellSets,
            "",
            "",
            isx::WriteTimeRelativeTo::FIRST_DATA_ITEM,
            true,
            outputPropsFile);

    REQUIRE(isx::runCellSetExporter(params, nullptr, [](float){return false;}) == isx::AsyncTaskStatus::COMPLETE);

    const std::vector<std::string> lines = getLinesFromFile(outputPropsFile);
    REQUIRE(lines.size() == 4);
    REQUIRE(lines[0] == "Name,Status,Color(R),Color(G),Color(B),Active(0)");
    REQUIRE(lines[1] == "C0,undecided,255,255,255,1");
    REQUIRE(lines[2] == "C1,undecided,255,255,255,1");
    REQUIRE(lines[3] == "C2,undecided,255,255,255,1");

    isx::removeDirectory(outputDir);
    isx::CoreShutdown();
}
