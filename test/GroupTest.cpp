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
        REQUIRE(!group->hasParent());
    }

    SECTION("Construct a group then set parent group")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        isx::SpGroup_t subGroup = std::make_shared<isx::Group>("mySubGroup");

        subGroup->setParent(group);

        REQUIRE(group->isValid());
        REQUIRE(group->getName() == "myGroup");
        REQUIRE(!group->hasParent());

        REQUIRE(subGroup->isValid());
        REQUIRE(subGroup->getName() == "mySubGroup");
        REQUIRE(*(subGroup->getParent()) == *group);
    }

    SECTION("Create two groups with the same name in a parent group")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        isx::SpGroup_t subGroup1 = std::make_shared<isx::Group>("mySubGroup");
        isx::SpGroup_t subGroup2 = std::make_shared<isx::Group>("mySubGroup");

        subGroup1->setParent(group);

        try
        {
            subGroup2->setParent(group);
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
        isx::SpGroup_t subGroup1 = std::make_shared<isx::Group>("mySubGroup1");
        isx::SpGroup_t subGroup2 = std::make_shared<isx::Group>("mySubGroup2");

        group->addGroup(subGroup1);
        group->addGroup(subGroup2);

        isx::SpGroup_t actualSubGroup = group->getGroup("mySubGroup1");

        REQUIRE(*actualSubGroup == *subGroup1);
    }

    SECTION("Remove a group by name")
    {
        isx::SpGroup_t group = std::make_shared<isx::Group>("myGroup");
        isx::SpGroup_t subGroup1 = std::make_shared<isx::Group>("mySubGroup1");
        isx::SpGroup_t subGroup2 = std::make_shared<isx::Group>("mySubGroup2");

        group->addGroup(subGroup1);
        group->addGroup(subGroup2);

        group->removeGroup("mySubGroup1");

        REQUIRE(group->getGroups().size() == 1);

        isx::SpGroup_t actualSubGroup = group->getGroup("mySubGroup2");

        REQUIRE(*actualSubGroup == *subGroup2);
    }

}
