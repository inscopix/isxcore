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
        isx::DataSet::Properties properties;
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
        isx::writeMosaicMovie(fileName, timingInfo, spacingInfo, isx::DataType::U16);

        isx::DataSet::Type type = isx::readDataSetType(fileName);

        REQUIRE(type == isx::DataSet::Type::MOVIE);
    }

}

TEST_CASE("DataSetToFromJson", "[core]")
{
    const std::string dsName = "myMovie";
    const isx::DataSet::Type dsType = isx::DataSet::Type::MOVIE;
    const std::string dsFileName = "myMovie.isxd";
    const std::string propKey = "test";
    const float propValue = 1.f;
    isx::DataSet::Properties properties;
    properties[propKey] = propValue;
    isx::DataSet dataSet(dsName, dsType, dsFileName, properties);

    const std::string dsNameD = "myMovieD";
    const std::string dsFileNameD = "myMovieD.isxd";
    isx::DataSet dataSetDerived(dsNameD, dsType, dsFileNameD, properties);

    isx::Group group("myGroup");
    dataSet.setParent(&group);
    dataSetDerived.setParent(&group);

    const std::string expected = "{\"original\":{\"dataset\":{\"dataSetType\":0,\"fileName\":\"myMovie.isxd\",\"name\":\"myMovie\",\"properties\":{\"test\":1},\"type\":\"DataSet\"},\"path\":\"myGroup\"}}";
    const std::string derived_expected = "{\"derived\":{\"dataset\":{\"dataSetType\":0,\"fileName\":\"myMovieD.isxd\",\"name\":\"myMovieD\",\"properties\":{\"test\":1},\"type\":\"DataSet\"},\"path\":\"myGroup\"},\"original\":{\"dataset\":{\"dataSetType\":0,\"fileName\":\"myMovie.isxd\",\"name\":\"myMovie\",\"properties\":{\"test\":1},\"type\":\"DataSet\"},\"path\":\"myGroup\"}}";

    SECTION("ToJson - original only")
    {
        std::string js = isx::DataSet::toJsonString(&dataSet);
        REQUIRE(js == expected);
    }
    
    SECTION("FromJson - original only")
    {
        std::string path;
        isx::DataSet ds;
        isx::DataSet dds;
        isx::DataSet::fromJsonString(expected, path, ds, dds);
        const isx::DataSet::Properties & props = ds.getProperties();
        REQUIRE(path == "myGroup/myMovie");
        REQUIRE(ds.getName() == dsName);
        REQUIRE(ds.getType() == dsType);
        REQUIRE(ds.getFileName() == dsFileName);
        REQUIRE(props.size() == 1);
        REQUIRE(props.find(propKey) != props.end());
        REQUIRE(props.at(propKey) == propValue);
    }

    SECTION("ToJson - original and derived")
    {
        std::string js = isx::DataSet::toJsonString(&dataSet, &dataSetDerived);
        REQUIRE(js == derived_expected);
    }
    
    SECTION("FromJson - original and derived")
    {
        std::string path;
        isx::DataSet ds;
        isx::DataSet dds;
        isx::DataSet::fromJsonString(derived_expected, path, ds, dds);
        const isx::DataSet::Properties & props = ds.getProperties();
        
        // Original
        REQUIRE(path == "myGroup");
        REQUIRE(ds.getName() == dsName);
        REQUIRE(ds.getType() == dsType);
        REQUIRE(ds.getFileName() == dsFileName);
        REQUIRE(props.size() == 1);
        REQUIRE(props.find(propKey) != props.end());
        REQUIRE(props.at(propKey) == propValue);

        // Derived        
        REQUIRE(dds.getName() == dsNameD);
        REQUIRE(dds.getType() == dsType);
        REQUIRE(dds.getFileName() == dsFileNameD);
        const isx::DataSet::Properties & props_dds = dds.getProperties();
        REQUIRE(props_dds.size() == 1);
        REQUIRE(props_dds.find(propKey) != props_dds.end());
        REQUIRE(props_dds.at(propKey) == propValue);
    }
    
}

