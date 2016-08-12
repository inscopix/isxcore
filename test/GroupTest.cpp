#include "isxGroup.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"

TEST_CASE("GroupTest", "[core]")
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
        REQUIRE(!group.getParent());
        REQUIRE(group.getPath() == "myGroup");
    }

    SECTION("Add a group to another group")
    {
        isx::Group group("myGroup");

        isx::Group * subGroup = group.addGroup(
                std::unique_ptr<isx::Group>(new isx::Group("mySubGroup")));

        REQUIRE(group.getGroup("mySubGroup") == subGroup);
    }

    SECTION("Add a group within a root (/) group)")
    {
        isx::Group group("/");

        isx::Group * subGroup = group.addGroup(
                std::unique_ptr<isx::Group>(new isx::Group("myGroup")));

        REQUIRE(group.getGroup("myGroup") == subGroup);
    }

    SECTION("Try to add two groups with the same name in another group")
    {
        isx::Group group("myGroup");
        group.addGroup(std::unique_ptr<isx::Group>(new isx::Group("mySubGroup")));
        try
        {
            group.addGroup(std::unique_ptr<isx::Group>(new isx::Group("mySubGroup")));
            FAIL("Failed to throw an exception.");
        }
        catch (const isx::ExceptionDataIO & error)
        {
            REQUIRE(std::string(error.what()) ==
                    "There is already an item with the name: mySubGroup");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionDataIO");
        }
    }

    SECTION("Add two groups with different names to another group")
    {
        isx::Group group("myGroup");
        isx::Group * subGroup1 = group.addGroup(
                std::unique_ptr<isx::Group>(new isx::Group("mySubGroup1")));
        isx::Group * subGroup2 = group.addGroup(
                std::unique_ptr<isx::Group>(new isx::Group("mySubGroup2")));

        REQUIRE(group.getGroup("mySubGroup1") == subGroup1);
        REQUIRE(group.getGroup("mySubGroup2") == subGroup2);
    }

    SECTION("Remove a group by name")
    {
        isx::Group group("myGroup");
        isx::Group * subGroup1 = group.addGroup(
                std::unique_ptr<isx::Group>(new isx::Group("mySubGroup1")));
        isx::Group * subGroup2 = group.addGroup(
                std::unique_ptr<isx::Group>(new isx::Group("mySubGroup2")));

        group.removeGroup("mySubGroup1");

        REQUIRE(group.getGroups().size() == 1);
        REQUIRE(group.getGroup("mySubGroup2") == subGroup2);
        try
        {
            group.getGroup("mySubGroup1");
            FAIL("Failed to throw an exception.");
        }
        catch (const isx::ExceptionDataIO & error)
        {
            REQUIRE(std::string(error.what()) ==
                    "Could not find group with the name: mySubGroup1");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionDataIO");
        }
    }

    SECTION("Add a data set to a group")
    {
        isx::Group group("myGroup");
        isx::DataSet * dataSet = group.addDataSet(std::unique_ptr<isx::DataSet>(
                    new isx::DataSet("myDataSet", isx::DataSet::Type::MOVIE, "myMovie.isxd")));

        REQUIRE(group.getDataSet("myDataSet") == dataSet);
    }

    SECTION("Try to create two data sets with the same name in a group")
    {
        isx::Group group("myGroup");
        group.addDataSet(std::unique_ptr<isx::DataSet>(
                    new isx::DataSet("myDataSet", isx::DataSet::Type::MOVIE, "myMovie1.isxd")));
        try
        {
            group.addDataSet(std::unique_ptr<isx::DataSet>(
                    new isx::DataSet("myDataSet", isx::DataSet::Type::MOVIE, "myMovie2.isxd")));
            FAIL("Failed to throw an exception.");
        }
        catch (const isx::ExceptionDataIO & error)
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
        isx::Group group("myGroup");
        group.addDataSet(std::unique_ptr<isx::DataSet>(
                    new isx::DataSet("myDataSet1", isx::DataSet::Type::MOVIE, "myMovie.isxd")));
        try
        {
            group.addDataSet(std::unique_ptr<isx::DataSet>(
                    new isx::DataSet("myDataSet2", isx::DataSet::Type::MOVIE, "myMovie.isxd")));
            FAIL("Failed to throw an exception.");
        }
        catch (const isx::ExceptionFileIO & error)
        {
            REQUIRE(std::string(error.what()) ==
                    "There is already a data set with the file name: myMovie.isxd");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionFileIO");
        }
    }

    SECTION("Add two data sets")
    {
        isx::Group group("myGroup");

        isx::DataSet * dataSet1 = group.addDataSet(std::unique_ptr<isx::DataSet>(
                    new isx::DataSet("myDataSet1", isx::DataSet::Type::MOVIE, "myMovie1.isxd")));
        isx::DataSet * dataSet2 = group.addDataSet(std::unique_ptr<isx::DataSet>(
                    new isx::DataSet("myDataSet2", isx::DataSet::Type::MOVIE, "myMovie2.isxd")));

        REQUIRE(group.getDataSet("myDataSet1") == dataSet1);
        REQUIRE(group.getDataSet("myDataSet1") == dataSet1);
    }

    SECTION("Remove a data set by name")
    {
        isx::Group group("myGroup");

        isx::DataSet * dataSet1 = group.addDataSet(std::unique_ptr<isx::DataSet>(
                    new isx::DataSet("myDataSet1", isx::DataSet::Type::MOVIE, "myMovie1.isxd")));
        isx::DataSet * dataSet2 = group.addDataSet(std::unique_ptr<isx::DataSet>(
                    new isx::DataSet("myDataSet2", isx::DataSet::Type::MOVIE, "myMovie2.isxd")));

        group.removeDataSet("myDataSet1");

        REQUIRE(group.getDataSets().size() == 1);
        REQUIRE(group.getDataSet("myDataSet2") == dataSet2);
        try
        {
            group.getGroup("myDataSet1");
            FAIL("Failed to throw an exception.");
        }
        catch (const isx::ExceptionDataIO & error)
        {
            REQUIRE(std::string(error.what()) ==
                    "Could not find group with the name: myDataSet1");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionDataIO");
        }
    }

    SECTION("Get data sets in a group")
    {
        isx::Group group("myGroup");

        isx::DataSet * dataSet1 = group.addDataSet(std::unique_ptr<isx::DataSet>(
                    new isx::DataSet("myDataSet1", isx::DataSet::Type::MOVIE, "myMovie1.isxd")));
        isx::DataSet * dataSet2 = group.addDataSet(std::unique_ptr<isx::DataSet>(
                    new isx::DataSet("myDataSet2", isx::DataSet::Type::MOVIE, "myMovie2.isxd")));

        std::vector<isx::DataSet *> actualDataSets = group.getDataSets();

        std::vector<isx::DataSet *> expectedDataSets = {dataSet1, dataSet2};
        REQUIRE(actualDataSets == expectedDataSets);
    }

    SECTION("Get data sets in a group recursively")
    {
        isx::Group group("myGroup");

        isx::Group * subGroup1 = group.addGroup(
                std::unique_ptr<isx::Group>(new isx::Group("mySubGroup1")));
        isx::Group * subGroup2 = group.addGroup(
                std::unique_ptr<isx::Group>(new isx::Group("mySubGroup2")));

        isx::DataSet * dataSet1 = group.addDataSet(std::unique_ptr<isx::DataSet>(
                    new isx::DataSet("myDataSet1", isx::DataSet::Type::MOVIE, "myMovie1.isxd")));
        isx::DataSet * dataSet2 = subGroup2->addDataSet(std::unique_ptr<isx::DataSet>(
                    new isx::DataSet("myDataSet2", isx::DataSet::Type::MOVIE, "myMovie2.isxd")));

        std::vector<isx::DataSet *> actualDataSets = group.getDataSets(true);

        std::vector<isx::DataSet *> expectedDataSets = {dataSet1, dataSet2};
        REQUIRE(actualDataSets == expectedDataSets);
    }

}
