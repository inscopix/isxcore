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
        REQUIRE(series.getPath() == "series");
        REQUIRE(series.getNumChildren() == 0);
    }

}

TEST_CASE("Series-insertDataSet", "[core]")
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

    SECTION("Insert one data set")
    {
        series.insertDataSet(movie1);

        REQUIRE(series.getChildren().at(0) == movie1.get());
    }

    SECTION("Insert the same data set into two different series")
    {
        series.insertDataSet(movie1);

        isx::Series series2("series2");
        series2.insertDataSet(movie1);

        ISX_REQUIRE_EXCEPTION(
                series.getChild("movie1"),
                isx::ExceptionDataIO,
                "Could not find child with the name: movie1");

        REQUIRE(series2.getChildren().at(0) == movie1.get());
    }


    SECTION("Add two consistent movies to the series in order")
    {
        series.insertDataSet(movie1);
        series.insertDataSet(movie2);

        REQUIRE(series.getChildren().at(0) == movie1.get());
        REQUIRE(series.getChildren().at(1) == movie2.get());
    }

    SECTION("Insert two consistent movies to the series out of order")
    {
        series.insertDataSet(movie2);
        series.insertDataSet(movie1);

        REQUIRE(series.getChildren().at(0) == movie1.get());
        REQUIRE(series.getChildren().at(1) == movie2.get());
    }

    SECTION("Try to insert a movie that temporally overlaps with an existing movie in the series")
    {
        series.insertDataSet(movie1);

        ISX_REQUIRE_EXCEPTION(
                series.insertDataSet(movie2Overlap),
                isx::ExceptionSeries,
                "The timing info temporally overlaps with the reference.");
    }

    SECTION("Try to insert a movie that has a different frame rate than an existing movie in the series")
    {
        series.insertDataSet(movie1);

        ISX_REQUIRE_EXCEPTION(
                series.insertDataSet(movie2Step2),
                isx::ExceptionSeries,
                "The timing info has a different frame rate than the reference.");
    }

    SECTION("Try to insert a movie that has different spacing info than an existing movie in the series")
    {
        series.insertDataSet(movie1);

        ISX_REQUIRE_EXCEPTION(
                series.insertDataSet(movie2Cropped),
                isx::ExceptionSeries,
                "The spacing info is different than that of the reference.");
    }

    SECTION("Try to insert a movie that has different data type than an existing movie in the series")
    {
        series.insertDataSet(movie1);

        ISX_REQUIRE_EXCEPTION(
                series.insertDataSet(movie2F32),
                isx::ExceptionSeries,
                "The data type is different than that of the reference.");
    }

    SECTION("Try to insert a behavioral movie")
    {
        series.insertDataSet(movie1);

        ISX_REQUIRE_EXCEPTION(
                series.insertDataSet(behavMovie),
                isx::ExceptionSeries,
                "A series can only contain nVista movies.");
    }

    SECTION("Try to insert a movie with different history")
    {
        series.insertDataSet(movie1);

        ISX_REQUIRE_EXCEPTION(
                series.insertDataSet(movie2Preprocessed),
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

    series.insertDataSet(movie1);
    series.insertDataSet(movie2);
    series.insertDataSet(movie3);

    SECTION("Remove a data set at the end")
    {
        series.removeDataSet("movie3");

        REQUIRE(series.getChildren().size() == 2);
        REQUIRE(series.getChild("movie1") == movie1.get());
        REQUIRE(series.getChild("movie2") == movie2.get());

        ISX_REQUIRE_EXCEPTION(
                series.getChild("movie3"),
                isx::ExceptionDataIO,
                "Could not find child with the name: movie3");
    }

    SECTION("Remove a data set in the middle")
    {
        series.removeDataSet("movie2");

        REQUIRE(series.getChildren().size() == 2);
        REQUIRE(series.getChild("movie1") == movie1.get());
        REQUIRE(series.getChild("movie3") == movie3.get());

        ISX_REQUIRE_EXCEPTION(
                series.getChild("movie2"),
                isx::ExceptionDataIO,
                "Could not find child with the name: movie2");
    }

    SECTION("Try to remove a data set that does not exist")
    {
        ISX_REQUIRE_EXCEPTION(
                series.removeDataSet("movie"),
                isx::ExceptionDataIO,
                "Could not find data set with the name: movie");
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

    SECTION("An empty series")
    {
        const std::string jsonString = series.toJsonString();
        const std::shared_ptr<isx::Series> actual = isx::Series::fromJsonString(jsonString);

        REQUIRE(*actual == series);
    }

    SECTION("A series containing two data sets")
    {
        series.insertDataSet(movie1);
        series.insertDataSet(movie2);

        const std::string jsonString = series.toJsonString();
        const std::shared_ptr<isx::Series> actual = isx::Series::fromJsonString(jsonString);

        REQUIRE(*actual == series);
    }

}
