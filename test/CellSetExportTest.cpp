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

#include <tiffio.h>

TEST_CASE("CellSetExportTest", "[core]")
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
        }
        
        // export and then verify
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);
        isx::CellSetExporterParams params(
            std::vector<isx::SpCellSet_t>{cellSet},
            exportedTraceFileName,
            std::string(),
            isx::CellSetExporterParams::WriteTimeRelativeTo::FIRST_DATA_ITEM);
        isx::runCellSetExporter(params, [](float){return false;});
        
        const std::string expected =
            "Time(s), Kunal, Mark, Abbas, C3, C4\n"
            "0.025, 0, 42, 84, 126, 168\n"
            "0.075, 0.01, 42.01, 84.01, 126.01, 168.01\n"
            "0.125, 0.02, 42.02, 84.02, 126.02, 168.02\n"
            "0.175, 0.03, 42.02999, 84.03001, 126.03, 168.03\n"
            "0.225, 0.04, 42.03999, 84.04001, 126.04, 168.04\n";
        
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
        }
        
        // export and then verify
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);
        isx::CellSetExporterParams params(
            std::vector<isx::SpCellSet_t>{cellSet},
            exportedTraceFileName,
            std::string(),
            isx::CellSetExporterParams::WriteTimeRelativeTo::FIRST_DATA_ITEM);
        isx::runCellSetExporter(params, [](float){return false;});
        
        const std::string expected =
            "Time(s), Lonely1\n"
            "0.025, 84\n"
            "0.075, 84.01\n"
            "0.125, 84.02\n"
            "0.175, 84.03001\n"
            "0.225, 84.04001\n";

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
        }
        
        // export 
        isx::SpCellSet_t cellSet = isx::readCellSet(fileName);
        isx::CellSetExporterParams params(
            std::vector<isx::SpCellSet_t>{cellSet},
            std::string(),
            exportedImageFileName,
            isx::CellSetExporterParams::WriteTimeRelativeTo::FIRST_DATA_ITEM);
        isx::runCellSetExporter(params, [](float){return false;});

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
        
    }

    isx::CoreShutdown();
    std::remove(fileName.c_str());
    std::remove(exportedTraceFileName.c_str());
}
