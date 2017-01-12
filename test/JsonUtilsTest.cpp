#include "isxJsonUtils.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxVariant.h"

#include <stdio.h>

TEST_CASE("JsonUtilsTest", "[core-internal]")
{
    std::string movieFileName = g_resources["unitTestDataPath"] + "/movie.isxd";
    std::remove(movieFileName.c_str());

    SECTION("Convert a movie data set to json")
    {
        isx::DataSet::Properties inProperties;
        inProperties["test"] = isx::Variant(1.0f);
        isx::DataSet dataSet("myDataSet", isx::DataSet::Type::MOVIE, movieFileName, inProperties);

        isx::json jsonObject = isx::convertDataSetToJson(&dataSet);

        std::string type = jsonObject["type"];
        isx::DataSet::Type dataSetType = isx::DataSet::Type(isx::isize_t(jsonObject["dataSetType"]));
        std::string fileName = jsonObject["fileName"];
        isx::DataSet::Properties outProperties = isx::convertJsonToProperties(jsonObject["properties"]);
        REQUIRE(type == "DataSet");
        REQUIRE(dataSetType == isx::DataSet::Type::MOVIE);
        REQUIRE(fileName == movieFileName);
        REQUIRE(outProperties["test"] == inProperties["test"]);
    }

    SECTION("Convert json to a movie data set")
    {
        isx::json jsonObject;
        jsonObject["type"] = "DataSet";
        jsonObject["name"] = "myDataSet";
        jsonObject["dataSetType"] = isx::isize_t(isx::DataSet::Type::MOVIE);
        jsonObject["fileName"] = movieFileName;
        isx::DataSet::Properties inProperties;
        inProperties["test"] = isx::Variant(1.0f);
        jsonObject["properties"] = isx::convertPropertiesToJson(inProperties);

        isx::Group group;
        isx::createDataSetFromJson(&group, jsonObject);
        isx::DataSet * dataSet = group.getDataSet("myDataSet");

        REQUIRE(dataSet->isValid());
        REQUIRE(dataSet->getName() == "myDataSet");
        REQUIRE(dataSet->getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(dataSet->getFileName() == movieFileName);
        isx::Variant value;
        REQUIRE(dataSet->getPropertyValue("test", value));
        REQUIRE(value.value<float>() == 1.0f);
    }

    SECTION("Convert an empty group to json")
    {
        std::string name = "myGroup";
        isx::Group group(name);

        isx::json jsonObject = isx::convertGroupToJson(&group);

        std::string jsonType = jsonObject["type"];
        std::string jsonName = jsonObject["name"];
        std::vector<isx::json> jsonGroups = jsonObject["groups"];
        std::vector<isx::json> jsonDataSets = jsonObject["dataSets"];
        REQUIRE(jsonType == "Group");
        REQUIRE(jsonName == name);
        REQUIRE(jsonGroups.empty());
        REQUIRE(jsonDataSets.empty());
    }

    SECTION("Convert json to an empty group")
    {
        std::string name = "myGroup";
        isx::json jsonObject;
        jsonObject["type"] = "Group";
        jsonObject["name"] = name;
        jsonObject["groups"] = isx::json::array();
        jsonObject["dataSets"] = isx::json::array();

        std::unique_ptr<isx::Group> group = isx::createProjectTreeFromJson(jsonObject);

        REQUIRE(group->isValid());
        REQUIRE(group->getName() == name);
        REQUIRE(group->getGroups().empty());
        REQUIRE(group->getDataSets().empty());
    }

    SECTION("Convert a group containing other groups to json")
    {
        isx::Group group("myGroup");
        group.createGroup("mySubGroup");

        isx::json jsonObject = isx::convertGroupToJson(&group);

        std::string jsonType = jsonObject["type"];
        std::string jsonName = jsonObject["name"];
        std::vector<isx::json> jsonGroups = jsonObject["groups"];
        std::vector<isx::json> jsonDataSets = jsonObject["dataSets"];
        REQUIRE(jsonType == "Group");
        REQUIRE(jsonName == "myGroup");

        REQUIRE(jsonGroups.size() == 1);
        isx::json jsonSubObject = jsonGroups[0];
        std::string jsonSubType = jsonSubObject["type"];
        std::string jsonSubName = jsonSubObject["name"];
        std::vector<isx::json> jsonSubGroups = jsonSubObject["groups"];
        std::vector<isx::json> jsonSubDataSets = jsonSubObject["dataSets"];
        REQUIRE(jsonSubName == "mySubGroup");
        REQUIRE(jsonSubType == "Group");
        REQUIRE(jsonSubGroups.empty());
        REQUIRE(jsonSubDataSets.empty());

        REQUIRE(jsonDataSets.empty());
    }

    SECTION("Convert timing info to json")
    {
        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::isize_t numTimes = 20;
        std::vector<isx::isize_t> droppedFrames{4, 7};
        isx::TimingInfo timingInfo(start, step, numTimes, droppedFrames);

        isx::json jsonObject = isx::convertTimingInfoToJson(timingInfo);

        isx::isize_t jsonNumTimes = jsonObject["numTimes"];
        isx::DurationInSeconds jsonStep = isx::convertJsonToRatio(jsonObject["period"]);
        isx::Time jsonStart = isx::convertJsonToTime(jsonObject["start"]);
        std::vector<isx::isize_t> jsonDroppedFrames = jsonObject["dropped"].get<std::vector<isx::isize_t>>();
        
        REQUIRE(jsonNumTimes == numTimes);
        REQUIRE(jsonStep == step);
        REQUIRE(jsonStart == start);
        REQUIRE(jsonDroppedFrames == droppedFrames);
    }

}
