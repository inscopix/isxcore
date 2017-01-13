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

    SECTION("Empty constructor")
    {
        isx::DataSet dataSet;

        REQUIRE(!dataSet.isValid());
    }

    SECTION("Construct a movie data set with no properties")
    {
        isx::DataSet dataSet("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd");

        REQUIRE(dataSet.isValid());
        REQUIRE(dataSet.getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(dataSet.getName() == "myMovie");
        REQUIRE(dataSet.getParent() == nullptr);
        REQUIRE(dataSet.getPath() == "myMovie");
        REQUIRE(dataSet.isModified() == false);
        REQUIRE(dataSet.getNumChildren() == 0);
        REQUIRE(dataSet.getProperties() == isx::DataSet::Properties());
    }

    SECTION("Construct a movie data set with some properties")
    {
        isx::DataSet::Properties properties;
        properties["test"] = isx::Variant(1.0f);
        isx::DataSet dataSet("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd", properties);

        REQUIRE(dataSet.isModified() == false);
        isx::Variant value;
        REQUIRE(dataSet.getPropertyValue("test", value));
        REQUIRE(value.value<float>() == 1.0f);
    }

    SECTION("Construct a cell set data set")
    {
        isx::DataSet dataSet("myCellSet", isx::DataSet::Type::CELLSET, "myCellSet.isxd");

        REQUIRE(dataSet.isValid());
        REQUIRE(dataSet.getType() == isx::DataSet::Type::CELLSET);
        REQUIRE(dataSet.getName() == "myCellSet");
        REQUIRE(dataSet.getParent() == nullptr);
        REQUIRE(dataSet.getPath() == "myCellSet");
        REQUIRE(dataSet.isModified() == false);
    }

}

TEST_CASE("DataSet-insertDerivedDataSet", "[core]")
{
    isx::DataSet movie("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd");

    SECTION("Insert a derived cellset data set")
    {
        auto cellSet = std::make_shared<isx::DataSet>("myCellSet", isx::DataSet::Type::CELLSET, "myCellSet.isxd");

        movie.insertDerivedDataSet(cellSet);

        REQUIRE(movie.getChild("myCellSet") == cellSet.get());
        REQUIRE(movie.isModified() == true);
    }

    SECTION("Insert the same derived cellset data set into two movies")
    {
        isx::DataSet movie2("myMovie2", isx::DataSet::Type::MOVIE, "myMovie.isxd");

        auto cellSet = std::make_shared<isx::DataSet>("myCellSet", isx::DataSet::Type::CELLSET, "myCellSet.isxd");

        movie.insertDerivedDataSet(cellSet);
        movie2.insertDerivedDataSet(cellSet);

        ISX_REQUIRE_EXCEPTION(
                movie.getChild("myCellSet"),
                isx::ExceptionDataIO,
                "Could not find child with the name: myCellSet");

        REQUIRE(movie2.getChild("myCellSet") == cellSet.get());
    }

    SECTION("Insert two derived cellset data sets")
    {
        auto pcaIca = std::make_shared<isx::DataSet>("pcaIca", isx::DataSet::Type::CELLSET, "pcaIca.isxd");
        auto rois = std::make_shared<isx::DataSet>("rois", isx::DataSet::Type::CELLSET, "rois.isxd");

        movie.insertDerivedDataSet(pcaIca);
        movie.insertDerivedDataSet(rois);

        REQUIRE(movie.getChild("pcaIca") == pcaIca.get());
        REQUIRE(movie.getChild("rois") == rois.get());
        REQUIRE(movie.isModified() == true);
    }

    SECTION("Try to insert a data set in itself")
    {
        auto movie2 = std::make_shared<isx::DataSet>("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd");
        ISX_REQUIRE_EXCEPTION(
                movie2->insertDerivedDataSet(movie2),
                isx::ExceptionDataIO,
                "An item cannot be inserted in itself.");
    }

    SECTION("Try to insert an ancestor of a data set")
    {
        auto movie2 = std::make_shared<isx::DataSet>("movie2", isx::DataSet::Type::MOVIE, "movie2.isxd");
        auto movie3 = std::make_shared<isx::DataSet>("movie3", isx::DataSet::Type::MOVIE, "movie3.isxd");
        movie2->insertDerivedDataSet(movie3);
        ISX_REQUIRE_EXCEPTION(
                movie3->insertDerivedDataSet(movie2),
                isx::ExceptionDataIO,
                "The inserted item is an ancestor of this.");
    }

}

TEST_CASE("DataSet-removeDerivedDataSet", "[core]")
{
    isx::DataSet movie("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd");
    auto pcaIca = std::make_shared<isx::DataSet>("pcaIca", isx::DataSet::Type::CELLSET, "pcaIca.isxd");
    auto rois = std::make_shared<isx::DataSet>("rois", isx::DataSet::Type::CELLSET, "rois.isxd");

    movie.insertDerivedDataSet(pcaIca);
    movie.insertDerivedDataSet(rois);

    SECTION("Remove a derived cellset data set")
    {
        movie.removeDerivedDataSet("pcaIca");

        REQUIRE(movie.getChild("rois") == rois.get());
        REQUIRE(movie.isModified() == true);
        ISX_REQUIRE_EXCEPTION(
                movie.getChild("pcaIca"),
                isx::ExceptionDataIO,
                "Could not find child with the name: pcaIca");
    }

}

TEST_CASE("DataSet-setParent", "[core]")
{
    isx::Group group("myGroup");
    isx::Series series("mySeries");
    isx::DataSet dataSet("myDataSet", isx::DataSet::Type::MOVIE, "myDataSet.isxd");
    isx::DataSet movie("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd");

    SECTION("Set the parent to be null")
    {
        movie.setParent(nullptr);

        REQUIRE(movie.getParent() == nullptr);
        REQUIRE(movie.getPath() == "myMovie");
        REQUIRE(movie.isModified() == true);
    }

    SECTION("Set the parent to be a group")
    {
        movie.setParent(&group);

        REQUIRE(movie.getParent() != nullptr);
        REQUIRE(movie.getParent() == &group);
        REQUIRE(movie.getPath() == "myGroup/myMovie");
        REQUIRE(movie.isModified() == true);
    }

    SECTION("Set the parent to be a series")
    {
        movie.setParent(&series);

        REQUIRE(movie.getParent() != nullptr);
        REQUIRE(movie.getParent() == &series);
        REQUIRE(movie.getPath() == "mySeries/myMovie");
        REQUIRE(movie.isModified() == true);
    }

    SECTION("Set the parent to be a dataset")
    {
        movie.setParent(&dataSet);

        REQUIRE(movie.getParent() != nullptr);
        REQUIRE(movie.getParent() == &dataSet);
        REQUIRE(movie.getPath() == "myDataSet/myMovie");
        REQUIRE(movie.isModified() == true);
    }
}

TEST_CASE("DataSet-setName", "[core]")
{
    isx::DataSet dataSet("myDataSet", isx::DataSet::Type::MOVIE, "movie.isxd");

    SECTION("Set the name of a data set")
    {
        dataSet.setName("myMovie");

        REQUIRE(dataSet.getName() == "myMovie");
        REQUIRE(dataSet.getPath() == "myMovie");
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
        isx::writeMosaicMovie(fileName, timingInfo, spacingInfo, isx::DataType::U16);

        isx::DataSet::Type type = isx::readDataSetType(fileName);

        REQUIRE(type == isx::DataSet::Type::MOVIE);
    }

}

TEST_CASE("DataSet-toFromJsonString", "[core]")
{

    SECTION("One data set with some properties")
    {
        isx::DataSet::Properties properties;
        properties["test"] = isx::Variant(1.0f);
        isx::DataSet expected("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd", properties);

        const std::string jsonString = expected.toJsonString();
        std::shared_ptr<isx::DataSet> actual = isx::DataSet::fromJsonString(jsonString);

        REQUIRE(*actual == expected);
    }

    SECTION("One data set with a derived data set")
    {
        isx::DataSet expected("movie", isx::DataSet::Type::MOVIE, "movie.isxd");
        auto derivedDataSet = std::make_shared<isx::DataSet>("cellSet", isx::DataSet::Type::CELLSET, "cellSet.isxd");
        expected.insertDerivedDataSet(derivedDataSet);

        const std::string jsonString = expected.toJsonString();
        std::shared_ptr<isx::DataSet> actual = isx::DataSet::fromJsonString(jsonString);

        REQUIRE(*actual == expected);
    }
}

TEST_CASE("DataSetToFromJson", "[core]")
{
    const std::string dataDir = g_resources["unitTestDataPath"];
    const std::string fileName = dataDir + "/testProject.isxp";
    std::remove(fileName.c_str());

    isx::Project project(fileName, "test project");

    const std::string dsName = "myMovie";
    const std::string dsPath = "/" + dsName;
    const isx::DataSet::Type dsType = isx::DataSet::Type::MOVIE;
    const std::string dsFileNameInput = "myMovie.isxd";
    const std::string propKey = "test";
    const isx::Variant propValue(1.f);
    isx::DataSet::Properties properties;
    properties[propKey] = propValue;

    const std::string dsNameD = "myCellSetD";
    const std::string dsPathD = dsPath + "/" + dsNameD;
    const isx::DataSet::Type dsTypeD = isx::DataSet::Type::CELLSET;
    const std::string dsFileNameDInput = "myCellSetD.isxd";

    isx::DataSet * dataSet = project.createDataSet(
        dsPath,
        dsType,
        dsFileNameInput,
        properties);
    auto derivedDs = std::make_shared<isx::DataSet>(dsNameD, dsTypeD, dsFileNameDInput);
    dataSet->insertDerivedDataSet(derivedDs);

    std::vector<const isx::DataSet *> dataSets{static_cast<isx::DataSet *>(project.getItem(dsPath))};
    std::vector<const isx::DataSet *> derivedDataSets{static_cast<isx::DataSet *>(project.getItem(dsPathD))};
    
    const std::string dsFileName = dataSets[0]->getFileName();
    const std::string dsFileNameD = derivedDataSets[0]->getFileName();

    SECTION("Original only")
    {
        std::vector<const isx::DataSet *> deriveds{nullptr};
        const std::string js = isx::DataSet::toJsonString(dsPath, dataSets, deriveds);

        std::string path;
        std::vector<isx::DataSet> ds;
        std::vector<isx::DataSet> dds;
        isx::DataSet::fromJsonString(js, path, ds, dds);
        REQUIRE(ds.size() == 1);
        REQUIRE(dds.size() == 0);

        const isx::DataSet::Properties & props = ds[0].getProperties();
        REQUIRE(path == dsPath);
        REQUIRE(ds[0].getName() == dsName);
        REQUIRE(ds[0].getType() == dsType);
        REQUIRE(ds[0].getFileName() == dsFileName);
        REQUIRE(props.size() == 1);
        REQUIRE(props.find(propKey) != props.end());
        REQUIRE(props.at(propKey) == propValue);
    }

    SECTION("Original and derived")
    {
        const std::string js = isx::DataSet::toJsonString(dsPath, dataSets, derivedDataSets);

        std::string path;
        std::vector<isx::DataSet> ds;
        std::vector<isx::DataSet> dds;
        isx::DataSet::fromJsonString(js, path, ds, dds);
        REQUIRE(ds.size() == 1);
        REQUIRE(dds.size() == 1);
        const isx::DataSet::Properties & props = ds[0].getProperties();

        // Original
        REQUIRE(path == dsPath);
        REQUIRE(ds[0].getName() == dsName);
        REQUIRE(ds[0].getType() == dsType);
        REQUIRE(ds[0].getFileName() == dsFileName);
        REQUIRE(props.size() == 1);
        REQUIRE(props.find(propKey) != props.end());
        REQUIRE(props.at(propKey) == propValue);

        // Derived
        REQUIRE(dds[0].getName() == dsNameD);
        REQUIRE(dds[0].getType() == dsTypeD);
        REQUIRE(dds[0].getFileName() == dsFileNameD);
        const isx::DataSet::Properties & props_dds = dds[0].getProperties();
        REQUIRE(props_dds.size() == 0);
    }
}

TEST_CASE("DataSetSeriesToFromJson", "[core]")
{
    const std::string dataDir = g_resources["unitTestDataPath"];
    const std::string fileName = dataDir + "/testProject.isxp";
    std::remove(fileName.c_str());

    isx::Project project(fileName, "test project");

    const std::string seriesName = "mySeries";
    const std::string seriesPath = "/" + seriesName;
    isx::Series * series = project.createSeries(seriesPath);

    // Movie 0
    const std::string dsName0 = "myMovie0";
    const std::string dsPath0 = seriesPath + "/" + dsName0;
    const isx::DataSet::Type dsType0 = isx::DataSet::Type::MOVIE;
    const std::string dsFileNameInput0 = g_resources["unitTestDataPath"] + "/myMovie0.isxd";
    const std::string propKey0 = "test0";
    const isx::Variant propValue0(1.f);
    isx::DataSet::Properties properties0;
    properties0[propKey0] = propValue0;

    const isx::TimingInfo timingInfo0(isx::Time(2016, 11, 8, 9, 24, 55), isx::DurationInSeconds(50, 1000), 5);
    const isx::SpacingInfo spacingInfo(isx::SizeInPixels_t(3, 4));
    std::remove(dsFileNameInput0.c_str());
    isx::writeMosaicMovie(dsFileNameInput0, timingInfo0, spacingInfo, isx::DataType::U16);

    const isx::DataSet * ds0 = project.createDataSet(
            dsPath0,
            dsType0,
            dsFileNameInput0,
            properties0);

    // Derived CellSet 0
    const std::string dsNameD0 = "myCellSetD0";
    const std::string dsPathD0 = dsPath0 + "/" + dsNameD0;
    const isx::DataSet::Type dsTypeD0 = isx::DataSet::Type::CELLSET;
    const std::string dsFileNameDInput0 = g_resources["unitTestDataPath"] + "/myCellSetD0.isxd";

    std::remove(dsFileNameDInput0.c_str());
    isx::writeCellSet(dsFileNameDInput0, timingInfo0, spacingInfo);

    const isx::DataSet * dds0 = project.createDataSet(
            dsPathD0,
            dsTypeD0,
            dsFileNameDInput0);

    // Movie 1
    const std::string dsName1 = "myMovie1";
    const std::string dsPath1 = seriesPath + "/" + dsName1;
    const isx::DataSet::Type dsType1 = isx::DataSet::Type::MOVIE;
    const std::string dsFileNameInput1 = g_resources["unitTestDataPath"] + "/myMovie1.isxd";
    const std::string propKey1 = "test1";
    const isx::Variant propValue1(1.1f);
    isx::DataSet::Properties properties1;
    properties1[propKey1] = propValue1;

    const isx::TimingInfo timingInfo1(isx::Time(2016, 11, 8, 9, 34, 55), isx::DurationInSeconds(50, 1000), 5);
    std::remove(dsFileNameInput1.c_str());
    isx::writeMosaicMovie(dsFileNameInput1, timingInfo1, spacingInfo, isx::DataType::U16);

    const isx::DataSet * ds1 = project.createDataSet(
            dsPath1,
            dsType1,
            dsFileNameInput1,
            properties1);

    // Derived CellSet 1
    const std::string dsNameD1 = "myCellSetD1";
    const std::string dsPathD1 = dsPath1 + "/" + dsNameD1;
    const isx::DataSet::Type dsTypeD1 = isx::DataSet::Type::CELLSET;
    const std::string dsFileNameDInput1 = g_resources["unitTestDataPath"] + "/myCellSetD1.isxd";

    std::remove(dsFileNameDInput1.c_str());
    isx::writeCellSet(dsFileNameDInput1, timingInfo1, spacingInfo);

    const isx::DataSet * dds1 = project.createDataSet(
            dsPathD1,
            dsTypeD1,
            dsFileNameDInput1);

    std::vector<const isx::DataSet *> dataSets{ds0, ds1};
    std::vector<const isx::DataSet *> derivedDataSets{dds0, dds1};
    
    const std::string dsFileName0 = dataSets[0]->getFileName();
    const std::string dsFileName1 = dataSets[1]->getFileName();
    const std::string dsFileNameD0 = derivedDataSets[0]->getFileName();
    const std::string dsFileNameD1 = derivedDataSets[1]->getFileName();

    SECTION("Original only")
    {
        std::vector<const isx::DataSet *> deriveds{nullptr, nullptr};
        std::string js = isx::DataSet::toJsonString(seriesPath, dataSets, deriveds);

        std::string path;
        std::vector<isx::DataSet> ds;
        std::vector<isx::DataSet> dds;
        isx::DataSet::fromJsonString(js, path, ds, dds);
        REQUIRE(ds.size() == 2);
        REQUIRE(dds.size() == 0);
        
        const isx::DataSet::Properties & props0 = ds[0].getProperties();
        REQUIRE(path == seriesPath);
        REQUIRE(ds[0].getName() == dsName0);
        REQUIRE(ds[0].getType() == dsType0);
        REQUIRE(ds[0].getFileName() == dsFileName0);
        REQUIRE(props0.size() == 1);
        REQUIRE(props0.find(propKey0) != props0.end());
        REQUIRE(props0.at(propKey0) == propValue0);
        
        const isx::DataSet::Properties & props1 = ds[1].getProperties();
        REQUIRE(ds[1].getName() == dsName1);
        REQUIRE(ds[1].getType() == dsType1);
        REQUIRE(ds[1].getFileName() == dsFileName1);
        REQUIRE(props1.size() == 1);
        REQUIRE(props1.find(propKey1) != props1.end());
        REQUIRE(props1.at(propKey1) == propValue1);
    }

    SECTION("Original and derived")
    {
        std::string js = isx::DataSet::toJsonString(seriesPath, dataSets, derivedDataSets);

        std::string path;
        std::vector<isx::DataSet> ds;
        std::vector<isx::DataSet> dds;
        isx::DataSet::fromJsonString(js, path, ds, dds);
        REQUIRE(ds.size() == 2);
        REQUIRE(dds.size() == 2);
        
        // Original 0
        const isx::DataSet::Properties & props0 = ds[0].getProperties();
        REQUIRE(path == seriesPath);
        REQUIRE(ds[0].getName() == dsName0);
        REQUIRE(ds[0].getType() == dsType0);
        REQUIRE(ds[0].getFileName() == dsFileName0);
        REQUIRE(props0.size() == 1);
        REQUIRE(props0.find(propKey0) != props0.end());
        REQUIRE(props0.at(propKey0) == propValue0);

        // Derived 0
        REQUIRE(dds[0].getName() == dsNameD0);
        REQUIRE(dds[0].getType() == dsTypeD0);
        REQUIRE(dds[0].getFileName() == dsFileNameD0);
        const isx::DataSet::Properties & props_dds0 = dds[0].getProperties();
        REQUIRE(props_dds0.size() == 0);
        
        // Original 1
        const isx::DataSet::Properties & props1 = ds[1].getProperties();
        REQUIRE(ds[1].getName() == dsName1);
        REQUIRE(ds[1].getType() == dsType1);
        REQUIRE(ds[1].getFileName() == dsFileName1);
        REQUIRE(props1.size() == 1);
        REQUIRE(props1.find(propKey1) != props1.end());
        REQUIRE(props1.at(propKey1) == propValue1);
        
        // Derived 1
        REQUIRE(dds[1].getName() == dsNameD1);
        REQUIRE(dds[1].getType() == dsTypeD1);
        REQUIRE(dds[1].getFileName() == dsFileNameD1);
        const isx::DataSet::Properties & props_dds1 = dds[1].getProperties();
        REQUIRE(props_dds1.size() == 0);
    }
}
