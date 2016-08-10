#include "isxGroup.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"

#include <stdio.h>

TEST_CASE("GroupTest", "[core]")
{

    SECTION("Empty constructor")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>();

        REQUIRE(!group->isValid());
    }

    SECTION("Construct a group")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");

        REQUIRE(group->isValid());
        REQUIRE(group->getName() == "myGroup");
        REQUIRE(!group->getParent());
        REQUIRE(group->getPath() == "myGroup");
    }

    SECTION("Create a group within another group")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");

        isx::SpGroup_t subGroup = group->createGroup("mySubGroup");

        REQUIRE(subGroup->isValid());
        REQUIRE(subGroup->getName() == "mySubGroup");
        REQUIRE(subGroup->getParent() == group);
        REQUIRE(subGroup->getPath() == "myGroup/mySubGroup");
    }

    SECTION("Create a group within a root (/) group)")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("/");

        isx::SpGroup_t subGroup = group->createGroup("myGroup");

        REQUIRE(subGroup->getPath() == "/myGroup");
    }

    SECTION("Try to create two groups with the same name in another group")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        isx::SpGroup_t subGroup1 = group->createGroup("mySubGroup");

        try
        {
            isx::SpGroup_t subGroup2 = group->createGroup("mySubGroup");
            FAIL("Failed to throw an exception.");
        }
        catch (isx::ExceptionDataIO & error)
        {
            REQUIRE(std::string(error.what()) ==
                    "There is already an item with the name: mySubGroup");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionDataIO");
        }
    }

    SECTION("Get a group by name")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        isx::SpGroup_t subGroup1 = group->createGroup("mySubGroup1");
        isx::SpGroup_t subGroup2 = group->createGroup("mySubGroup2");

        REQUIRE(group->getGroup("mySubGroup1") == subGroup1);
    }

    SECTION("Remove a group by name")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        isx::SpGroup_t subGroup1 = group->createGroup("mySubGroup1");
        isx::SpGroup_t subGroup2 = group->createGroup("mySubGroup2");

        group->removeGroup("mySubGroup1");

        REQUIRE(group->getGroups().size() == 1);
        REQUIRE(group->getGroup("mySubGroup2") == subGroup2);
    }

    SECTION("Create a data set in a group")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        isx::SpDataSet_t dataSet = group->createDataSet(
                "myDataSet", isx::DataSet::Type::MOVIE, "myMovie.isxd");

        REQUIRE(dataSet->isValid());
        REQUIRE(dataSet->getName() == "myDataSet");
        REQUIRE(dataSet->getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(dataSet->getFileName() == "myMovie.isxd");
        REQUIRE(dataSet->getParent() == group);
    }


    SECTION("Try to create two data sets with the same name in a group")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        isx::SpDataSet_t dataSet1 = group->createDataSet(
                "myDataSet", isx::DataSet::Type::MOVIE, "myMovie1.isxd");
        try
        {
            isx::SpDataSet_t dataSet2 = group->createDataSet(
                "myDataSet", isx::DataSet::Type::MOVIE, "myMovie2.isxd");
            FAIL("Failed to throw an exception.");
        }
        catch (isx::ExceptionDataIO & error)
        {
            REQUIRE(std::string(error.what()) ==
                    "There is already an item with the name: myDataSet");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionDataIO");
        }
    }

    SECTION("Try to create two data sets with the same file name in a group")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        isx::SpDataSet_t dataSet1 = group->createDataSet(
                "myDataSet1", isx::DataSet::Type::MOVIE, "myMovie.isxd");
        try
        {
            isx::SpDataSet_t dataSet2 = group->createDataSet(
                "myDataSet2", isx::DataSet::Type::MOVIE, "myMovie.isxd");
            FAIL("Failed to throw an exception.");
        }
        catch (isx::ExceptionFileIO & error)
        {
            REQUIRE(std::string(error.what()) ==
                    "There is already a data set with the file name: myMovie.isxd");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionFileIO");
        }
    }

    SECTION("Get a data set by name")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        isx::SpDataSet_t dataSet1 = group->createDataSet(
                "myDataSet1", isx::DataSet::Type::MOVIE, "myMovie1.isxd");
        isx::SpDataSet_t dataSet2 = group->createDataSet(
                "myDataSet2", isx::DataSet::Type::MOVIE, "myMovie2.isxd");

        isx::SpDataSet_t actualDataSet = group->getDataSet("myDataSet1");

        REQUIRE(group->getDataSet("myDataSet1") == dataSet1);
    }

    SECTION("Remove a data set by name")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        isx::SpDataSet_t dataSet1 = group->createDataSet(
                "myDataSet1", isx::DataSet::Type::MOVIE, "myMovie1.isxd");
        isx::SpDataSet_t dataSet2 = group->createDataSet(
                "myDataSet2", isx::DataSet::Type::MOVIE, "myMovie2.isxd");

        group->removeDataSet("myDataSet1");
        isx::SpDataSet_t actualDataSet = group->getDataSet("myDataSet2");

        REQUIRE(group->getDataSets().size() == 1);
        REQUIRE(actualDataSet == dataSet2);
    }

    SECTION("Get data sets in a group")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        isx::SpDataSet_t dataSet1 = group->createDataSet(
                "myDataSet1", isx::DataSet::Type::MOVIE, "myMovie1.isxd");
        isx::SpDataSet_t dataSet2 = group->createDataSet(
                "myDataSet2", isx::DataSet::Type::MOVIE, "myMovie2.isxd");

        std::vector<isx::SpDataSet_t> actualDataSets = group->getDataSets();

        std::vector<isx::SpDataSet_t> expectedDataSets = {dataSet1, dataSet2};
        REQUIRE(actualDataSets == expectedDataSets);
    }

    SECTION("Get data sets in a group recursively")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        isx::SpGroup_t subGroup1 = group->createGroup("mySubGroup1");
        isx::SpGroup_t subGroup2 = group->createGroup("mySubGroup2");
        isx::SpDataSet_t dataSet1 = group->createDataSet(
                "myDataSet1", isx::DataSet::Type::MOVIE, "myMovie1.isxd");
        isx::SpDataSet_t dataSet2 = subGroup2->createDataSet(
                "myDataSet2", isx::DataSet::Type::MOVIE, "myMovie2.isxd");

        std::vector<isx::SpDataSet_t> actualDataSets = group->getDataSets(true);

        std::vector<isx::SpDataSet_t> expectedDataSets = {dataSet1, dataSet2};
        REQUIRE(actualDataSets == expectedDataSets);
    }

}
