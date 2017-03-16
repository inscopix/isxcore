#include "isxDataSet.h"
#include "isxTest.h"
#include "isxException.h"
#include "isxGroup.h"
#include "isxProject.h"
#include "isxMovieFactory.h"
#include "isxCellSetFactory.h"
#include "isxVariant.h"

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

TEST_CASE("DataSetToFromJson", "[core]")
{
    const std::string expPath = "this is a DatatSet path";
    const std::string expDerivedPath = "this is a DatatSet path for a derived DataSet";
    const std::string expTitle = "this is a title";
    isx::HistoricalDetails hd("mainTest", "");
    isx::HistoricalDetails hdd("derivedTest", "");
    
    const std::string expPropKey = "test";
    const isx::Variant expPropValue(1.f);
    isx::DataSet::Properties expProperties;
    expProperties[expPropKey] = expPropValue;
    
    auto expDs = std::make_shared<isx::DataSet>(
        "datasetName",
        isx::DataSet::Type::MOVIE,
        "datasetFilename",
        hd,
        expProperties);
    auto expDds = std::make_shared<isx::DataSet>(
        "derivedDatasetName",
        isx::DataSet::Type::CELLSET,
        "derivedDatasetFilename",
        hdd,
        expProperties);
    
    std::vector<const isx::DataSet *> dataSets{expDs.get()};
    std::vector<const isx::DataSet *> derivedDataSets{expDds.get()};

    SECTION("Original only")
    {
        std::vector<const isx::DataSet *> deriveds{nullptr};
        const std::string js = isx::DataSet::toJsonString(
            expPath,
            "",
            expTitle,
            dataSets,
            deriveds);

        std::string path;
        std::string derivedPath;
        std::string title;
        std::vector<isx::DataSet> ds;
        std::vector<isx::DataSet> dds;
        isx::DataSet::fromJsonString(js, path, derivedPath, title, ds, dds);
        REQUIRE(ds.size() == 1);
        REQUIRE(dds.size() == 0);
        
        const isx::DataSet::Properties & props = ds[0].getProperties();
        REQUIRE(path == expPath);
        REQUIRE(title == expTitle);
        REQUIRE(ds[0].getName() == expDs->getName());
        REQUIRE(ds[0].getType() == expDs->getType());
        REQUIRE(ds[0].getFileName() == expDs->getFileName());
        REQUIRE(ds[0].getHistoricalDetails() == hd);
        REQUIRE(props.size() == 1);
        REQUIRE(props.find(expPropKey) != props.end());
        REQUIRE(props.at(expPropKey) == expPropValue);
    }

    SECTION("Original and derived")
    {
        const std::string js = isx::DataSet::toJsonString(
            expPath,
            expDerivedPath,
            expTitle,
            dataSets,
            derivedDataSets);

        std::string path;
        std::string derivedPath;
        std::string title;
        std::vector<isx::DataSet> ds;
        std::vector<isx::DataSet> dds;
        isx::DataSet::fromJsonString(js, path, derivedPath, title, ds, dds);
        REQUIRE(ds.size() == 1);
        REQUIRE(dds.size() == 1);
        const isx::DataSet::Properties & props = ds[0].getProperties();

        // Original
        REQUIRE(path == expPath);
        REQUIRE(title == expTitle);
        REQUIRE(ds[0].getName() == expDs->getName());
        REQUIRE(ds[0].getType() == expDs->getType());
        REQUIRE(ds[0].getFileName() == expDs->getFileName());
        REQUIRE(ds[0].getHistoricalDetails() == hd);
        REQUIRE(props.size() == 1);
        REQUIRE(props.find(expPropKey) != props.end());
        REQUIRE(props.at(expPropKey) == expPropValue);

        // Derived
        REQUIRE(dds[0].getName() == expDds->getName());
        REQUIRE(dds[0].getType() == expDds->getType());
        REQUIRE(dds[0].getFileName() == expDds->getFileName());
        REQUIRE(dds[0].getHistoricalDetails() == hdd);
        const isx::DataSet::Properties & props_dds = dds[0].getProperties();
        REQUIRE(props_dds.size() == 1);
        REQUIRE(props_dds.find(expPropKey) != props_dds.end());
        REQUIRE(props_dds.at(expPropKey) == expPropValue);
    }
}

TEST_CASE("DataSetSeriesToFromJson", "[core]")
{
    const std::string expPath = "this is a DatatSet path";
    const std::string expDerivedPath = "this is a DatatSet path for a derived DataSet";
    const std::string expTitle = "this is a title";
    isx::HistoricalDetails hd("mainTest", "");
    isx::HistoricalDetails hdd("derivedTest", "");

    const std::string expPropKey = "test";
    const isx::Variant expPropValue(1.f);
    isx::DataSet::Properties expProperties;
    expProperties[expPropKey] = expPropValue;

    const std::vector<isx::SpDataSet_t> expDss =
    { std::make_shared<isx::DataSet>(
        "datasetName 0",
        isx::DataSet::Type::MOVIE,
        "datasetFilename 0",
        hd,
        expProperties),
      std::make_shared<isx::DataSet>(
        "datasetName 1",
        isx::DataSet::Type::MOVIE,
        "datasetFilename 1",
        hd,
        expProperties) };
    
    const std::vector<isx::SpDataSet_t> expDdss =
    { std::make_shared<isx::DataSet>(
        "derivedDatasetName 0",
        isx::DataSet::Type::MOVIE,
        "datasetFilename 0",
        hdd,
        expProperties),
      std::make_shared<isx::DataSet>(
        "datasetName 1",
        isx::DataSet::Type::MOVIE,
        "derivedDatasetFilename 1",
        hdd,
        expProperties) };
    
    std::vector<const isx::DataSet *> dataSets{expDss[0].get(), expDss[1].get()};
    std::vector<const isx::DataSet *> derivedDataSets{expDdss[0].get(), expDdss[1].get()};
  
    SECTION("Original only")
    {
        std::vector<const isx::DataSet *> deriveds{nullptr, nullptr};
        const std::string js = isx::DataSet::toJsonString(
            expPath,
            "",
            expTitle,
            dataSets,
            deriveds);
        
        std::string path;
        std::string derivedPath;
        std::string title;
        std::vector<isx::DataSet> ds;
        std::vector<isx::DataSet> dds;
        isx::DataSet::fromJsonString(js, path, derivedPath, title, ds, dds);
        REQUIRE(ds.size() == 2);
        REQUIRE(dds.size() == 0);
        REQUIRE(path == expPath);
        REQUIRE(title == expTitle);
        for (uint64_t i = 0; i < 2; ++i)
        {
            REQUIRE(ds[i].getName() == expDss[i]->getName());
            REQUIRE(ds[i].getType() == expDss[i]->getType());
            REQUIRE(ds[i].getFileName() == expDss[i]->getFileName());
            REQUIRE(ds[i].getHistoricalDetails() == hd);
            const isx::DataSet::Properties & props = ds[i].getProperties();
            REQUIRE(props.size() == 1);
            REQUIRE(props.find(expPropKey) != props.end());
            REQUIRE(props.at(expPropKey) == expPropValue);
        }
    }
    
    SECTION("Original and derived")
    {
        const std::string js = isx::DataSet::toJsonString(
            expPath,
            expDerivedPath,
            expTitle,
            dataSets,
            derivedDataSets);
        
        std::string path;
        std::string derivedPath;
        std::string title;
        std::vector<isx::DataSet> ds;
        std::vector<isx::DataSet> dds;
        isx::DataSet::fromJsonString(js, path, derivedPath, title, ds, dds);
        REQUIRE(ds.size() == 2);
        REQUIRE(dds.size() == 2);
        REQUIRE(path == expPath);
        REQUIRE(derivedPath == expDerivedPath);
        REQUIRE(title == expTitle);
        for (uint64_t i = 0; i < 2; ++i)
        {
            REQUIRE(ds[i].getName() == expDss[i]->getName());
            REQUIRE(ds[i].getType() == expDss[i]->getType());
            REQUIRE(ds[i].getFileName() == expDss[i]->getFileName());
            REQUIRE(ds[i].getHistoricalDetails() == hd);
            const isx::DataSet::Properties & props = ds[i].getProperties();
            REQUIRE(props.size() == 1);
            REQUIRE(props.find(expPropKey) != props.end());
            REQUIRE(props.at(expPropKey) == expPropValue);
        }
        for (uint64_t i = 0; i < 2; ++i)
        {
            REQUIRE(dds[i].getName() == expDdss[i]->getName());
            REQUIRE(dds[i].getType() == expDdss[i]->getType());
            REQUIRE(dds[i].getFileName() == expDdss[i]->getFileName());
            REQUIRE(dds[i].getHistoricalDetails() == hdd);
            const isx::DataSet::Properties & props = dds[i].getProperties();
            REQUIRE(props.size() == 1);
            REQUIRE(props.find(expPropKey) != props.end());
            REQUIRE(props.at(expPropKey) == expPropValue);
        }
    }
}

