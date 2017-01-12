#include "isxDataSet.h"
#include "isxTest.h"
#include "isxException.h"
#include "isxGroup.h"
#include "isxProject.h"
#include "isxMovieFactory.h"
#include "isxVariant.h"

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
        properties["test"] = isx::Variant(1.0f);
        isx::DataSet dataSet("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd", properties);

        REQUIRE(dataSet.isValid());
        REQUIRE(dataSet.getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(dataSet.getName() == "myMovie");
        REQUIRE(!dataSet.getParent());
        REQUIRE(dataSet.getPath() == "myMovie");
        isx::Variant value;
        REQUIRE(dataSet.getPropertyValue("test", value));
        REQUIRE(value.value<float>() == 1.0f);
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
    const std::string dataDir = g_resources["unitTestDataPath"];
    const std::string fileName = dataDir + "/testProject.isxp";
    std::remove(fileName.c_str());

    isx::Project project(fileName, "test project");

    const std::string groupName = "/myGroup";
    const std::string dsName = "myMovie";
    const std::string dsPath = groupName + "/" + dsName;
    const isx::DataSet::Type dsType = isx::DataSet::Type::MOVIE;
    const std::string dsFileNameInput = "myMovie.isxd";
    const std::string propKey = "test";
    const isx::Variant propValue(1.f);
    isx::DataSet::Properties properties;
    properties[propKey] = propValue;

    const std::string dsNameD = "myCellSetD";
    const std::string dsPathD = dsPath + "/derived/" + dsNameD;
    const isx::DataSet::Type dsTypeD = isx::DataSet::Type::CELLSET;
    const std::string dsFileNameDInput = "myCellSetD.isxd";

    project.createGroup(groupName);
    project.createDataSet(
        dsPath,
        dsType,
        dsFileNameInput,
        properties);
    project.createDataSet(
        dsPathD,
        dsTypeD,
        dsFileNameDInput);

    std::vector<isx::DataSet *> dataSets{project.getDataSet(dsPath)};
    std::vector<isx::DataSet *> derivedDataSets{project.getDataSet(dsPathD)};
    
    const std::string dsFileName = dataSets[0]->getFileName();
    const std::string dsFileNameD = derivedDataSets[0]->getFileName();

    const std::string expected =
        "{\"dataSets\":[{\"original\":{\"dataSetType\":0,\"fileName\":\""
        + dsFileName
        + "\",\"name\":\"myMovie\",\"properties\":{\"test\":1},\"type\":\"DataSet\"}}],\"path\":\"/myGroup/myMovie\"}";
    const std::string derived_expected = "{\"dataSets\":[{\"derived\":{\"dataSetType\":1,\"fileName\":\""
        + dsFileNameD
        + "\",\"name\":\"myCellSetD\",\"properties\":{},\"type\":\"DataSet\"},\"original\":{\"dataSetType\":0,\"fileName\":\""
        + dsFileName
        + "\",\"name\":\"myMovie\",\"properties\":{\"test\":1},\"type\":\"DataSet\"}}],\"path\":\"/myGroup/myMovie\"}";
    
    SECTION("ToJson - original only")
    {
        std::vector<isx::DataSet *> deriveds{nullptr};
        std::string js = isx::DataSet::toJsonString(dsPath, dataSets, deriveds);
        REQUIRE(js == expected);
    }
    
    SECTION("FromJson - original only")
    {
        std::string path;
        std::vector<isx::DataSet> ds;
        std::vector<isx::DataSet> dds;
        isx::DataSet::fromJsonString(expected, path, ds, dds);
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

    SECTION("ToJson - original and derived")
    {
        std::string js = isx::DataSet::toJsonString(dsPath, dataSets, derivedDataSets);
        REQUIRE(js == derived_expected); 
    }

    SECTION("FromJson - original and derived")
    {
        std::string path;
        std::vector<isx::DataSet> ds;
        std::vector<isx::DataSet> dds;
        isx::DataSet::fromJsonString(derived_expected, path, ds, dds);
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

TEST_CASE("DataSetGroupToFromJson", "[core]")
{
    const std::string dataDir = g_resources["unitTestDataPath"];
    const std::string fileName = dataDir + "/testProject.isxp";
    std::remove(fileName.c_str());

    isx::Project project(fileName, "test project");
    const std::string groupName = "/myGroup";
    project.createGroup(groupName);

    //
    // 0
    //
    const std::string dsName0 = "myMovie0";
    const std::string dsPath0 = groupName + "/" + dsName0;
    const isx::DataSet::Type dsType0 = isx::DataSet::Type::MOVIE;
    const std::string dsFileNameInput0 = "myMovie0.isxd";
    const std::string propKey0 = "test0";
    const isx::Variant propValue0(1.f);
    isx::DataSet::Properties properties0;
    properties0[propKey0] = propValue0;

    const std::string dsNameD0 = "myCellSetD0";
    const std::string dsPathD0 = dsPath0 + "/derived/" + dsNameD0;
    const isx::DataSet::Type dsTypeD0 = isx::DataSet::Type::CELLSET;
    const std::string dsFileNameDInput0 = "myCellSetD0.isxd";

    project.createDataSet(
        dsPath0,
        dsType0,
        dsFileNameInput0,
        properties0);
    project.createDataSet(
        dsPathD0,
        dsTypeD0,
        dsFileNameDInput0);

    //
    // 1
    //
    const std::string dsName1 = "myMovie1";
    const std::string dsPath1 = groupName + "/" + dsName1;
    const isx::DataSet::Type dsType1 = isx::DataSet::Type::MOVIE;
    const std::string dsFileNameInput1 = "myMovie1.isxd";
    const std::string propKey1 = "test1";
    const isx::Variant propValue1(1.1f);
    isx::DataSet::Properties properties1;
    properties1[propKey1] = propValue1;

    const std::string dsNameD1 = "myCellSetD1";
    const std::string dsPathD1 = dsPath1 + "/derived/" + dsNameD1;
    const isx::DataSet::Type dsTypeD1 = isx::DataSet::Type::CELLSET;
    const std::string dsFileNameDInput1 = "myCellSetD1.isxd";

    project.createDataSet(
        dsPath1,
        dsType1,
        dsFileNameInput1,
        properties1);
    project.createDataSet(
        dsPathD1,
        dsTypeD1,
        dsFileNameDInput1);

    std::vector<isx::DataSet *> dataSets{project.getDataSet(dsPath0), project.getDataSet(dsPath1)};
    std::vector<isx::DataSet *> derivedDataSets{project.getDataSet(dsPathD0), project.getDataSet(dsPathD1)};
    
    const std::string dsFileName0 = dataSets[0]->getFileName();
    const std::string dsFileName1 = dataSets[1]->getFileName();
    const std::string dsFileNameD0 = derivedDataSets[0]->getFileName();
    const std::string dsFileNameD1 = derivedDataSets[1]->getFileName();

    const std::string expected = "{\"dataSets\":[{\"original\":{\"dataSetType\":0,\"fileName\":\""
        + dsFileName0
        + "\",\"name\":\"myMovie0\",\"properties\":{\"test0\":1},\"type\":\"DataSet\"}},{\"original\":{\"dataSetType\":0,\"fileName\":\""
        + dsFileName1
        + "\",\"name\":\"myMovie1\",\"properties\":{\"test1\":1.10000002384186},\"type\":\"DataSet\"}}],\"path\":\"/myGroup\"}";
    const std::string derived_expected = "{\"dataSets\":[{\"derived\":{\"dataSetType\":1,\"fileName\":\""
        + dsFileNameD0
        + "\",\"name\":\"myCellSetD0\",\"properties\":{},\"type\":\"DataSet\"},\"original\":{\"dataSetType\":0,\"fileName\":\""
        + dsFileName0
        + "\",\"name\":\"myMovie0\",\"properties\":{\"test0\":1},\"type\":\"DataSet\"}},{\"derived\":{\"dataSetType\":1,\"fileName\":\""
        + dsFileNameD1
        + "\",\"name\":\"myCellSetD1\",\"properties\":{},\"type\":\"DataSet\"},\"original\":{\"dataSetType\":0,\"fileName\":\""
        + dsFileName1
        + "\",\"name\":\"myMovie1\",\"properties\":{\"test1\":1.10000002384186},\"type\":\"DataSet\"}}],\"path\":\"/myGroup\"}";

    SECTION("ToJson - original only")
    {
        std::vector<isx::DataSet *> deriveds{nullptr, nullptr};
        std::string js = isx::DataSet::toJsonString(groupName, dataSets, deriveds);
        REQUIRE(js == expected);
    }

    SECTION("FromJson - original only")
    {
        std::string path;
        std::vector<isx::DataSet> ds;
        std::vector<isx::DataSet> dds;
        isx::DataSet::fromJsonString(expected, path, ds, dds);
        REQUIRE(ds.size() == 2);
        REQUIRE(dds.size() == 0);
        
        const isx::DataSet::Properties & props0 = ds[0].getProperties();
        REQUIRE(path == groupName);
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

    SECTION("ToJson - original and derived")
    {
        std::string js = isx::DataSet::toJsonString(groupName, dataSets, derivedDataSets);
        REQUIRE(js == derived_expected);
    }
    
    SECTION("FromJson - original and derived")
    {
        std::string path;
        std::vector<isx::DataSet> ds;
        std::vector<isx::DataSet> dds;
        isx::DataSet::fromJsonString(derived_expected, path, ds, dds);
        REQUIRE(ds.size() == 2);
        REQUIRE(dds.size() == 2);
        
        // Original 0
        const isx::DataSet::Properties & props0 = ds[0].getProperties();
        REQUIRE(path == groupName);
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
