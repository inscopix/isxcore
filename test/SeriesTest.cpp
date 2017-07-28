#include "isxSeries.h"
#include "isxDataSet.h"
#include "isxMovieFactory.h"
#include "isxCellSetFactory.h"
#include "isxPathUtils.h"

#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"

TEST_CASE("Series-Series", "[core]")
{

    SECTION("Empty constructor")
    {
        isx::Series series;

        REQUIRE(!series.isValid());
    }

    SECTION("Construct a series")
    {
        isx::Series series("series");

        REQUIRE(series.isValid());
        REQUIRE(series.getName() == "series");
        REQUIRE(series.getParent() == nullptr);
        REQUIRE(series.getUniqueIdentifier() != "");
        REQUIRE(series.getNumChildren() == 0);
    }

}

TEST_CASE("Series-insertUnitarySeries", "[core]")
{
    isx::Series series("series");

    std::string movie1File, movie2File, movie3File,
            movie2OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File, image1File;
    createSeriesTestData(movie1File, movie2File, movie3File,
            movie2OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File, image1File);

    isx::HistoricalDetails hd;
    isx::HistoricalDetails hdPreprocessed("Preprocessing", "");

    auto movie1 = std::make_shared<isx::DataSet>("movie1", isx::DataSet::Type::MOVIE, movie1File, hd);
    auto movie2 = std::make_shared<isx::DataSet>("movie2", isx::DataSet::Type::MOVIE, movie2File, hd);
    auto movie3 = std::make_shared<isx::DataSet>("movie3", isx::DataSet::Type::MOVIE, movie3File, hd);
    auto movie2Overlap = std::make_shared<isx::DataSet>("movie2Overlap", isx::DataSet::Type::MOVIE, movie2OverlapFile, hd);
    auto movie2Step2 = std::make_shared<isx::DataSet>("movie2Step2", isx::DataSet::Type::MOVIE, movie2Step2File, hd);
    auto movie2Cropped = std::make_shared<isx::DataSet>("movie2Cropped", isx::DataSet::Type::MOVIE, movie2CroppedFile, hd);
    auto movie2F32 = std::make_shared<isx::DataSet>("movie2F32", isx::DataSet::Type::MOVIE, movie2F32File, hd);
    auto behavMovie = std::make_shared<isx::DataSet>("behavior", isx::DataSet::Type::BEHAVIOR, "behavior.mpg", hd);
    auto movie2Preprocessed = std::make_shared<isx::DataSet>("movie2Preprocessed", isx::DataSet::Type::MOVIE, movie2File, hdPreprocessed);
    auto image1 = std::make_shared<isx::DataSet>("image1", isx::DataSet::Type::IMAGE, image1File, hd);

    auto movie1Series = std::make_shared<isx::Series>(movie1);
    auto movie2Series = std::make_shared<isx::Series>(movie2);
    auto movie3Series = std::make_shared<isx::Series>(movie3);
    auto movie2OverlapSeries = std::make_shared<isx::Series>(movie2Overlap);
    auto movie2Step2Series = std::make_shared<isx::Series>(movie2Step2);
    auto movie2CroppedSeries = std::make_shared<isx::Series>(movie2Cropped);
    auto movie2F32Series = std::make_shared<isx::Series>(movie2F32);
    auto behavMovieSeries = std::make_shared<isx::Series>(behavMovie);
    auto movie2PreprocessedSeries = std::make_shared<isx::Series>(movie2Preprocessed);
    auto image1Series = std::make_shared<isx::Series>(image1);

    SECTION("Insert one data set")
    {
        series.insertUnitarySeries(movie1Series);

        REQUIRE(series.getDataSet(0) == movie1.get());
    }

    SECTION("Insert the same data set into two different series")
    {
        series.insertUnitarySeries(movie1Series);

        isx::Series series2("series2");
        ISX_REQUIRE_EXCEPTION(
                series2.insertUnitarySeries(movie1Series),
                isx::ExceptionDataIO,
                "Series is already in another container!");

        REQUIRE(series.getDataSet(0) == movie1.get());
    }


    SECTION("Add two consistent movies to the series in order")
    {
        series.insertUnitarySeries(movie1Series);
        series.insertUnitarySeries(movie2Series);

        REQUIRE(series.getDataSet(0) == movie1.get());
        REQUIRE(series.getDataSet(1) == movie2.get());
    }

    SECTION("Insert two consistent movies to the series out of order")
    {
        series.insertUnitarySeries(movie2Series);
        series.insertUnitarySeries(movie1Series);
        
        REQUIRE(series.getDataSet(0) == movie1.get());
        REQUIRE(series.getDataSet(1) == movie2.get());
    }

    SECTION("Try to insert a movie that temporally overlaps with an existing movie in the series")
    {
        series.insertUnitarySeries(movie1Series);

        ISX_REQUIRE_EXCEPTION(
                series.insertUnitarySeries(movie2OverlapSeries),
                isx::ExceptionSeries,
                "Unable to insert data that temporally overlaps with other parts of the series. Data sets in a series must all be non-overlapping.");
    }

    SECTION("Try to insert a movie that has a different frame rate than an existing movie in the series")
    {
        series.insertUnitarySeries(movie1Series);

        ISX_REQUIRE_EXCEPTION(
                series.insertUnitarySeries(movie2Step2Series),
                isx::ExceptionSeries,
                "Unable to add a data set with a different frame rate than the rest of the series.");
    }

    SECTION("Try to insert a movie that has different spacing info than an existing movie in the series")
    {
        series.insertUnitarySeries(movie1Series);

        ISX_REQUIRE_EXCEPTION(
                series.insertUnitarySeries(movie2CroppedSeries),
                isx::ExceptionSeries,
                "The new data set has different spacing information than the rest of the series. Spacing information must be equal among series' components.");
    }

    SECTION("Try to insert a movie that has different data type than an existing movie in the series")
    {
        series.insertUnitarySeries(movie1Series);

        ISX_REQUIRE_EXCEPTION(
                series.insertUnitarySeries(movie2F32Series),
                isx::ExceptionSeries,
                "Unable to add data of type float to a series with data of type uint16.");
    }

    SECTION("Try to insert a behavioral movie")
    {
        series.insertUnitarySeries(movie1Series);

        ISX_REQUIRE_EXCEPTION(
                series.insertUnitarySeries(behavMovieSeries),
                isx::ExceptionSeries,
                "Unable to add a data set of type Behavior to a series of type Movie.");
    }

    SECTION("Try to insert a movie with different history")
    {
        series.insertUnitarySeries(movie1Series);

        ISX_REQUIRE_EXCEPTION(
                series.insertUnitarySeries(movie2PreprocessedSeries),
                isx::ExceptionSeries,
                "The new data set has a different processing history than the rest of the series' components. Only unprocessed data sets can be moved into a series.");
    }

    SECTION("Try to insert an image in an empty series")
    {
        ISX_REQUIRE_EXCEPTION(
                series.insertUnitarySeries(image1Series),
                isx::ExceptionSeries,
                "You are attempting to add an image to a series. Image series are not yet supported.");
    }

}

TEST_CASE("Series-removeDataSet", "[core]")
{
    isx::Series series("series");

    std::string movie1File, movie2File, movie3File,
            movie2OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File, image1File;
    createSeriesTestData(movie1File, movie2File, movie3File,
            movie2OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File, image1File);

    isx::HistoricalDetails hd;

    auto movie1 = std::make_shared<isx::DataSet>("movie1", isx::DataSet::Type::MOVIE, movie1File, hd);
    auto movie2 = std::make_shared<isx::DataSet>("movie2", isx::DataSet::Type::MOVIE, movie2File, hd);
    auto movie3 = std::make_shared<isx::DataSet>("movie3", isx::DataSet::Type::MOVIE, movie3File, hd);

    auto movie1Series = std::make_shared<isx::Series>(movie1);
    auto movie2Series = std::make_shared<isx::Series>(movie2);
    auto movie3Series = std::make_shared<isx::Series>(movie3);

    series.insertUnitarySeries(movie1Series);
    series.insertUnitarySeries(movie2Series);
    series.insertUnitarySeries(movie3Series);


#if ISX_OS_WIN32
    const char * vectorOutOfRangeMessage = "invalid vector<T> subscript";
#elif ISX_OS_LINUX
    const char * vectorOutOfRangeMessage = "vector::_M_range_check";
#elif ISX_OS_MACOS
    const char * vectorOutOfRangeMessage = "vector";
#else
#error initialize vectorOutOfRangeMessage
#endif

    SECTION("Remove a data set at the end")
    {
        series.removeDataSet(movie3.get());

        REQUIRE(series.getNumDataSets() == 2);
        REQUIRE(series.getDataSet(0) == movie1.get());
        REQUIRE(series.getDataSet(1) == movie2.get());

        ISX_REQUIRE_EXCEPTION(
                series.getDataSet(2),
                std::out_of_range,
                vectorOutOfRangeMessage);
    }

    SECTION("Remove a data set in the middle")
    {
        series.removeDataSet(movie2.get());

        REQUIRE(series.getNumDataSets() == 2);
        REQUIRE(series.getDataSet(0) == movie1.get());
        REQUIRE(series.getDataSet(1) == movie3.get());

        ISX_REQUIRE_EXCEPTION(
                series.getDataSet(2),
                std::out_of_range,
                vectorOutOfRangeMessage);
    }

    SECTION("Try to remove a data set that does not exist")
    {
        series.removeDataSet(movie3.get());
        ISX_REQUIRE_EXCEPTION(
                series.removeDataSet(movie3.get()),
                isx::ExceptionDataIO,
                "Could not find item with the name: movie3");
    }

}

TEST_CASE("Series-toFromJsonString", "[core]")
{
    isx::Series series("series");

    std::string movie1File, movie2File, movie3File,
            movie2OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File, image1File;
    createSeriesTestData(movie1File, movie2File, movie3File,
            movie2OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File, image1File);

    isx::HistoricalDetails hd;
    auto movie1 = std::make_shared<isx::DataSet>("movie1", isx::DataSet::Type::MOVIE, movie1File, hd);
    auto movie2 = std::make_shared<isx::DataSet>("movie2", isx::DataSet::Type::MOVIE, movie2File, hd);
    
    auto movie1Series = std::make_shared<isx::Series>(movie1);
    auto movie2Series = std::make_shared<isx::Series>(movie2);

    SECTION("An empty series")
    {
        const std::string jsonString = series.toJsonString();
        const std::shared_ptr<isx::Series> actual = isx::Series::fromJsonString(jsonString);

        REQUIRE(*actual == series);
    }

    SECTION("A series containing two data sets")
    {
        series.insertUnitarySeries(movie1Series);
        series.insertUnitarySeries(movie2Series);

        const std::string jsonString = series.toJsonString();
        const std::shared_ptr<isx::Series> actual = isx::Series::fromJsonString(jsonString);

        REQUIRE(*actual == series);
    }

}

// Utility functions for the following tests.
namespace
{

isx::SpSeries_t
makeMovieSeries(
        const std::string & inName,
        const std::string & inFilePath,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        const bool inImported = false)
{
    std::remove(inFilePath.c_str());
    const isx::SpWritableMovie_t movie = isx::writeMosaicMovie(inFilePath, inTimingInfo, inSpacingInfo, isx::DataType::U16);
    movie->closeForWriting();
    const isx::HistoricalDetails history;
    const isx::DataSet::Properties properties;
    const auto dataSet = std::make_shared<isx::DataSet>(inName, isx::DataSet::Type::MOVIE, inFilePath, history, properties, inImported);
    return std::make_shared<isx::Series>(dataSet);
}

isx::SpSeries_t
makeBehavioralMovieSeries(
        const std::string & inName,
        const std::string & inFilePath,
        const isx::DataSet::Properties & inProperties)
{
    const isx::HistoricalDetails history;
    const auto dataSet = std::make_shared<isx::DataSet>(inName, isx::DataSet::Type::BEHAVIOR, inFilePath, history);
    return std::make_shared<isx::Series>(dataSet);
}

isx::SpSeries_t
makeCellSetSeries(
        const std::string & inName,
        const std::string & inFilePath,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo)
{
    std::remove(inFilePath.c_str());
    const isx::SpCellSet_t cellSet = isx::writeCellSet(inFilePath, inTimingInfo, inSpacingInfo);
    cellSet->closeForWriting();
    const isx::HistoricalDetails history;
    const auto dataSet = std::make_shared<isx::DataSet>(inName, isx::DataSet::Type::CELLSET, inFilePath, history);
    return std::make_shared<isx::Series>(dataSet);
}

isx::SpSeries_t
makeGpioSeries(
        const std::string & inName,
        const std::string & inFilePath)
{
    const isx::HistoricalDetails history;
    const auto dataSet = std::make_shared<isx::DataSet>(inName, isx::DataSet::Type::GPIO, inFilePath, history);
    return std::make_shared<isx::Series>(dataSet);
}

isx::SpSeries_t
makeImageSeries(
        const std::string & inName,
        const std::string & inFilePath)
{
    const isx::HistoricalDetails history;
    const auto dataSet = std::make_shared<isx::DataSet>(inName, isx::DataSet::Type::IMAGE, inFilePath, history);
    return std::make_shared<isx::Series>(dataSet);
}

isx::SpSeries_t
makeImageSeries(
        const std::string & inName,
        const std::string & inFilePath,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo)
{
    ISX_ASSERT(inTimingInfo.getNumTimes() == 1);
    std::remove(inFilePath.c_str());
    const isx::SpWritableMovie_t movie = isx::writeMosaicMovie(inFilePath, inTimingInfo, inSpacingInfo, isx::DataType::U16);
    movie->closeForWriting();
    const isx::HistoricalDetails history;
    const auto dataSet = std::make_shared<isx::DataSet>(inName, isx::DataSet::Type::IMAGE, inFilePath, history);
    return std::make_shared<isx::Series>(dataSet);
}

isx::SpSeries_t
makeSeries(const std::string & inName, const std::vector<isx::SpSeries_t> & inMembers)
{
    auto series = std::make_shared<isx::Series>(inName);
    for (const auto & m : inMembers)
    {
        series->insertUnitarySeries(m);
    }
    return series;
}

} // namespace

TEST_CASE("Series-addChildWithCompatibilityCheck", "[core]")
{
    // Parent has type U16 and has two potential time segments:
    // - 20 samples at 20 Hz starting at 18:15:49
    // - 15 samples at 20 Hz starting at 18:15:51 (2 seconds or 20 samples later)
    // and two potential number of pixels.
    const isx::DataType dataType = isx::DataType::U16;
    const isx::DurationInSeconds step(50, 1000);

    const isx::SpacingInfo si1(isx::SizeInPixels_t(4, 3));
    const isx::SpacingInfo si2(isx::SizeInPixels_t(3, 2));

    const isx::DataSet::Properties props1{
        { isx::DataSet::PROP_MOVIE_START_TIME, isx::Variant(isx::Time()) },
        { isx::DataSet::PROP_BEHAV_GOP_SIZE, isx::Variant(int64_t(10)) },
        { isx::DataSet::PROP_BEHAV_NUM_FRAMES, isx::Variant(int64_t(16)) }
    };

    const isx::Time start1(2017, 4, 18, 15, 49);
    const isx::isize_t numTimes1 = 20;
    const isx::TimingInfo ti1(start1, step, numTimes1);

    const isx::Time start2(2017, 4, 18, 15, 51);
    const isx::isize_t numTimes2 = 15;
    const isx::TimingInfo ti2(start2, step, numTimes2);

    const std::string parentFilePath1 = g_resources["unitTestDataPath"] + "/Series-addChildWithCompatibilityCheck-parent1.isxd";
    const std::string parentFilePath2 = g_resources["unitTestDataPath"] + "/Series-addChildWithCompatibilityCheck-parent2.isxd";

    const std::string childFilePath1 = g_resources["unitTestDataPath"] + "/Series-addChildWithCompatibilityCheck-child1.isxd";
    const std::string childFilePath2 = g_resources["unitTestDataPath"] + "/Series-addChildWithCompatibilityCheck-child2.isxd";

    std::string errorMessage;

    SECTION("Parent has one movie")
    {
        const isx::SpSeries_t parentSeries = makeMovieSeries("parent", parentFilePath1, ti1, si1);

        SECTION("Child has one movie with the same timing info as the parent")
        {
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, ti1, si1);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one movie that is temporally contained in the parent")
        {
            const isx::TimingInfo childTi(start1 + step, step, numTimes1 - 4);
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, childTi, si1);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one movie that begins before the parent")
        {
            const isx::TimingInfo childTi(start1 - step, step, numTimes1);
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, childTi, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one movie that ends after the parent")
        {
            const isx::TimingInfo childTi(ti1.getEnd() + step, step, numTimes1);
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, childTi, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one movie with different number of pixels to the parent")
        {
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, ti1, si2);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset with the same timing info as the parent")
        {
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, ti1, si1);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset that is temporally contained in the parent")
        {
            const isx::TimingInfo childTi(start1 + step, step, numTimes1 - 2);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset that begins before the parent")
        {
            const isx::TimingInfo childTi(start1 - step, step, numTimes1);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset that ends after the parent")
        {
            const isx::TimingInfo childTi(ti1.getEnd() + step, step, numTimes1);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset with different number of pixels to the parent")
        {
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, ti1, si2);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one behavioral movie")
        {
            const isx::SpSeries_t childSeries = makeBehavioralMovieSeries("child", g_resources["unitTestDataPath"] + "/trial9_OneSec.mpg", props1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one GPIO")
        {
            const isx::SpSeries_t childSeries = makeGpioSeries("child", g_resources["unitTestDataPath"] + "/test_gpio_analog_2.isxd");

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one valid image")
        {
            const isx::SpSeries_t childSeries = makeImageSeries("child", childFilePath1, isx::TimingInfo(ti1.getStart(), ti1.getStep(), 1), si1);
            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one invalid snapshot image")
        {
            const isx::SpSeries_t childSeries = makeImageSeries("child", g_resources["unitTestDataPath"] + "/Snapshots/nVista_ratPFC2_tif/snapshot_20160705_094404.xml");
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

    }

    SECTION("Parent has two movies")
    {
        const isx::SpSeries_t movie1 = makeMovieSeries("movie1", parentFilePath1, ti1, si1);
        const isx::SpSeries_t movie2 = makeMovieSeries("movie2", parentFilePath2, ti2, si1);
        const isx::SpSeries_t parentSeries = makeSeries("parent", {movie1, movie2});

        SECTION("Child has one movie with the same timing span as the parent")
        {
            const isx::TimingInfo childTi(start1, step, numTimes1 + numTimes2 + 20);
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, childTi, si1);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one movie that is temporally contained in the parent")
        {
            const isx::TimingInfo childTi(start1 + step, step, numTimes1 - 4);
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, childTi, si1);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one movie that begins before the parent")
        {
            const isx::TimingInfo childTi(start1 - step, step, numTimes1);
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, childTi, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one movie that ends after the parent")
        {
            const isx::TimingInfo childTi(ti2.getEnd() + step, step, numTimes1);
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, childTi, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one movie with a different number of pixels to the parent")
        {
            const isx::TimingInfo childTi(start1, step, numTimes1 + numTimes2 + 20);
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, childTi, si2);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset with the same timing span as the parent")
        {
            const isx::TimingInfo childTi(start1, step, numTimes1 + numTimes2 + 20);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset that is temporally contained in the parent")
        {
            const isx::TimingInfo childTi(start1 + step, step, numTimes1 - 2);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset that begins before the parent")
        {
            const isx::TimingInfo childTi(start1 - step, step, numTimes1);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset that ends after the parent")
        {
            const isx::TimingInfo childTi(ti2.getEnd() + step, step, numTimes1);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset with a different number of pixels to the parent")
        {
            const isx::TimingInfo childTi(start1, step, numTimes1 + numTimes2 + 20);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si2);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one behavioral movie")
        {
            const isx::SpSeries_t childSeries = makeBehavioralMovieSeries("child", g_resources["unitTestDataPath"] + "/trial9_OneSec.mpg", props1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one valid image")
        {
            const isx::SpSeries_t childSeries = makeImageSeries("child", childFilePath1, isx::TimingInfo(ti1.getStart(), ti1.getStep(), 1), si1);
            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one invalid snapshot image")
        {
            const isx::SpSeries_t childSeries = makeImageSeries("child", g_resources["unitTestDataPath"] + "/Snapshots/nVista_ratPFC2_tif/snapshot_20160705_094404.xml");
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

    }

    SECTION("Parent has one cellset")
    {
        const isx::SpSeries_t parentSeries = makeCellSetSeries("parent", parentFilePath1, ti1, si1);

        SECTION("Child has one cellset with the same timing info as the parent")
        {
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, ti1, si1);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset that is temporally contained in the parent")
        {
            const isx::TimingInfo childTi(start1 + step, step, numTimes1 - 2);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset that begins before the parent")
        {
            const isx::TimingInfo childTi(start1 - step, step, numTimes1);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset that ends after the parent")
        {
            const isx::TimingInfo childTi(ti1.getEnd() + step, step, numTimes1);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one movie with the same timing info as the parent")
        {
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, ti1, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset with a different number of pixels to the parent")
        {
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, ti1, si2);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one behavioral movie")
        {
            const isx::SpSeries_t childSeries = makeBehavioralMovieSeries("child", g_resources["unitTestDataPath"] + "/trial9_OneSec.mpg", props1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one GPIO")
        {
            const isx::SpSeries_t childSeries = makeGpioSeries("child", g_resources["unitTestDataPath"] + "/test_gpio_analog_2.isxd");

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one valid image")
        {
            const isx::SpSeries_t childSeries = makeImageSeries("child", childFilePath1, isx::TimingInfo(ti1.getStart(), ti1.getStep(), 1), si1);
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

    }

    SECTION("Parent has two cellsets")
    {
        const isx::SpSeries_t cellSet1 = makeCellSetSeries("cellSet1", parentFilePath1, ti1, si1);
        const isx::SpSeries_t cellSet2 = makeCellSetSeries("cellSet2", parentFilePath2, ti2, si1);
        const isx::SpSeries_t parentSeries = makeSeries("parent", {cellSet1, cellSet2});

        SECTION("Child has one cellset with the same timing span as the parent")
        {
            const isx::TimingInfo childTi(start1, step, numTimes1 + numTimes2 + 20);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset that is temporally contained in the parent")
        {
            const isx::TimingInfo childTi(start1 + step, step, numTimes1 - 2);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset that begins before the parent")
        {
            const isx::TimingInfo childTi(start1 - step, step, numTimes1);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset that ends after the parent")
        {
            const isx::TimingInfo childTi(ti2.getEnd() + step, step, numTimes1);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one movie with the same timing span as the parent")
        {
            const isx::TimingInfo childTi(start1, step, numTimes1 + numTimes2 + 20);
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, childTi, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset with a different number of pixels to the parent")
        {
            const isx::TimingInfo childTi(start1, step, numTimes1 + numTimes2 + 20);
            const isx::SpSeries_t childSeries = makeCellSetSeries("child", childFilePath1, childTi, si2);

            REQUIRE(parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one behavioral movie")
        {
            const isx::SpSeries_t childSeries = makeBehavioralMovieSeries("child", g_resources["unitTestDataPath"] + "/trial9_OneSec.mpg", props1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one valid image")
        {
            const isx::SpSeries_t childSeries = makeImageSeries("child", childFilePath1, isx::TimingInfo(ti1.getStart(), ti1.getStep(), 1), si1);
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

    }

    SECTION("Parent has one behavioral movie")
    {
        const isx::SpSeries_t parentSeries = makeBehavioralMovieSeries("parent", g_resources["unitTestDataPath"] + "/trial9_OneSec.mpg", props1);

        SECTION("Child has one movie")
        {
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, ti1, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset")
        {
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, ti1, si1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one behavioral movie")
        {
            const isx::SpSeries_t childSeries = makeBehavioralMovieSeries("child", g_resources["unitTestDataPath"] + "/trial9_OneSec.mpg", props1);

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one GPIO")
        {
            const isx::SpSeries_t childSeries = makeGpioSeries("child", g_resources["unitTestDataPath"] + "/test_gpio_analog_2.isxd");

            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one valid image")
        {
            const isx::SpSeries_t childSeries = makeImageSeries("child", childFilePath1, isx::TimingInfo(ti1.getStart(), ti1.getStep(), 1), si1);
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

    }

    SECTION("Parent has one GPIO")
    {
        const isx::SpSeries_t parentSeries = makeGpioSeries("parent", g_resources["unitTestDataPath"] + "/test_gpio_analog_2.isxd");

        SECTION("Child has one movie")
        {
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, ti1, si1);
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset")
        {
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, ti1, si1);
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one behavioral movie")
        {
            const isx::SpSeries_t childSeries = makeBehavioralMovieSeries("child", g_resources["unitTestDataPath"] + "/trial9_OneSec.mpg", props1);
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one GPIO")
        {
            const isx::SpSeries_t childSeries = makeGpioSeries("child", g_resources["unitTestDataPath"] + "/test_gpio_events_2.isxd");
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one valid image")
        {
            const isx::SpSeries_t childSeries = makeImageSeries("child", childFilePath1, isx::TimingInfo(ti1.getStart(), ti1.getStep(), 1), si1);
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

    }

    SECTION("Parent has one image")
    {
        const isx::SpSeries_t parentSeries = makeImageSeries("parent", parentFilePath1, isx::TimingInfo(ti1.getStart(), ti1.getStep(), 1), si1);

        SECTION("Child has one movie")
        {
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, ti1, si1);
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one cellset")
        {
            const isx::SpSeries_t childSeries = makeMovieSeries("child", childFilePath1, ti1, si1);
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one behavioral movie")
        {
            const isx::SpSeries_t childSeries = makeBehavioralMovieSeries("child", g_resources["unitTestDataPath"] + "/trial9_OneSec.mpg", props1);
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one GPIO")
        {
            const isx::SpSeries_t childSeries = makeGpioSeries("child", g_resources["unitTestDataPath"] + "/test_gpio_events_2.isxd");
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

        SECTION("Child has one valid image")
        {
            const isx::SpSeries_t childSeries = makeImageSeries("child", childFilePath1, isx::TimingInfo(ti1.getStart(), ti1.getStep(), 1), si1);
            REQUIRE(!parentSeries->addChildWithCompatibilityCheck(childSeries, errorMessage));
        }

    }

    std::remove(parentFilePath1.c_str());
    std::remove(parentFilePath2.c_str());
    std::remove(childFilePath1.c_str());
    std::remove(childFilePath2.c_str());
}

// Utility functions for below test case
namespace
{

std::string
makeGetChildrenFilePath(const std::string & inName)
{
    return g_resources["unitTestDataPath"] + "/Series-getChildren-" + inName + ".isxd";
}

} // namespace

TEST_CASE("Series-getChildren", "[core]")
{
    const isx::TimingInfo ti(isx::Time(), isx::DurationInSeconds(50, 1000), 4);
    const isx::SpacingInfo si(isx::SizeInPixels_t(2, 3));

    isx::SpSeries_t series = std::make_shared<isx::Series>("series");
    isx::SpSeries_t child1 = makeMovieSeries("child1", makeGetChildrenFilePath("child1"), ti, si);
    isx::SpSeries_t child2 = makeMovieSeries("child2", makeGetChildrenFilePath("child2"), ti, si);
    isx::SpSeries_t child3 = makeMovieSeries("child3", makeGetChildrenFilePath("child3"), ti, si);
    isx::SpSeries_t grandChild1 = makeMovieSeries("grandChild1", makeGetChildrenFilePath("grandChild1"), ti, si);
    isx::SpSeries_t grandChild2 = makeMovieSeries("grandChild2", makeGetChildrenFilePath("grandChild2"), ti, si);
    isx::SpSeries_t grandChild3 = makeMovieSeries("grandChild3", makeGetChildrenFilePath("grandChild3"), ti, si);
    isx::SpSeries_t greatGrandChild1 = makeMovieSeries("greatGrandChild1", makeGetChildrenFilePath("greatGrandChild1"), ti, si);

    series->addChild(child1);
    child1->addChild(grandChild1);
    child1->addChild(grandChild2);
    grandChild2->addChild(greatGrandChild1);

    series->addChild(child2);
    child2->addChild(grandChild3);

    series->addChild(child3);

    SECTION("Get children only")
    {
        const std::vector<isx::Series *> expected = {child1.get(), child2.get(), child3.get()};
        REQUIRE(series->getChildren(false) == expected);
    }

    SECTION("Get all descendants")
    {
        const std::vector<isx::Series *> expected = {child1.get(), grandChild1.get(), grandChild2.get(), greatGrandChild1.get(), child2.get(), grandChild3.get(), child3.get()};
        REQUIRE(series->getChildren(true) == expected);
    }

    SECTION("Get children of child1.get()")
    {
        const std::vector<isx::Series *> expected = {grandChild1.get(), grandChild2.get()};
        REQUIRE(child1.get()->getChildren(false) == expected);
    }

    SECTION("Get descendants of child1.get()")
    {
        const std::vector<isx::Series *> expected = {grandChild1.get(), grandChild2.get(), greatGrandChild1.get()};
        REQUIRE(child1.get()->getChildren(true) == expected);
    }

    for (const auto & s : series->getChildren(true))
    {
        for (const auto & d : s->getDataSets())
        {
            std::remove(d->getFileName().c_str());
        }
    }
}

namespace
{

std::string
makeDeleteFilesFilePath(const std::string & inName)
{
    return g_resources["unitTestDataPath"] + "/Series-deleteFiles-" + inName + ".isxd";
}

} // namespace

TEST_CASE("Series-deleteFiles", "[core]")
{
    const isx::TimingInfo ti1(isx::Time(), isx::DurationInSeconds(50, 1000), 4);
    const isx::TimingInfo ti2(isx::Time() + isx::DurationInSeconds(1000, 1000), isx::DurationInSeconds(50, 1000), 4);
    const isx::SpacingInfo si(isx::SizeInPixels_t(2, 3));
    isx::SpSeries_t series = std::make_shared<isx::Series>("series");

    SECTION("Delete two non-imported files in a series")
    {
        isx::SpSeries_t us1 = makeMovieSeries("us1", makeDeleteFilesFilePath("us1"), ti1, si, false);
        isx::SpSeries_t us2 = makeMovieSeries("us2", makeDeleteFilesFilePath("us2"), ti2, si, false);

        series->insertUnitarySeries(us1);
        series->insertUnitarySeries(us2);

        series->deleteFiles();

        REQUIRE(!isx::pathExists(us1->getDataSet(0)->getFileName()));
        REQUIRE(!isx::pathExists(us2->getDataSet(0)->getFileName()));
    }

    SECTION("Delete an imported and non-imported file in a series")
    {
        isx::SpSeries_t us1 = makeMovieSeries("us1", makeDeleteFilesFilePath("us1"), ti1, si, true);
        isx::SpSeries_t us2 = makeMovieSeries("us2", makeDeleteFilesFilePath("us2"), ti2, si, false);

        series->insertUnitarySeries(us1);
        series->insertUnitarySeries(us2);

        series->deleteFiles();

        REQUIRE(isx::pathExists(us1->getDataSet(0)->getFileName()));
        REQUIRE(!isx::pathExists(us2->getDataSet(0)->getFileName()));
    }

    SECTION("Delete imported files in a series")
    {
        isx::SpSeries_t us1 = makeMovieSeries("us1", makeDeleteFilesFilePath("us1"), ti1, si, true);
        isx::SpSeries_t us2 = makeMovieSeries("us2", makeDeleteFilesFilePath("us2"), ti2, si, true);

        series->insertUnitarySeries(us1);
        series->insertUnitarySeries(us2);

        series->deleteFiles();

        REQUIRE(isx::pathExists(us1->getDataSet(0)->getFileName()));
        REQUIRE(isx::pathExists(us2->getDataSet(0)->getFileName()));
    }

    for (const auto & s : series->getChildren(true))
    {
        for (const auto & d : s->getDataSets())
        {
            std::remove(d->getFileName().c_str());
        }
    }
}
