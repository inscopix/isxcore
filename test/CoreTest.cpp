#include "isxCore.h"
#include "catch.hpp"
#include "isxLog.h"

TEST_CASE("Core", "[core]") {

    isx::CoreInitialize();

    SECTION("initialize") {
        REQUIRE(isx::CoreIsInitialized());
        isx::CoreShutdown();
        REQUIRE(!isx::CoreIsInitialized());
        isx::CoreInitialize();
    }

    isx::CoreShutdown();
}
