#include "isxDataSet.h"
#include "isxTest.h"
#include "isxException.h"
#include "isxGroup.h"

#include <fstream>
#include "catch.hpp"
#include <stdio.h>

TEST_CASE("DataSetTest", "[core]")
{
    SECTION("Empty constructor")
    {
        isx::SpDataSet_t dataSet = std::make_shared<isx::DataSet>();
        REQUIRE(!dataSet->isValid());
    }

    SECTION("Construct a movie data set")
    {
        std::string fileName = g_resources["testDataPath"] + "/myMovie.isxd";
        std::remove(fileName.c_str());

        isx::SpDataSet_t dataSet = std::make_shared<isx::DataSet>(
                "myMovie", isx::DataSet::MOVIE, fileName);

        REQUIRE(dataSet->isValid());
        REQUIRE(dataSet->getType() == isx::DataSet::MOVIE);
        REQUIRE(dataSet->getName() == "myMovie");
        REQUIRE(!dataSet->hasParent());
        REQUIRE(dataSet->getPath() == "myMovie");
    }

    SECTION("Construct a movie data set then set parent group")
    {
        std::string fileName = g_resources["testDataPath"] + "/myMovie.isxd";
        std::remove(fileName.c_str());
        isx::SpDataSet_t dataSet = std::make_shared<isx::DataSet>(
                "myMovie", isx::DataSet::MOVIE, fileName);

        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        dataSet->setParent(group);

        REQUIRE(dataSet->getName() == "myMovie");
        REQUIRE(dataSet->getType() == isx::DataSet::MOVIE);
        REQUIRE(dataSet->getFileName() == fileName);
        REQUIRE(dataSet->hasParent());
        REQUIRE(dataSet->getParent() == group);
        REQUIRE(dataSet->getPath() == "myGroup/myMovie");
    }

    SECTION("Construct two data sets with same name and try to put in a one group")
    {
        std::string fileName1 = g_resources["testDataPath"] + "/myMovie1.isxd";
        std::remove(fileName1.c_str());

        std::string fileName2 = g_resources["testDataPath"] + "/myMovie2.isxd";
        std::remove(fileName2.c_str());

        isx::SpDataSet_t dataSet1 = std::make_shared<isx::DataSet>(
                "myMovie", isx::DataSet::MOVIE, fileName1);
        isx::SpDataSet_t dataSet2 = std::make_shared<isx::DataSet>(
                "myMovie", isx::DataSet::MOVIE, fileName2);

        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        dataSet1->setParent(group);

        try
        {
            dataSet2->setParent(group);
            FAIL("Failed to throw an exception.");
        }
        catch (isx::ExceptionDataIO & error)
        {
            REQUIRE(std::string(error.what()) ==
                    "There is already an item with the name: myMovie");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionDataIO");
        }
    }

    //SECTION("Try to create a movie data sets with a file name that already exists")
    //{
    //    std::string fileName = g_resources["testDataPath"] + "/myMovie.isxd";
    //    std::remove(fileName.c_str());

    //    {
    //        std::ofstream outFile(fileName);
    //        outFile << "DataSetTest";
    //    }

    //    try
    //    {
    //        isx::SpDataSet_t dataSet1 = std::make_shared<isx::DataSet>(
    //            "myMovie", isx::DataSet::MOVIE, fileName);
    //        FAIL("Failed to throw an exception.");
    //    }
    //    catch (isx::ExceptionFileIO & error)
    //    {
    //        REQUIRE(std::string(error.what()) ==
    //                "The file name already exists: " + fileName);
    //    }
    //    catch (...)
    //    {
    //        FAIL("Failed to throw an isx::ExceptionDataIO");
    //    }
    //}
}
