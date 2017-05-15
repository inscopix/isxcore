#include "isxDataSet.h"
#include "isxTest.h"
#include "isxException.h"
#include "isxGroup.h"
#include "isxProject.h"
#include "isxMovieFactory.h"
#include "isxCellSetFactory.h"
#include "isxVariant.h"
#include "isxPathUtils.h"

#include "catch.hpp"

TEST_CASE("DataSet-DataSet", "[core]")
{
    isx::HistoricalDetails hd("mainTest", "");
    isx::HistoricalDetails hdd("derivedTest", "");

    SECTION("Empty constructor")
    {
        isx::DataSet dataSet;

        REQUIRE(!dataSet.isValid());
    }

    SECTION("Construct a movie data set with no properties")
    {
        isx::DataSet dataSet("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd", hd);

        REQUIRE(dataSet.isValid());
        REQUIRE(dataSet.getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(dataSet.getName() == "myMovie");
        REQUIRE(dataSet.isModified() == false);
        REQUIRE(dataSet.getHistoricalDetails() == hd);
        REQUIRE(dataSet.getProperties() == isx::DataSet::Properties());
    }

    SECTION("Construct a movie data set with some properties")
    {
        isx::DataSet::Properties properties;
        properties["test"] = isx::Variant(1.0f);
        isx::DataSet dataSet("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd", hd, properties);

        REQUIRE(dataSet.isModified() == false);
        isx::Variant value;
        REQUIRE(dataSet.getPropertyValue("test", value));
        REQUIRE(value.value<float>() == 1.0f);
    }

    SECTION("Construct a cell set data set")
    {
        isx::DataSet dataSet("myCellSet", isx::DataSet::Type::CELLSET, "myCellSet.isxd", hdd);

        REQUIRE(dataSet.isValid());
        REQUIRE(dataSet.getType() == isx::DataSet::Type::CELLSET);
        REQUIRE(dataSet.getName() == "myCellSet");
        REQUIRE(dataSet.getHistoricalDetails() == hdd);
        REQUIRE(dataSet.isModified() == false);
    }

}

TEST_CASE("DataSet-setName", "[core]")
{
    isx::HistoricalDetails hd("mainTest", "");
    isx::DataSet dataSet("myDataSet", isx::DataSet::Type::MOVIE, "movie.isxd", hd);

    SECTION("Set the name of a data set")
    {
        dataSet.setName("myMovie");

        REQUIRE(dataSet.getName() == "myMovie");
        REQUIRE(dataSet.isModified() == true);
    }
}

TEST_CASE("readDataSetType", "[core]")
{

    std::string fileName = g_resources["unitTestDataPath"] + "/myDataSet.isxd";
    std::remove(fileName.c_str());

    isx::Time start(2016, 8, 26, 10, 31, 26, isx::DurationInSeconds(117, 1000));
    isx::DurationInSeconds step(50, 1000);
    isx::isize_t numTimes = 5;
    isx::TimingInfo timingInfo(start, step, numTimes);

    isx::SizeInPixels_t numPixels(3, 4);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

    SECTION("Movie")
    {
        auto m(isx::writeMosaicMovie(fileName, timingInfo, spacingInfo, isx::DataType::U16));
        m->closeForWriting();

        isx::DataSet::Type type = isx::readDataSetType(fileName);

        REQUIRE(type == isx::DataSet::Type::MOVIE);
    }

}

TEST_CASE("DataSet-toFromJsonString", "[core]")
{
    isx::HistoricalDetails hd("mainTest", "");
    isx::HistoricalDetails hdd("derivedTest", "");

    SECTION("One data set with some properties")
    {
        isx::DataSet::Properties properties;
        properties["test"] = isx::Variant(1.0f);
        isx::DataSet expected("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd", hd, properties);

        const std::string jsonString = expected.toJsonString();
        std::shared_ptr<isx::DataSet> actual = isx::DataSet::fromJsonString(jsonString);

        REQUIRE(*actual == expected);
    }
}

// Utility functions for the following tests.
namespace
{

isx::SpDataSet_t
makeMovieDataSet(
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
    return std::make_shared<isx::DataSet>(inName, isx::DataSet::Type::MOVIE, inFilePath, history, properties, inImported);
}

std::string
makeDeleteFilesFilePath(const std::string & inName)
{
    return g_resources["unitTestDataPath"] + "/DataSet-deleteFiles-" + inName + ".isxd";
}

} // namespace

TEST_CASE("DataSet-deleteFile", "[core]")
{
    const isx::TimingInfo ti(isx::Time(), isx::DurationInSeconds(50, 1000), 4);
    const isx::SpacingInfo si(isx::SizeInPixels_t(2, 3));
    const std::string filePath = makeDeleteFilesFilePath("movie");

    SECTION("Delete an imported dataset file")
    {
        isx::SpDataSet_t ds = makeMovieDataSet("movie", filePath, ti, si, true);
        ds->deleteFile();
        REQUIRE(isx::pathExists(ds->getFileName()));
    }

    SECTION("Delete an non-imported dataset file")
    {
        isx::SpDataSet_t ds = makeMovieDataSet("movie", filePath, ti, si, false);
        ds->deleteFile();
        REQUIRE(!isx::pathExists(ds->getFileName()));
    }

    std::remove(filePath.c_str());
}
