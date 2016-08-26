#include "isxDataSet.h"
#include "isxTest.h"
#include "isxException.h"
#include "isxGroup.h"

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
        isx::DataSet dataSet("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd");

        REQUIRE(dataSet.isValid());
        REQUIRE(dataSet.getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(dataSet.getName() == "myMovie");
        REQUIRE(!dataSet.getParent());
        REQUIRE(dataSet.getPath() == "myMovie");
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
