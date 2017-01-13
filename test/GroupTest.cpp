#include "isxGroup.h"
#include "isxSeries.h"
#include "isxDataSet.h"

#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"

TEST_CASE("Group-Group", "[core]")
{

    SECTION("Empty constructor")
    {
        isx::Group group;

        REQUIRE(!group.isValid());
    }

    SECTION("Construct a group")
    {
        isx::Group group("myGroup");

        REQUIRE(group.isValid());
        REQUIRE(group.getName() == "myGroup");
        REQUIRE(group.getParent() == nullptr);
        REQUIRE(group.getPath() == "myGroup");
        REQUIRE(group.getNumChildren() == 0);
    }

}

TEST_CASE("Group-insertChild", "[core]")
{
    isx::Group group("myGroup");
    auto subGroup = std::make_shared<isx::Group>("subGroup");
    auto series = std::make_shared<isx::Series>("series");
    auto dataSet = std::make_shared<isx::DataSet>("dataSet", isx::DataSet::Type::MOVIE, "movie.isxd");

    SECTION("Insert a sub-group at the end of a group")
    {
        group.insertChild(subGroup);

        REQUIRE(group.getChild("subGroup") == subGroup.get());
    }

    SECTION("Insert the same sub-group into two groups")
    {
        group.insertChild(subGroup);

        isx::Group group2("myGroup2");
        group2.insertChild(subGroup);

        ISX_REQUIRE_EXCEPTION(
                group.getChild("subGroup"),
                isx::ExceptionDataIO,
                "Could not find child with the name: subGroup");

        REQUIRE(group2.getChild("subGroup") == subGroup.get());
    }

    SECTION("Insert a series at the end of a group")
    {
        group.insertChild(series);

        REQUIRE(group.getChild("series") == series.get());
    }

    SECTION("Insert a dataset at the end of a group")
    {
        group.insertChild(dataSet);

        REQUIRE(group.getChild("dataSet") == dataSet.get());
    }

    SECTION("Try to insert two datasets with the same name in a group")
    {
        group.insertChild(dataSet);
        auto dataSet2 = std::make_shared<isx::DataSet>("dataSet", isx::DataSet::Type::MOVIE, "movie2.isxd");
        ISX_REQUIRE_EXCEPTION(
                group.insertChild(dataSet2),
                isx::ExceptionDataIO,
                "There is already an item with the name: dataSet");
    }

    SECTION("Insert two datasets with different names to the end of a group")
    {
        group.insertChild(dataSet);
        auto dataSet2 = std::make_shared<isx::DataSet>("dataSet2", isx::DataSet::Type::MOVIE, "movie2.isxd");
        group.insertChild(dataSet2);

        REQUIRE(group.getChild("dataSet") == dataSet.get());
        REQUIRE(group.getChild("dataSet2") == dataSet2.get());
    }

    SECTION("Insert two datasets with different names to a group in an interesting order")
    {
        group.insertChild(dataSet);
        auto dataSet2 = std::make_shared<isx::DataSet>("dataSet2", isx::DataSet::Type::MOVIE, "movie2.isxd");
        group.insertChild(dataSet2, 0);

        std::vector<isx::ProjectItem *> children = group.getChildren();
        REQUIRE(children.at(0) == dataSet2.get());
        REQUIRE(children.at(1) == dataSet.get());
    }

    SECTION("Try to insert a group in itself")
    {
        ISX_REQUIRE_EXCEPTION(
                subGroup->insertChild(subGroup),
                isx::ExceptionDataIO,
                "An item cannot be inserted in itself.");
    }

    SECTION("Try to insert an ancestor of a group")
    {
        auto group2 = std::make_shared<isx::Group>("group");
        group2->insertChild(subGroup);
        ISX_REQUIRE_EXCEPTION(
                subGroup->insertChild(group2),
                isx::ExceptionDataIO,
                "The inserted item is an ancestor of this.");
    }

}

TEST_CASE("Group-removeChild", "[core]")
{
    isx::Group group("myGroup");
    auto subGroup = std::make_shared<isx::Group>("subGroup");
    auto series = std::make_shared<isx::Series>("series");
    auto dataSet = std::make_shared<isx::DataSet>("dataSet", isx::DataSet::Type::MOVIE, "movie.isxd");

    group.insertChild(subGroup);
    group.insertChild(series);
    group.insertChild(dataSet);

    SECTION("Remove an item at the end")
    {
        group.removeChild("dataSet");

        REQUIRE(group.getChildren().size() == 2);
        REQUIRE(group.getChild("subGroup") == subGroup.get());
        REQUIRE(group.getChild("series") == series.get());

        ISX_REQUIRE_EXCEPTION(
                group.getChild("dataSet"),
                isx::ExceptionDataIO,
                "Could not find child with the name: dataSet");
    }

    SECTION("Remove an item in the middle")
    {
        group.removeChild("series");

        REQUIRE(group.getChildren().size() == 2);
        REQUIRE(group.getChild("subGroup") == subGroup.get());
        REQUIRE(group.getChild("dataSet") == dataSet.get());

        ISX_REQUIRE_EXCEPTION(
                group.getChild("series"),
                isx::ExceptionDataIO,
                "Could not find child with the name: series");
    }

    SECTION("Remove an item that does not exist")
    {
        ISX_REQUIRE_EXCEPTION(
                group.removeChild("movie"),
                isx::ExceptionDataIO,
                "Could not find item with the name: movie");
    }

}

TEST_CASE("Group-toFromJsonString", "[core]")
{
    auto subGroup = std::make_shared<isx::Group>("subGroup");
    auto series = std::make_shared<isx::Series>("series");
    auto dataSet = std::make_shared<isx::DataSet>("dataSet", isx::DataSet::Type::MOVIE, "movie.isxd");

    SECTION("An empty group")
    {
        const isx::Group expected("group");

        const std::string jsonString = expected.toJsonString();
        const std::shared_ptr<isx::Group> actual = isx::Group::fromJsonString(jsonString);

        REQUIRE(*actual == expected);
    }

    SECTION("A group containing a sub-group, series and data set")
    {
        isx::Group expected("group");
        expected.insertChild(subGroup);
        expected.insertChild(series);
        expected.insertChild(dataSet);

        const std::string jsonString = expected.toJsonString();
        const std::shared_ptr<isx::Group> actual = isx::Group::fromJsonString(jsonString);

        REQUIRE(*actual == expected);
    }

}

TEST_CASE("Group-getIndex", "[core]")
{
    isx::Group root("/");

    auto group1 = std::make_shared<isx::Group>("group1");
    auto group2 = std::make_shared<isx::Group>("group2");
    auto group3 = std::make_shared<isx::Group>("group3");
    auto group4 = std::make_shared<isx::Group>("group4");

    root.insertChild(group1);
    group2->insertChild(group3);
    group2->insertChild(group4);
    root.insertChild(group2);

    SECTION("Get the index of the root group")
    {
        REQUIRE(root.getIndex() == -1);
    }

    SECTION("Get the index of the groups immediately owned by root")
    {
        REQUIRE(group1->getIndex() == 0);
        REQUIRE(group2->getIndex() == 1);
    }

    SECTION("Get the index of the groups immediately owned by group2")
    {
        REQUIRE(group3->getIndex() == 0);
        REQUIRE(group4->getIndex() == 1);
    }

}
