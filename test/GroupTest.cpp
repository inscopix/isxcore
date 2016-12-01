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

        isx::Group * subGroup = group.createGroup("mySubGroup");

        REQUIRE(group.getGroup("mySubGroup") == subGroup);
    }

    SECTION("Add a group within a root (/) group)")
    {
        isx::Group group("/");

        isx::Group * subGroup = group.createGroup("myGroup");

        REQUIRE(group.getGroup("myGroup") == subGroup);
    }

    SECTION("Try to add two groups with the same name in another group")
    {
        isx::Group group("myGroup");
        group.createGroup("mySubGroup");

        ISX_EXPECT_EXCEPTION();
        try
        {
            group.createGroup("mySubGroup");
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
        isx::Group * subGroup1 = group.createGroup("mySubGroup1");
        isx::Group * subGroup2 = group.createGroup("mySubGroup2");

        REQUIRE(group.getGroup("mySubGroup1") == subGroup1);
        REQUIRE(group.getGroup("mySubGroup2") == subGroup2);
    }

    SECTION("Remove a group by name")
    {
        isx::Group group("myGroup");
        isx::Group * subGroup1 = group.createGroup("mySubGroup1");
        isx::Group * subGroup2 = group.createGroup("mySubGroup2");

        group.removeGroup("mySubGroup1");

        REQUIRE(group.getGroups().size() == 1);
        REQUIRE(group.getGroup("mySubGroup2") == subGroup2);

        ISX_EXPECT_EXCEPTION();
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
        isx::DataSet * dataSet = group.createDataSet("myDataSet", isx::DataSet::Type::MOVIE, "myMovie.isxd");

        REQUIRE(group.getDataSet("myDataSet") == dataSet);
    }

    SECTION("Try to create two data sets with the same name in a group")
    {
        isx::Group group("myGroup");
        group.createDataSet("myDataSet", isx::DataSet::Type::MOVIE, "myMovie1.isxd");

        ISX_EXPECT_EXCEPTION();
        try
        {
            group.createDataSet("myDataSet", isx::DataSet::Type::MOVIE, "myMovie2.isxd");
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
        group.createDataSet("myDataSet1", isx::DataSet::Type::MOVIE, "myMovie.isxd");

        ISX_EXPECT_EXCEPTION();
        try
        {
            group.createDataSet("myDataSet2", isx::DataSet::Type::MOVIE, "myMovie.isxd");
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

        isx::DataSet * dataSet1 = group.createDataSet("myDataSet1", isx::DataSet::Type::MOVIE, "myMovie1.isxd");
        isx::DataSet * dataSet2 = group.createDataSet("myDataSet2", isx::DataSet::Type::MOVIE, "myMovie2.isxd");

        REQUIRE(group.getDataSet("myDataSet1") == dataSet1);
        REQUIRE(group.getDataSet("myDataSet1") == dataSet1);
    }

    SECTION("Remove a data set by name")
    {
        isx::Group group("myGroup");

        isx::DataSet * dataSet1 = group.createDataSet("myDataSet1", isx::DataSet::Type::MOVIE, "myMovie1.isxd");
        isx::DataSet * dataSet2 = group.createDataSet("myDataSet2", isx::DataSet::Type::MOVIE, "myMovie2.isxd");

        group.removeDataSet("myDataSet1");

        REQUIRE(group.getDataSets().size() == 1);
        REQUIRE(group.getDataSet("myDataSet2") == dataSet2);

        ISX_EXPECT_EXCEPTION();
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

        isx::DataSet * dataSet1 = group.createDataSet("myDataSet1", isx::DataSet::Type::MOVIE, "myMovie1.isxd");
        isx::DataSet * dataSet2 = group.createDataSet("myDataSet2", isx::DataSet::Type::MOVIE, "myMovie2.isxd");

        std::vector<isx::DataSet *> actualDataSets = group.getDataSets();

        std::vector<isx::DataSet *> expectedDataSets = {dataSet1, dataSet2};
        REQUIRE(actualDataSets == expectedDataSets);
    }

    SECTION("Get data sets in a group recursively")
    {
        isx::Group group("myGroup");

        isx::Group * subGroup1 = group.createGroup("mySubGroup1");
        isx::Group * subGroup2 = group.createGroup("mySubGroup2");

        isx::DataSet * dataSet1 = group.createDataSet("myDataSet1", isx::DataSet::Type::MOVIE, "myMovie1.isxd");
        isx::DataSet * dataSet2 = subGroup2->createDataSet("myDataSet2", isx::DataSet::Type::MOVIE, "myMovie2.isxd");

        std::vector<isx::DataSet *> actualDataSets = group.getDataSets(true);

        std::vector<isx::DataSet *> expectedDataSets = {dataSet1, dataSet2};
        REQUIRE(actualDataSets == expectedDataSets);
    }

}

TEST_CASE("Group-moveDataSet", "[core]")
{
    isx::Group root("/");
    isx::DataSet * movie1 = root.createDataSet("movie1", isx::DataSet::Type::MOVIE, "movie1.isxd");
    isx::DataSet * movie2 = root.createDataSet("movie2", isx::DataSet::Type::MOVIE, "movie2.isxd");
    isx::Group * series = root.createGroup("Day 1", isx::Group::Type::SERIES);

    SECTION("Move a data set from root to a series")
    {
        root.moveDataSet("movie1", series);

        REQUIRE(root.getDataSets().size() == 1);
        REQUIRE(root.getDataSet("movie2") == movie2);
        ISX_EXPECT_EXCEPTION();
        try
        {
            root.getDataSet("movie1");
            FAIL("Failed to throw an exception.");
        }
        catch (const isx::ExceptionDataIO & error)
        {
            REQUIRE(std::string(error.what()) ==
                    "Could not find data set with name: movie1");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionDataIO");
        }

        REQUIRE(series->getDataSets().size() == 1);
        REQUIRE(series->getDataSet("movie1") == movie1);
    }

}

TEST_CASE("Group-moveGroup", "[core]")
{
    isx::Group root("/");
    isx::Group * group1 = root.createGroup("group1");
    isx::Group * group2 = root.createGroup("group2");
    isx::Group * group3 = root.createGroup("group3");

    SECTION("Move a group from root to another group")
    {
        root.moveGroup("group1", group3);

        REQUIRE(root.getGroups().size() == 2);
        REQUIRE(root.getGroup("group2") == group2);
        ISX_EXPECT_EXCEPTION();
        try
        {
            root.getGroup("group1");
            FAIL("Failed to throw an exception.");
        }
        catch (const isx::ExceptionDataIO & error)
        {
            REQUIRE(std::string(error.what()) ==
                    "Could not find group with the name: group1");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionDataIO");
        }

        REQUIRE(group3->getGroups().size() == 1);
        REQUIRE(group3->getGroup("group1") == group1);
    }

    SECTION("Identity move")
    {
        root.moveGroup("group1", &root, 0);

        const std::vector<isx::Group *> groups = root.getGroups();
        REQUIRE(groups.size() == 3);
        REQUIRE(groups.at(0) == group1);
        REQUIRE(groups.at(1) == group2);
        REQUIRE(groups.at(2) == group3);
    }

    SECTION("Internal move to the beginning of a group")
    {
        root.moveGroup("group2", &root, 0);

        const std::vector<isx::Group *> groups = root.getGroups();
        REQUIRE(groups.size() == 3);
        REQUIRE(groups.at(0) == group2);
        REQUIRE(groups.at(1) == group1);
        REQUIRE(groups.at(2) == group3);
    }

    SECTION("Internal move to the middle of a group")
    {
        root.moveGroup("group1", &root, 1);

        const std::vector<isx::Group *> groups = root.getGroups();
        REQUIRE(groups.size() == 3);
        REQUIRE(groups.at(0) == group2);
        REQUIRE(groups.at(1) == group1);
        REQUIRE(groups.at(2) == group3);
    }

    SECTION("Internal move to the end of a group")
    {
        root.moveGroup("group1", &root, 2);

        const std::vector<isx::Group *> groups = root.getGroups();
        REQUIRE(groups.size() == 3);
        REQUIRE(groups.at(0) == group2);
        REQUIRE(groups.at(1) == group3);
        REQUIRE(groups.at(2) == group1);
    }

    SECTION("Internal move past the end of a group")
    {
        root.moveGroup("group1", &root, 3);

        const std::vector<isx::Group *> groups = root.getGroups();
        REQUIRE(groups.size() == 3);
        REQUIRE(groups.at(0) == group2);
        REQUIRE(groups.at(1) == group3);
        REQUIRE(groups.at(2) == group1);
    }

}

TEST_CASE("Group-getIndex", "[core]")
{
    isx::Group root("/");
    isx::Group * group1 = root.createGroup("group1");
    isx::Group * group2 = root.createGroup("group2");
    isx::Group * group3 = group2->createGroup("group3");
    isx::Group * group4 = group2->createGroup("group4");

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


