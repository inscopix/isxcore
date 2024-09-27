#ifndef ISX_TEST_H
#define ISX_TEST_H

#include <string>
#include <map>
#include "catch.hpp"

#include "isxWritableMovie.h"
#include "isxCoreFwd.h"
#include "isxTrace.h"
#include "isxVesselSet.h"

extern std::map<std::string, std::string> g_resources;

/// Hashes a frame and pixel index to be under a max value.
///
size_t
hashFrameAndPixelIndex(const size_t inFrameIndex, const size_t inPixelIndex, const size_t inMaxValue);

/// Checks if the given test data path exists.
///
/// \param  path    The data path to check.
/// \return         True if the given test data path exists.
bool isDataPath(const std::string& path);

/// Find the test data path by performing a limited search up the tree.
///
/// \param  path    The test data path, which is updated in place.
/// \param  limit   The maximum number of steps up the tree to search.
/// \return         True if the test path was found.
bool findDataPath(std::string& path, int limit);

/// Writes some very small test data for series tests.
///
/// Movies 1, 2 and 3 can be put into a series successfully.
/// The other files modify movie 2 so that will no longer be
/// successful for a number of reasons.
///
/// The output arguments are file paths to the movies.
/// All files should be in g_resources["unitTestDataPath"] + "/series"
///
/// \param  outMovie1           Movie 1 is consistent with movies 2 and 3.
/// \param  outMovie2           Movie 2 is consistent with movies 1 and 3.
/// \param  outMovie3           Movie 3 is consistent with movies 1 and 2.
/// \param  outMovie2Overlap    Similar to movie 2, except with a start time
///                             that makes it overlap with movie 1.
/// \param  outMovie2Step2      Similar to movie 2, except with a different frame rate.
/// \param  outMovie2Cropped    Similar to movie 2, except with different spacing info.
/// \param  outMovie2F32        Similar to movie 2, except with F32 pixel values instead of U16.
/// \param  outImage1           The first frame of movie 1.
void
createSeriesTestData(
        std::string & outMovie1,
        std::string & outMovie2,
        std::string & outMovie3,
        std::string & outMovie2Overlap,
        std::string & outMovie2Step2,
        std::string & outMovie2Cropped,
        std::string & outMovie2F32,
        std::string & outImage1);

/// Write out a test movie in U16 format
///
/// \param  inFileName     The name of the file
/// \param  inTimingInfo   Timing info
/// \param  inSpacingInfo  Spacing info
/// \param  inData         Optional parameter pointing to buffer with data
/// \return                Shared pointer to the movie object
isx::SpWritableMovie_t
writeTestU16MovieGeneric(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        uint16_t * inData = NULL);

/// Write out a test movie in F32 format
///
/// \param  inFileName     The name of the file
/// \param  inTimingInfo   Timing info
/// \param  inSpacingInfo  Spacing info
/// \return                Shared pointer to the movie object
isx::SpWritableMovie_t
writeTestF32MovieGeneric(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo);

/// Write out a test movie in F32 format
///
/// \param  inFileName     The name of the file
/// \param  inTimingInfo   Timing info
/// \param  inSpacingInfo  Spacing info
/// \param  inData         Points to buffer with data
/// \return                Shared pointer to the movie object
template<typename T>
isx::SpWritableMovie_t
writeTestF32MovieGeneric(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        T * inData);

/// Checks that two traces are equal use Catch REQUIRE statements.
///
/// This compares both data values and meta-data (e.g. timing info).
///
/// \param  inActual    The actual trace.
/// \param  inExpected  The expected trace.
/// \param  inTolerance   Optional tolerance.
void
requireEqualTraces(
        isx::FTrace_t & inActual,
        isx::FTrace_t & inExpected,
        const float inTolerance = 0);

/// Checks that two traces are equal use Catch REQUIRE statements.
///
/// This compares both data values and meta-data (e.g. timing info).
///
/// \param  inActual    The actual trace.
/// \param  inExpected  The expected trace.
/// \param  inTolerance   Optional tolerance.
void
requireEqualTraces(
        const isx::SpFTrace_t & inActual,
        const isx::SpFTrace_t & inExpected,
        const float inTolerance = 0);

/// Checks that two images are equal use Catch REQUIRE statements.
///
/// This compares both data values and meta-data (e.g. spacing info).
/// The data types must also match.
///
/// \param  inActual    The actual image.
/// \param  inExpected  The expected image.
void
requireEqualImages(
        const isx::SpImage_t & inActual,
        const isx::SpImage_t & inExpected);

/// Checks that two images are equal use Catch REQUIRE statements.
///
/// This compares both data values and meta-data (e.g. spacing info).
/// The data types must also match.
///
/// \param  inActual    The actual image.
/// \param  inExpected  The expected image.
/// \param  inRelTol    The relative tolerance used for floating point comparisons.
void
requireEqualImages(
        const isx::Image & inActual,
        const isx::Image & inExpected,
        const double inRelTol = 0);

/// Computes the sum of a video frame
///
/// \param  inFrame    The video frame to sum.
uint64_t
computeFrameSum(
    const isx::SpVideoFrame_t & inFrame
);

/// Computes the sum of an image
///
/// \param  inFrame    The image to sum.
uint64_t
computeFrameSum(
    const isx::Image & inImage
);

/// Computes the sum of a movie
///
/// \param  inFrame    The movie to sum.
uint64_t
computeMovieSum(
    const isx::SpMovie_t & inMovie
);

/// Checks that image is zero use Catch REQUIRE statements.
///
/// This compares both data values and meta-data (e.g. spacing info).
/// The data types must also match.
///
/// \param  inImage    The image.
void
requireZeroImage(
    const isx::Image & inImage);

/// \param  inFilePath      The path of the file from which to read actual lines.
/// \param  inExpLines      The expected lines.
void
requireEqualLines(
        const std::string & inFilePath,
        const std::vector<std::string> & inExpLines);

/// \param  inFilePath      The path of the file from which to read actual values.
/// \param  inExpValues     The expected values.
/// \param  inRelTol        The relative difference allowed for approximate equality.
/// \sa approxEqual
void
requireEqualCsvValues(
        const std::string & inFilePath,
        const std::string & inExpTitleLine,
        const std::vector<std::vector<double>> & inExpValues,
        const double inRelTol = 0.0);

/// \param  inActual    The actual events.
/// \param  inExpected  The expected events.
/// \param  inRelTo     The relative difference allowed for approximate equality of values.
void
requireEqualEvents(
        const isx::SpEvents_t & inActual,
        const isx::SpEvents_t & inExpected,
        const double inRelTol = 0.0);

/// \param  inActual    The actual cell set
/// \param  inExpected  The expected cell set
void 
requireEqualCellSets(
        const isx::SpCellSet_t & inActual,
        const isx::SpCellSet_t & inExpected);

/// \param  inActual    The actual vessel set
/// \param  inExpected  The expected vessel set
void
requireEqualVesselSets(
        const isx::SpVesselSet_t & inActual,
        const isx::SpVesselSet_t & inExpected);

/// \param  inActual    The actual cell set
/// \param  inExpected  The expected cell set
void requireEqualCellStatuses(
    const isx::SpCellSet_t & inActual,
    const isx::SpCellSet_t & inExpected);

/// \param  inActual    The actual vessel set
/// \param  inExpected  The expected vessel set
void requireEqualVesselStatuses(
        const isx::SpVesselSet_t & inActual,
        const isx::SpVesselSet_t & inExpected);

/// \param  inActual    The actual vessel line
/// \param  inExpected  The expected vessel line
void requireEqualVesselLines(
        const isx::SpVesselLine_t & inActual,
        const isx::SpVesselLine_t & inExpected);

/// \param  inActual    The actual vessel correlation triptych
/// \param  inExpected  The expected vessel correlation triptych
void requireEqualVesselCorrelations(
    const isx::SpVesselCorrelations_t & inActual,
    const isx::SpVesselCorrelations_t & inExpected);

/// \param  actualOutputFilename    Filename of actual spatial overlap matrix
/// \param  expectedOutputFilename  Filename of expected spatial overlap matrix
void
requireEqualSpatialOverlapMatrices(
    const std::string actualOutputFilename,
    const std::string expectedOutputFilename,
    const double relTol = 0);

/// \param  actualOutputFilename    Filename of actual registration matrix
/// \param  expectedOutputFilename  Filename of expected registration matrix
void requireEqualRegistrationMatrices(
    const std::string actualOutputFilename,
    const std::string expectedOutputFilename);

/// \param  inActual    Actual vector
/// \param  inExpected  Expected vector
template<typename T>
void requireEqualVectors(
    const std::vector<T> & inActual,
    const std::vector<T> & inExpected)
{
    REQUIRE((inActual.size() == inExpected.size()));

    size_t numMatches = 0;
    for (size_t i = 0; i < inActual.size(); i++)
    {
        if (inActual[i] == inExpected[i])
        {
            numMatches++;
        }
    }

    REQUIRE(numMatches == inActual.size());
}

/// Require that the expected and actual movies are identical.
///
/// \param  inActual    Path to the actual movie.
/// \param  inExpected  Path to the expected movie.
/// \param  inRelTol    The relative difference allowed for approximate equality.
///                     If 0, the values must be exactly equal.
void requireEqualMovies(
    const std::string inActual,
    const std::string inExpected,
    const double inRelTol);

/// \param  inActual    The actual value.
/// \param  inExpected  The expected value.
/// \param  inRelTol    The relative difference allowed for approximate equality.
///                     If 0, the values must be exactly equal.
/// \return             True if the actual value is approximately equal to the expected value.
bool
approxEqual(
        const double inActual,
        const double inExpected,
        const double inRelTol);

/// Remove a directory then remake it so that it is empty.
///
void makeCleanDirectory(const std::string & inPath);

/// Read a csv file into a 2D-array of a single type of data
/// \param  inFilename  Filename of csv file
/// \param  outColumns  Column names
/// \param  outRows     Row names (empty if the file does not use a row index)
/// \param  outData     2D array of data
template<typename T>
void readCsvAs2DArray(
    const std::string inFilename,
    std::vector<std::string> & outColumns,
    std::vector<std::string> & outRows,
    std::vector<std::vector<T>> & outData)
{
    std::ifstream file(inFilename);
    if (!file.is_open() || !file.good())
    {
        ISX_THROW(isx::ExceptionUserInput, "Could not open csv file");
    }

    std::string line, column;
    std::stringstream ss;

    std::getline(file, line);
    ss.str(line);
    bool namedRows = ss.peek() == ',';
    if (namedRows) ss.ignore();
    while (std::getline(ss, column, ','))
    {
        outColumns.push_back(column);
    }

    int rowIndex = 0;
    T value;
    std::string strValue;
    while (std::getline(file, line))
    {
        ss.clear();
        ss.str(line);

        if (namedRows)
        {
            std::string row;
            std::getline(ss, row, ',');
            outRows.push_back(row);
        }

        outData.push_back({});
        while (std::getline(ss, strValue, ','))
        {
            std::stringstream ssValue(strValue);
            ssValue >> value;
            outData[rowIndex].push_back(value);
            if (ss.peek() == ',') ss.ignore();
        }
        rowIndex++;
    }

    file.close();
}

/// Writes a synthetic movie
/// If using a constant pixel value per frame then each pixel value is equal to the frame number
/// Otherwise each pixel value is equal to the frame number + pixel number.
///
/// \param  inFileName                  The file name of the movie to write.
/// \param  inTimingInfo                The timing info of the movie to write.
/// \param  inSpacingInfo               The spacing info of the movie to write.
/// \param  inUseConstantPixelPerFrame  Specifies whether to use constant pixel values.
/// \param  inDataType                  The datatype of the movie to write.
void
writeSyntheticMovie(
    const std::string & inFileName,
    const isx::TimingInfo & inTimingInfo,
    const isx::SpacingInfo & inSpacingInfo,
    const bool inUseConstantPixelPerFrame = false,
    const isx::DataType inDataType = isx::DataType::F32);

/// \return The lines from a plain text file.
///
std::vector<std::string> getLinesFromFile(const std::string & inFilePath);

/// \def ISX_LOG_NO_NEW_LINE(MESSAGE)
///
/// Logs a message without a new line.
///
/// \param  MESSAGE     The message to log.
#if ISX_OS_WIN32
#define ISX_LOG_NO_NEW_LINE(MESSAGE) \
    if (IsDebuggerPresent()) \
    { \
        OutputDebugString(MESSAGE); \
    } \
    else \
    { \
        std::cout << MESSAGE; \
        std::cout << std::flush; \
    }
#else
#define ISX_LOG_NO_NEW_LINE(MESSAGE) \
    std::cout << MESSAGE; \
    std::cout << std::flush;
#endif

/// \def    ISX_EXPECT_EXCEPTION
///
/// Log a warning message before an expected exception.
#define ISX_EXPECT_EXCEPTION() \
    ISX_LOG_NO_NEW_LINE("EXPECTED EXCEPTION: ");

/// \def    ISX_EXPECT_WARNING
///
/// Log a warning message before an expected warning.
#define ISX_EXPECT_WARNING() \
    ISX_LOG_NO_NEW_LINE("EXPECTED WARNING: ");

/// \def ISX_REQUIRE_EXCEPTION(command, type, message)
///
/// Require that running a command throws an exception of a given type
/// and a given message.
///
/// \param  COMMAND     The command to run.
/// \param  TYPE        The expected type of the exception.
/// \param  MESSAGE     The expected message of the exception.
#define ISX_REQUIRE_EXCEPTION(COMMAND, TYPE, MESSAGE)\
    ISX_EXPECT_EXCEPTION();\
    {\
        bool caught = false;\
        try\
        {\
            COMMAND;\
        }\
        catch (const TYPE & error)\
        {\
            caught = true;\
            if (!std::string(MESSAGE).empty())\
            {\
                REQUIRE(std::string(error.what()) == MESSAGE);\
            }\
        }\
        catch (...)\
        {\
            caught = true;\
            FAIL("Failed to throw an exception of type " #TYPE);\
        }\
        if (!caught)\
        {\
            FAIL("Failed to throw an exception.");\
        }\
    }

#endif // ISX_TEST_H
