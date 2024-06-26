#include "isxTest.h"

#include "isxMovieFactory.h"
#include "isxPathUtils.h"
#include "isxImage.h"
#include "isxException.h"
#include "isxEvents.h"
#include "isxCellSet.h"
#include "isxVesselSet.h"

#include <QFileInfo>
#include <string>
#include <map>
#include <json.hpp>

std::map<std::string, std::string> g_resources;

size_t
hashFrameAndPixelIndex(const size_t inFrameIndex, const size_t inPixelIndex, const size_t inMaxValue)
{
    return (inFrameIndex + inPixelIndex) % (inMaxValue + 1);
}

bool
isDataPath(const std::string& dataPath)
{
    QFileInfo dataDir(dataPath.c_str());
    return dataDir.exists() && dataDir.isDir();
}

bool
findDataPath(std::string& dataPath, int limit)
{
    for (int i = 0; i < limit; ++i)
    {
        if (isDataPath(dataPath))
        {
            return true;
        }
        else
        {
            dataPath = "../" + dataPath;
        }
    }
    return false;
}

void
createSeriesTestData(
        std::string & outMovie1,
        std::string & outMovie2,
        std::string & outMovie3,
        std::string & outMovie2Overlap,
        std::string & outMovie2Step2,
        std::string & outMovie2Cropped,
        std::string & outMovie2F32,
        std::string & outImage1)
{
    const isx::Time start1(2016, 11, 8, 9, 24, 55);
    const isx::Time start2Overlap(2016, 11, 8, 9, 24, 55, isx::DurationInSeconds(2, 20));
    const isx::Time start2(2016, 11, 8, 9, 34, 29);
    const isx::Time start3(2016, 11, 8, 9, 44, 29);
    const isx::DurationInSeconds step1(1, 20);
    const isx::DurationInSeconds step2(1, 15);
    const isx::TimingInfo timingInfo1(start1, step1, 5);
    const isx::TimingInfo timingInfo2(start2, step1, 6);
    const isx::TimingInfo timingInfo2Overlap(start2Overlap, step1, 9);
    const isx::TimingInfo timingInfo2Step2(start2, step2, 6);
    const isx::TimingInfo timingInfo3(start3, step1, 7);

    const isx::SpacingInfo spacingInfo(isx::SizeInPixels_t(3, 7));
    const isx::SpacingInfo spacingInfoCropped(isx::SizeInPixels_t(3, 4));

    const std::string dataPath = g_resources["unitTestDataPath"] + "/series";
    isx::makeDirectory(dataPath);

    // Movies 1, 2 and 3 form a consistent series
    outMovie1 = dataPath + "/movie1.isxd";
    std::remove(outMovie1.c_str());
    auto m(isx::writeMosaicMovie(outMovie1, timingInfo1, spacingInfo, isx::DataType::U16));
    m->closeForWriting();

    outMovie2 = dataPath + "/movie2.isxd";
    std::remove(outMovie2.c_str());
    m = isx::writeMosaicMovie(outMovie2, timingInfo2, spacingInfo, isx::DataType::U16);
    m->closeForWriting();

    outMovie3 = dataPath + "/movie3.isxd";
    std::remove(outMovie3.c_str());
    m = isx::writeMosaicMovie(outMovie3, timingInfo3, spacingInfo, isx::DataType::U16);
    m->closeForWriting();

    // The same as movie2, but starts earlier so it temporally overlaps with movie1
    outMovie2Overlap = dataPath + "/movie2Overlap.isxd";
    std::remove(outMovie2Overlap.c_str());
    m = isx::writeMosaicMovie(outMovie2Overlap, timingInfo2Overlap, spacingInfo, isx::DataType::U16);
    m->closeForWriting();

    // The same as movie2, but with a different frame rate
    outMovie2Step2 = dataPath + "/movie2Step2.isxd";
    std::remove(outMovie2Step2.c_str());
    m = isx::writeMosaicMovie(outMovie2Step2, timingInfo2Step2, spacingInfo, isx::DataType::U16);
    m->closeForWriting();

    // The same as movie2, but with different spacing info
    outMovie2Cropped = dataPath + "/movie2Cropped.isxd";
    std::remove(outMovie2Cropped.c_str());
    m = isx::writeMosaicMovie(outMovie2Cropped, timingInfo2, spacingInfoCropped, isx::DataType::U16);
    m->closeForWriting();

    // The same as movie2, but with a different pixel data type
    outMovie2F32 = dataPath + "/movie2F32.isxd";
    std::remove(outMovie2F32.c_str());
    m = isx::writeMosaicMovie(outMovie2F32, timingInfo2, spacingInfo, isx::DataType::F32);
    m->closeForWriting();

    // The first frame of movie1
    outImage1 = dataPath + "/image1.isxd";
    std::remove(outImage1.c_str());
    m = isx::writeMosaicMovie(outImage1, isx::TimingInfo(start1, step1, 1), spacingInfo, isx::DataType::U16);
    m->closeForWriting();
}

isx::SpWritableMovie_t
writeTestU16MovieGeneric(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        uint16_t * inData)
{
    isx::isize_t numFrames = inTimingInfo.getNumTimes();
    isx::isize_t numPixels = inSpacingInfo.getTotalNumPixels();

    isx::SpWritableMovie_t movie = isx::writeMosaicMovie(inFileName, inTimingInfo, inSpacingInfo, isx::DataType::U16);

    const uint16_t maxValue = std::numeric_limits<uint16_t>::max();

    isx::isize_t framesWritten = 0;
    for (isx::isize_t f = 0; f < numFrames; ++f)
    {
        if (inTimingInfo.isIndexValid(f))
        {
            isx::SpVideoFrame_t frame = movie->makeVideoFrame(f);

            uint16_t * pPixels = frame->getPixelsAsU16();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                if (inData == NULL)
                {
                    *pPixels = uint16_t(((f * numPixels) + p) % maxValue);
                }
                else
                {
                    *pPixels = uint16_t((inData[(f * numPixels) + p]) % maxValue);
                }
                pPixels++;
            }

            movie->writeFrame(frame);
            ++framesWritten;
        }
    }
    movie->closeForWriting();
    ISX_ASSERT(framesWritten == inTimingInfo.getNumValidTimes());
    return movie;
}

isx::SpWritableMovie_t
writeTestF32MovieGeneric(
    const std::string & inFileName,
    const isx::TimingInfo & inTimingInfo,
    const isx::SpacingInfo & inSpacingInfo)
{
    return writeTestF32MovieGeneric(inFileName, inTimingInfo, inSpacingInfo, (float *)nullptr);
}

template <typename T>
isx::SpWritableMovie_t
writeTestF32MovieGeneric(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        T * inData)
{
    isx::isize_t numFrames = inTimingInfo.getNumTimes();
    isx::isize_t numPixels = inSpacingInfo.getTotalNumPixels();

    isx::SpWritableMovie_t movie = isx::writeMosaicMovie(inFileName, inTimingInfo, inSpacingInfo, isx::DataType::F32);

    isx::isize_t framesWritten = 0;
    for (isx::isize_t f = 0; f < numFrames; ++f)
    {
        if (inTimingInfo.isIndexValid(f))
        {
            isx::SpVideoFrame_t frame = movie->makeVideoFrame(f);

            float * pPixels = frame->getPixelsAsF32();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                if (inData == NULL)
                {
                    *pPixels = float((f * numPixels) + p);
                }
                else
                {
                    *pPixels = float(inData[(f * numPixels) + p]);
                }
                pPixels++;
            }

            movie->writeFrame(frame);
            ++framesWritten;
        }
    }
    movie->closeForWriting();
    ISX_ASSERT(framesWritten == inTimingInfo.getNumValidTimes());
    return movie;
}

// writeTestF32MovieGeneric template instantiations
template
isx::SpWritableMovie_t
writeTestF32MovieGeneric<double>(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        double * inData);

template
isx::SpWritableMovie_t
writeTestF32MovieGeneric<float>(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        float * inData);

void
requireEqualTraces(
        isx::FTrace_t & inActual,
        isx::FTrace_t & inExpected,
        const float inTolerance)
{
    const isx::TimingInfo timingInfo = inExpected.getTimingInfo();

    REQUIRE(inActual.getTimingInfo() == timingInfo);

    const isx::isize_t numTimes = timingInfo.getNumTimes();

    float * actualValues = inActual.getValues();
    float * expectedValues = inExpected.getValues();
    for (isx::isize_t i = 0; i < numTimes; ++i)
    {
        const float act = actualValues[i];
        const float exp = expectedValues[i];
        if (!( std::abs(act - exp) <= inTolerance || (std::isnan(act) && std::isnan(exp)) ))
        {
            FAIL("Trace value " << i << " does not match: " << actualValues[i] << " != " << expectedValues[i]);
        }
    }
}

void
requireEqualTraces(
        const isx::SpFTrace_t & inActual,
        const isx::SpFTrace_t & inExpected,
        const float inTolerance)
{
    if (inExpected == nullptr)
    {
        REQUIRE(inActual == nullptr);
    }
    requireEqualTraces(*inActual, *inExpected);
}

void
requireEqualImages(
        const isx::SpImage_t & inActual,
        const isx::SpImage_t & inExpected)
{
    requireEqualImages(*inActual, *inExpected);
}

void
requireEqualImages(
        const isx::Image & inActual,
        const isx::Image & inExpected,
        const double inRelTol)
{
    const isx::DataType dataType = inExpected.getDataType();
    const isx::SpacingInfo spacingInfo = inExpected.getSpacingInfo();

    REQUIRE(inActual.getDataType() == dataType);
    REQUIRE(inActual.getSpacingInfo() == spacingInfo);

    const isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
    switch (dataType)
    {
        case isx::DataType::U16:
        {
            const uint16_t * actualPixels = inActual.getPixelsAsU16();
            const uint16_t * expectedPixels = inExpected.getPixelsAsU16();
            for (isx::isize_t i = 0; i < numPixels; ++i)
            {
                if (actualPixels[i] != expectedPixels[i])
                {
                    FAIL("Image pixel " << i << " does not match: " << actualPixels[i] << " != " << expectedPixels[i]);
                }
            }
            break;
        }
        case isx::DataType::F32:
        {
            const float * actualPixels = inActual.getPixelsAsF32();
            const float * expectedPixels = inExpected.getPixelsAsF32();
            for (isx::isize_t i = 0; i < numPixels; ++i)
            {
                if (!approxEqual(actualPixels[i], expectedPixels[i], inRelTol))
                {
                    FAIL("Image pixel " << i << " does not match: " << actualPixels[i] << " != " << expectedPixels[i]);
                }
            }
            break;
        }
        case isx::DataType::U8:
        {
            const uint8_t * actualPixels = inActual.getPixelsAsU8();
            const uint8_t * expectedPixels = inExpected.getPixelsAsU8();
            for (isx::isize_t i = 0; i < numPixels; ++i)
            {
                if (actualPixels[i] != expectedPixels[i])
                {
                    FAIL("Image pixel " << i << " does not match: " << actualPixels[i] << " != " << expectedPixels[i]);
                }
            }
            break;
        }
        default:
        {
            ISX_THROW(isx::ExceptionDataIO, "Data type not recognized: ", dataType);
        }
    }
}


void
requireZeroImage(
        const isx::Image & inImage)
{
    const isx::DataType dataType = inImage.getDataType();
    const isx::SpacingInfo spacingInfo = inImage.getSpacingInfo();

    const isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
    switch (dataType)
    {
    case isx::DataType::U16:
    {
        const uint16_t * zeroPixels = inImage.getPixelsAsU16();
        for (isx::isize_t i = 0; i < numPixels; ++i)
        {
            if (zeroPixels[i] != 0)
            {
                FAIL("Image pixel " << i << " does not match zero: " << zeroPixels[i]);
            }
        }
        break;
    }
    case isx::DataType::F32:
    {
        const float * zeroPixels = inImage.getPixelsAsF32();
        for (isx::isize_t i = 0; i < numPixels; ++i)
        {
            if (zeroPixels[i] != 0)
            {
                FAIL("Image pixel " << i << " does not match zero: " << zeroPixels[i]);
            }
        }
        break;
    }
    default:
    {
        ISX_THROW(isx::ExceptionDataIO, "Data type not recognized: ", dataType);
    }
    }
}

void
requireEqualLines(
        const std::string & inFilePath,
        const std::vector<std::string> & inExpLines)
{
    std::ifstream outputFile(inFilePath);
    std::string line;
    for (const auto & expLine : inExpLines)
    {
        getline(outputFile, line);
        REQUIRE(line == expLine);
    }
}

void
requireEqualCsvValues(
        const std::string & inFilePath,
        const std::string & inExpTitle,
        const std::vector<std::vector<double>> & inExpValues,
        const double inRelTol)
{
    std::ifstream outputFile(inFilePath);
    std::string line;

    getline(outputFile, line);
    REQUIRE(line == inExpTitle);
    for (size_t i = 0; i < inExpValues.size(); ++i)
    {
        getline(outputFile, line);
        std::vector<double> actValues;
        for (const auto & comp : isx::splitString(line, ','))
        {
            actValues.push_back(std::stod(comp));
        }
        const auto & expValues = inExpValues[i];
        REQUIRE(expValues.size() == actValues.size());
        for (size_t j = 0; j < expValues.size(); ++j)
        {
            if (!approxEqual(actValues[j], expValues[j], inRelTol))
            {
                FAIL("CSV line " << i + 1 << ", value " << j << " does not match: " << actValues[j] << " != " << expValues[j]);
            }
        }
    }
}

void
requireEqualEvents(
        const isx::SpEvents_t & inActual,
        const isx::SpEvents_t & inExpected,
        const double inRelTol)
{
    REQUIRE(inActual->numberOfCells() == inExpected->numberOfCells());
    REQUIRE(inActual->getTimingInfo() == inExpected->getTimingInfo());

    const std::vector<std::string> cellNames = inActual->getCellNamesList();
    REQUIRE(cellNames == inExpected->getCellNamesList());
    for (const auto & cell : cellNames)
    {
        const isx::SpLogicalTrace_t actualData = inActual->getLogicalData(cell);
        const isx::SpLogicalTrace_t expectedData = inExpected->getLogicalData(cell);
        const std::map<isx::Time, float> actualValues = actualData->getValues();
        const std::map<isx::Time, float> expectedValues = expectedData->getValues();
        if (actualValues.size() != expectedValues.size())
        {
            FAIL("Cell: " << cell << ", actual number of time/value pairs not equal to expected: " << actualValues.size() << " != " << expectedValues.size());
        }
        for (const auto & value : actualValues)
        {
            const auto it = expectedValues.find(value.first);
            if (it == expectedValues.end())
            {
                FAIL("Cell: " << cell << ", could not find time stamp " << value.first << " in expected events.");
            }
            if (!approxEqual(value.second, it->second, inRelTol))
            {
                FAIL("Cell: " << cell << ", time: " << value.first << ", value: " << value.second << " != " << it->second << " using a relative tolerance of " << inRelTol);
            }
        }
    }
}

void requireEqualCellSets(
    const isx::SpCellSet_t & inActual,
    const isx::SpCellSet_t & inExpected)
{
    REQUIRE(inActual->getNumCells() == inExpected->getNumCells());
    REQUIRE(inActual->getSpacingInfo() == inExpected->getSpacingInfo());
    REQUIRE(inActual->getTimingInfo() == inExpected->getTimingInfo());

    for (isx::isize_t cellId = 0; cellId < inActual->getNumCells(); cellId++)
    {
        requireEqualImages(inActual->getImage(cellId), inExpected->getImage(cellId));
        requireEqualTraces(inActual->getTrace(cellId), inExpected->getTrace(cellId));
    }

    requireEqualCellStatuses(inActual, inExpected);
    REQUIRE(inActual->getEfocusValues() == inExpected->getEfocusValues());
}

void requireEqualVesselSets(
        const isx::SpVesselSet_t & inActual,
        const isx::SpVesselSet_t & inExpected)
{
    REQUIRE(inActual->getNumVessels() == inExpected->getNumVessels());
    REQUIRE(inActual->getSpacingInfo() == inExpected->getSpacingInfo());
    REQUIRE(inActual->getTimingInfo() == inExpected->getTimingInfo());

    for (isx::isize_t vesselId = 0; vesselId < inActual->getNumVessels(); vesselId++)
    {
        requireEqualImages(inActual->getImage(vesselId), inExpected->getImage(vesselId));
        requireEqualTraces(inActual->getTrace(vesselId), inExpected->getTrace(vesselId));
        requireEqualVesselLines(inActual->getLineEndpoints(vesselId), inExpected->getLineEndpoints(vesselId));
        if (inActual->getVesselSetType() == isx::VesselSetType_t::RBC_VELOCITY)
        {
            requireEqualTraces(inActual->getDirectionTrace(vesselId), inExpected->getDirectionTrace(vesselId));
            for (size_t t = 0; t < inActual->getTimingInfo().getNumTimes(); t++)
            {
                requireEqualVesselCorrelations(inActual->getCorrelations(vesselId, t), inExpected->getCorrelations(vesselId, t));
            }
        }
        else
        {
            requireEqualTraces(inActual->getCenterTrace(vesselId), inExpected->getCenterTrace(vesselId));
        }
    }

    requireEqualVesselStatuses(inActual, inExpected);
    REQUIRE(inActual->getEfocusValues() == inExpected->getEfocusValues());
}

void requireEqualCellStatuses(
    const isx::SpCellSet_t & inActual,
    const isx::SpCellSet_t & inExpected)
{
    REQUIRE(inActual->getNumCells() == inExpected->getNumCells());
    for (isx::isize_t cellId = 0; cellId < inExpected->getNumCells(); cellId++)
    {
        if (inActual->getCellStatus(cellId) != inExpected->getCellStatus(cellId))
        {
            FAIL("Cell status mismatch for cell " << cellId);
        }
    }
}

void requireEqualVesselStatuses(
        const isx::SpVesselSet_t & inActual,
        const isx::SpVesselSet_t & inExpected)
{
    REQUIRE(inActual->getNumVessels() == inExpected->getNumVessels());
    for (isx::isize_t vesselId = 0; vesselId < inExpected->getNumVessels(); vesselId++)
    {
        if (inActual->getVesselStatus(vesselId) != inExpected->getVesselStatus(vesselId))
        {
            FAIL("Vessel status mismatch for cell " << vesselId);
        }
    }
}

void requireEqualVesselLines(
        const isx::SpVesselLine_t & inActual,
        const isx::SpVesselLine_t & inExpected)
{
    REQUIRE(inActual->m_contour.size() == inExpected->m_contour.size());
    for (size_t i = 0; i < inActual->m_contour.size(); i++)
    {
        REQUIRE(inActual->m_contour[i] == inExpected->m_contour[i]);
    }
}

void requireEqualVesselCorrelations(
    const isx::SpVesselCorrelations_t & inActual,
    const isx::SpVesselCorrelations_t & inExpected)
{
    if (inActual == nullptr || inExpected == nullptr)
    {
        REQUIRE(inActual == nullptr);
        REQUIRE(inExpected == nullptr);
        return;
    }
    
    for (int offset = -1; offset <= 1; offset++)
    {
        requireEqualImages(inActual->getHeatmap(offset), inExpected->getHeatmap(offset));
    }
}

void requireEqualSpatialOverlapMatrices(
    const std::string actualOutputFilename,
    const std::string expectedOutputFilename,
    const double relTol)
{
    std::vector<std::string> actualCellNames1, actualCellNames2, expectedCellNames1, expectedCellNames2;
    std::vector<std::vector<double>> actualScores, expectedScores;
    readCsvAs2DArray(actualOutputFilename, actualCellNames1, actualCellNames2, actualScores);
    readCsvAs2DArray(expectedOutputFilename, expectedCellNames1, expectedCellNames2, expectedScores);

    const isx::isize_t numCols = expectedCellNames1.size();
    const isx::isize_t numRows = expectedCellNames2.size();
    REQUIRE(actualCellNames1.size() == numCols);
    REQUIRE(actualCellNames2.size() == numRows);

    for (isx::isize_t i = 0; i < numCols; i++)
    {
        if (actualCellNames1[i] != expectedCellNames1[i])
        {
            FAIL("Cell name " << i << " in the first cell set does not match: " << actualCellNames1[i] << " != " << expectedCellNames1[i]);
        }
    }

    for (isx::isize_t i = 0; i < numRows; i++)
    {
        if (actualCellNames2[i] != expectedCellNames2[i])
        {
            FAIL("Cell name " << i << " in the second cell set does not match: " << actualCellNames2[i] << " != " << expectedCellNames2[i]);
        }
    }

    for (isx::isize_t row = 0; row < numRows; row++)
    {
        for (isx::isize_t col = 0; col < numCols; col++)
        {
            if (!approxEqual(actualScores[row][col], expectedScores[row][col], relTol))
            {
                FAIL("Pairwise score between " << actualCellNames1[col] << " and " << actualCellNames2[row] << " does not match: " << actualScores[row][col] << " != " << expectedScores[row][col]);
            }
        }
    }
}

void requireEqualRegistrationMatrices(
    const std::string actualOutputFilename,
    const std::string expectedOutputFilename)
{
    std::vector<std::string> actualRows, actualCols, expectedRows, expectedCols;
    std::vector<std::vector<std::string>> actualData, expectedData;
    readCsvAs2DArray(actualOutputFilename, actualCols, actualRows, actualData);
    readCsvAs2DArray(expectedOutputFilename, expectedCols, expectedRows, expectedData);

    REQUIRE(actualCols.size() == expectedCols.size());
    REQUIRE(actualRows.size() == expectedRows.size());
    for (isx::isize_t i = 0; i < actualCols.size(); i++)
    {
        if (actualCols[i] != expectedCols[i])
        {
            FAIL("Header name " << i << " in the first cell set does not match: " << actualCols[i] << " != " << expectedCols[i]);
        }
    }

    for (isx::isize_t i = 0; i < actualRows.size(); i++)
    {
        if (actualRows[i] != actualRows[i])
        {
            FAIL("Cell name " << i << " in the second cell set does not match: " << actualRows[i] << " != " << expectedRows[i]);
        }
    }

    REQUIRE(actualData.size() == expectedData.size());
    for (isx::isize_t row = 0; row < actualData.size(); row++)
    {
        REQUIRE(actualData[row].size() == expectedData[row].size());
        for (isx::isize_t col = 0; col < actualData[row].size(); col++)
        {
            if (actualData[row][col] != expectedData[row][col])
            {
                FAIL("Match data " << actualCols[col] << " at row " << actualRows[row] << " does not match: " << actualData[row][col] << " != " << expectedData[row][col]);
            }
        }
    }
}

bool
approxEqual(
        const double inActual,
        const double inExpected,
        const double inRelTol)
{
    if (std::isnan(inActual))
    {
        return std::isnan(inExpected);
    }
    if (inRelTol > 0)
    {
        return inActual == Approx(inExpected).epsilon(inRelTol);
    }
    return inActual == inExpected;
}

void
makeCleanDirectory(const std::string & inPath)
{
    isx::removeDirectory(inPath);
    isx::makeDirectory(inPath);
}

std::vector<std::string>
getLinesFromFile(const std::string & inFilePath)
{
    std::ifstream file(inFilePath);
    std::vector<std::string> lines;
    std::string line;
    while (isx::getLine(file, line))
    {
        lines.push_back(line);
    }
    return lines;
}

void
writeSyntheticMovie(
    const std::string & inFileName,
    const isx::TimingInfo & inTimingInfo,
    const isx::SpacingInfo & inSpacingInfo,
    const bool inUseConstantPixelPerFrame,
    const isx::DataType inDataType)
{
    isx::SpWritableMovie_t movie = isx::writeMosaicMovie(inFileName, inTimingInfo, inSpacingInfo, inDataType);
    const isx::isize_t numFrames = inTimingInfo.getNumTimes();
    const isx::isize_t numPixels = inSpacingInfo.getTotalNumPixels();
    for (isx::isize_t f = 0; f < numFrames; ++f)
    {
        if (!inTimingInfo.isIndexValid(f))
        {
            continue;
        }
        isx::SpVideoFrame_t frame = movie->makeVideoFrame(f);

        if (inDataType == isx::DataType::F32)
        {
            float * pixels = frame->getPixelsAsF32();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                if (inUseConstantPixelPerFrame)
                {
                    pixels[p] = float(f);
                }
                else
                {
                    pixels[p] = float(f + p);
                }
            }
        }
        else if (inDataType == isx::DataType::U16)
        {
            uint16_t * pixels = frame->getPixelsAsU16();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                if (inUseConstantPixelPerFrame)
                {
                    pixels[p] = uint16_t (f);
                }
                else
                {
                    pixels[p] = uint16_t(f + p);
                }
            }
        }

        movie->writeFrame(frame);
    }
    movie->closeForWriting();
}

void requireEqualMovies(
    const std::string inActual,
    const std::string inExpected,
    const double inRelTol)
{
    isx::SpMovie_t expectedMovie = isx::readMovie(inExpected);
    isx::SpMovie_t actualMovie = isx::readMovie(inActual);

    REQUIRE(expectedMovie->getTimingInfo() == actualMovie->getTimingInfo());
    REQUIRE(expectedMovie->getSpacingInfo() == actualMovie->getSpacingInfo());

    const isx::isize_t totalPixels = expectedMovie->getSpacingInfo().getTotalNumPixels();
    for (isx::isize_t f = 0; f < expectedMovie->getTimingInfo().getNumValidTimes(); ++f)
    {
        isx::SpVideoFrame_t expectedFrame = expectedMovie->getFrame(f);
        isx::SpVideoFrame_t actualFrame = actualMovie->getFrame(f);

        float * expectedPixels = expectedFrame->getPixelsAsF32();
        float * actualPixels = actualFrame->getPixelsAsF32();

        for (isx::isize_t p = 0; p < totalPixels; ++p)
        {
            REQUIRE(approxEqual(expectedPixels[p], actualPixels[p], inRelTol));
        }
    }
}
