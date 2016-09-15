#include "isxDataSet.h"
#include "isxTest.h"
#include "isxException.h"
#include "isxGroup.h"
#include "isxMovieFactory.h"

#include "catch.hpp"

TEST_CASE("DataSetTest", "[core]")
{

    SECTION("Empty constructor")
    {
        isx::DataSet dataSet;

        REQUIRE(!dataSet.isValid());
    }

    SECTION("Construct a movie data set")
    {
        std::map<std::string, float> properties;
        properties["test"] = 1.0f;
        isx::DataSet dataSet("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd", properties);

        REQUIRE(dataSet.isValid());
        REQUIRE(dataSet.getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(dataSet.getName() == "myMovie");
        REQUIRE(!dataSet.getParent());
        REQUIRE(dataSet.getPath() == "myMovie");
        float value;
        REQUIRE(dataSet.getPropertyValue("test", value));
        REQUIRE(value == 1.0f);
    }

    SECTION("Construct a movie data set then set parent group")
    {
        isx::DataSet dataSet("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd");
        isx::Group group("myGroup");

        dataSet.setParent(&group);

        REQUIRE(dataSet.getParent());
        REQUIRE(dataSet.getParent() == &group);
        REQUIRE(dataSet.getPath() == "myGroup/myMovie");
    }

    SECTION("Construct a cell set data set")
    {
        isx::DataSet dataSet("myCellSet", isx::DataSet::Type::CELLSET, "myCellSet.isxd");

        REQUIRE(dataSet.isValid());
        REQUIRE(dataSet.getType() == isx::DataSet::Type::CELLSET);
        REQUIRE(dataSet.getName() == "myCellSet");
        REQUIRE(!dataSet.getParent());
        REQUIRE(dataSet.getPath() == "myCellSet");
    }

    SECTION("Construct a movie data set then set parent group")
    {
        isx::DataSet dataSet("myCellSet", isx::DataSet::Type::CELLSET, "myCellSet.isxd");
        isx::Group group("myGroup");

        dataSet.setParent(&group);

        REQUIRE(dataSet.getParent());
        REQUIRE(dataSet.getParent() == &group);
        REQUIRE(dataSet.getPath() == "myGroup/myCellSet");
    }

}

TEST_CASE("readDataSetTypeTest", "[core]")
{

    std::string fileName = g_resources["testDataPath"] + "/myDataSet.isxd";
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
        isx::writeMosaicMovie(fileName, timingInfo, spacingInfo, isx::DataType::U16);

        isx::DataSet::Type type = isx::readDataSetType(fileName);

        REQUIRE(type == isx::DataSet::Type::MOVIE);
    }

}
