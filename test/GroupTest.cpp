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
        REQUIRE(group.getContainer() == nullptr);
        REQUIRE(group.getNumGroupMembers() == 0);
    }

}

TEST_CASE("Group-insertGroupMember", "[core]")
{
    isx::Group group("myGroup");
    auto subGroup = std::make_shared<isx::Group>("subGroup");
    auto series = std::make_shared<isx::Series>("series");
    isx::HistoricalDetails hd("test", "");
    auto dataSet = std::make_shared<isx::DataSet>("dataSet", isx::DataSet::Type::MOVIE, "movie.isxd", hd);

    SECTION("Insert a sub-group at the end of a group")
    {
        group.insertGroupMember(subGroup, 0);

        REQUIRE(group.getGroupMember(0) == static_cast<isx::ProjectItem *>(subGroup.get()));
    }

    SECTION("Insert the same sub-group into two groups")
    {
        group.insertGroupMember(subGroup, 0);
        isx::Group group2("myGroup2");
        ISX_REQUIRE_EXCEPTION(
                group2.insertGroupMember(subGroup, 0),
                isx::ExceptionDataIO,
                "New item is still inside of another container: subGroup");

        REQUIRE(group.getGroupMember(0) == static_cast<isx::ProjectItem *>(subGroup.get()));
    }

    SECTION("Insert a series at the end of a group")
    {
        group.insertGroupMember(series, 0);

        REQUIRE(group.getGroupMember(0) == static_cast<isx::ProjectItem *>(series.get()));
    }

    SECTION("Try to insert two series with the same name in a group")
    {
        group.insertGroupMember(series, 0);
        auto series2 = std::make_shared<isx::Series>("series");
        ISX_REQUIRE_EXCEPTION(
                group.insertGroupMember(series2, 0),
                isx::ExceptionDataIO,
                "There is already an item with the name: series");
    }

    SECTION("Insert two series with different names to the end of a group")
    {
        group.insertGroupMember(series, 0);
        auto series2 = std::make_shared<isx::Series>("series2");
        group.insertGroupMember(series2, 1);

        REQUIRE(group.getGroupMember(0) == static_cast<isx::ProjectItem *>(series.get()));
        REQUIRE(group.getGroupMember(1) == static_cast<isx::ProjectItem *>(series2.get()));
    }

    SECTION("Insert two series with different names to a group in an interesting order")
    {
        group.insertGroupMember(series, 0);
        auto series2 = std::make_shared<isx::Series>("series2");
        group.insertGroupMember(series2, 0);
        
        REQUIRE(group.getGroupMember(1) == static_cast<isx::ProjectItem *>(series.get()));
        REQUIRE(group.getGroupMember(0) == static_cast<isx::ProjectItem *>(series2.get()));
    }

    SECTION("Try to insert a group in itself")
    {
        ISX_REQUIRE_EXCEPTION(
                subGroup->insertGroupMember(subGroup, 0),
                isx::ExceptionDataIO,
                "An item cannot be inserted in itself.");
    }

    SECTION("Try to insert an ancestor of a group")
    {
        auto group2 = std::make_shared<isx::Group>("group");
        group2->insertGroupMember(subGroup, 0);
        ISX_REQUIRE_EXCEPTION(
                subGroup->insertGroupMember(group2, 0),
                isx::ExceptionDataIO,
                "The inserted item is an ancestor of this.");
    }

}

TEST_CASE("Group-removeChild", "[core]")
{
    isx::Group group("myGroup");
    auto subGroup = std::make_shared<isx::Group>("subGroup");
    auto series = std::make_shared<isx::Series>("series");
    auto series2 = std::make_shared<isx::Series>("series2");
    auto series3 = std::make_shared<isx::Series>("series3");

    group.insertGroupMember(subGroup, 0);
    group.insertGroupMember(series, 1);
    group.insertGroupMember(series2, 2);

    SECTION("Remove an item at the end")
    {
        group.removeGroupMember(series2.get());

        REQUIRE(group.getNumGroupMembers() == 2);
        REQUIRE(group.getGroupMember(0) == static_cast<isx::ProjectItem *>(subGroup.get()));
        REQUIRE(group.getGroupMember(1) == static_cast<isx::ProjectItem *>(series.get()));

        ISX_REQUIRE_EXCEPTION(
                group.getGroupMember(2),
                isx::ExceptionDataIO,
                "There is no group member with index: 2");
    }

    SECTION("Remove an item in the middle")
    {
        group.removeGroupMember(series.get());

        REQUIRE(group.getNumGroupMembers() == 2);
        REQUIRE(group.getGroupMember(0) == static_cast<isx::ProjectItem *>(subGroup.get()));
        REQUIRE(group.getGroupMember(1) == static_cast<isx::ProjectItem *>(series2.get()));

        ISX_REQUIRE_EXCEPTION(
                group.getGroupMember(2),
                isx::ExceptionDataIO,
                "There is no group member with index: 2");
    }

    SECTION("Remove an item that does not exist")
    {
        ISX_REQUIRE_EXCEPTION(
            group.removeGroupMember(series3.get()),
            isx::ExceptionDataIO,
            "Could not find item with the name: series3");
    }
}

TEST_CASE("Group-toFromJsonString", "[core]")
{
    auto subGroup = std::make_shared<isx::Group>("subGroup");
    auto series = std::make_shared<isx::Series>("series");
    auto series2 = std::make_shared<isx::Series>("series2");

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
        expected.insertGroupMember(subGroup, 0);
        expected.insertGroupMember(series, 1);
        expected.insertGroupMember(series2, 2);

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

    root.insertGroupMember(group1, 0);
    group2->insertGroupMember(group3, 0);
    group2->insertGroupMember(group4, 0);
    root.insertGroupMember(group2, 0);

    SECTION("Get the index of the groups immediately owned by root")
    {
        REQUIRE(group1->getMemberIndex() == 1);
        REQUIRE(group2->getMemberIndex() == 0);
    }

    SECTION("Get the index of the groups immediately owned by group2")
    {
        REQUIRE(group3->getMemberIndex() == 1);
        REQUIRE(group4->getMemberIndex() == 0);
    }

}
