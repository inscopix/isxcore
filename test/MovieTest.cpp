#include "isxMovie.h"
#include "catch.hpp"

TEST_CASE("MovieTest", "[core]") {

    SECTION("valid usage of constructor") {
        //! Tests valid usage of constructor.
        isx::Movie movie;
        REQUIRE(true);
    }
}
