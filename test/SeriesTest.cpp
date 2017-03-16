#include "isxSeries.h"
#include "isxDataSet.h"
#include "isxMovieFactory.h"

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
            movie2OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File;
    createSeriesTestData(movie1File, movie2File, movie3File,
            movie2OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File);

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
    
    auto movie1Series = std::make_shared<isx::Series>(movie1);
    auto movie2Series = std::make_shared<isx::Series>(movie2);
    auto movie3Series = std::make_shared<isx::Series>(movie3);
    auto movie2OverlapSeries = std::make_shared<isx::Series>(movie2Overlap);
    auto movie2Step2Series = std::make_shared<isx::Series>(movie2Step2);
    auto movie2CroppedSeries = std::make_shared<isx::Series>(movie2Cropped);
    auto movie2F32Series = std::make_shared<isx::Series>(movie2F32);
    auto behavMovieSeries = std::make_shared<isx::Series>(behavMovie);
    auto movie2PreprocessedSeries = std::make_shared<isx::Series>(movie2Preprocessed);

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
                "The timing info temporally overlaps with the reference.");
    }

    SECTION("Try to insert a movie that has a different frame rate than an existing movie in the series")
    {
        series.insertUnitarySeries(movie1Series);

        ISX_REQUIRE_EXCEPTION(
                series.insertUnitarySeries(movie2Step2Series),
                isx::ExceptionSeries,
                "The timing info has a different frame rate than the reference.");
    }

    SECTION("Try to insert a movie that has different spacing info than an existing movie in the series")
    {
        series.insertUnitarySeries(movie1Series);

        ISX_REQUIRE_EXCEPTION(
                series.insertUnitarySeries(movie2CroppedSeries),
                isx::ExceptionSeries,
                "The spacing info is different than that of the reference.");
    }

    SECTION("Try to insert a movie that has different data type than an existing movie in the series")
    {
        series.insertUnitarySeries(movie1Series);

        ISX_REQUIRE_EXCEPTION(
                series.insertUnitarySeries(movie2F32Series),
                isx::ExceptionSeries,
                "The data type is different from that of the reference.");
    }

    SECTION("Try to insert a behavioral movie")
    {
        series.insertUnitarySeries(movie1Series);

        ISX_REQUIRE_EXCEPTION(
                series.insertUnitarySeries(behavMovieSeries),
                isx::ExceptionSeries,
                "The DataSet type is different from that of the reference.");
    }

    SECTION("Try to insert a movie with different history")
    {
        series.insertUnitarySeries(movie1Series);

        ISX_REQUIRE_EXCEPTION(
                series.insertUnitarySeries(movie2PreprocessedSeries),
                isx::ExceptionSeries,
                "The history details are different than those of the reference.");
    }
}

TEST_CASE("Series-removeDataSet", "[core]")
{
    isx::Series series("series");

    std::string movie1File, movie2File, movie3File,
            movie2OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File;
    createSeriesTestData(movie1File, movie2File, movie3File,
            movie2OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File);

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
            movie2OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File;
    createSeriesTestData(movie1File, movie2File, movie3File,
            movie2OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File);

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
