#include "isxDataSet.h"
#include "isxTest.h"
#include "isxException.h"
#include "isxGroup.h"

#include "catch.hpp"

TEST_CASE("DataSetTest", "[core]")
{

    SECTION("Empty constructor")
    {
        isx::SpDataSet_t dataSet = std::make_shared<isx::DataSet>();

        REQUIRE(!dataSet->isValid());
    }

    SECTION("Construct a movie data set")
    {
        isx::SpDataSet_t dataSet = std::make_shared<isx::DataSet>(
                "myMovie", isx::DataSet::MOVIE, "myMovie.isxd");

        REQUIRE(dataSet->isValid());
        REQUIRE(dataSet->getType() == isx::DataSet::MOVIE);
        REQUIRE(dataSet->getName() == "myMovie");
        REQUIRE(!dataSet->getParent());
        REQUIRE(dataSet->getPath() == "myMovie");
    }

    SECTION("Construct a movie data set then set parent group")
    {
        isx::SpDataSet_t dataSet = std::make_shared<isx::DataSet>(
                "myMovie", isx::DataSet::MOVIE, "myMovie.isxd");
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");

        dataSet->setParent(group);

        REQUIRE(dataSet->getParent());
        REQUIRE(dataSet->getParent() == group);
        REQUIRE(dataSet->getPath() == "myGroup/myMovie");
    }

}
