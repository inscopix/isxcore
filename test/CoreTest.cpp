#include "isxCore.h"
#include "catch.hpp"
#include "isxLog.h"

TEST_CASE("Core", "[core]")
{
    isx::CoreInitialize();

    SECTION("initialize")
    {
        REQUIRE(isx::CoreIsInitialized());
        isx::CoreShutdown();
        REQUIRE(!isx::CoreIsInitialized());
        isx::CoreInitialize();
    }

    isx::CoreShutdown();
}

TEST_CASE("versionAtLeast", "[core]")
{
    SECTION("2.1.1 >= 1.1.1")
    {
        REQUIRE(isx::versionAtLeast("2.1.1-aabbccd", 1, 1, 1));
    }

    SECTION("1.2.1 >= 1.1.1")
    {
        REQUIRE(isx::versionAtLeast("1.2.1-aabbccd", 1, 1, 1));
    }

    SECTION("1.1.2 >= 1.1.1")
    {
        REQUIRE(isx::versionAtLeast("1.1.2-aabbccd", 1, 1, 1));
    }

    SECTION("1.1.1 >= 1.1.1")
    {
        REQUIRE(isx::versionAtLeast("1.1.1-aabbccd", 1, 1, 1));
    }

    SECTION("1.1.0 >= 1.1.1")
    {
        REQUIRE(!isx::versionAtLeast("1.1.0-aabbccd", 1, 1, 1));
    }
}
