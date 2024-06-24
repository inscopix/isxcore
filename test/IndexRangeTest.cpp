#include "isxTest.h"
#include "isxIndexRange.h"

#include "catch.hpp"

TEST_CASE("IndexRange", "[core]")
{
    SECTION("Default constructor")
    {
        isx::IndexRange range;
        REQUIRE(range.m_first == 0);
        REQUIRE(range.m_last == 0);
    }

    SECTION("Single index constructor")
    {
        isx::IndexRange range(5);
        REQUIRE(range.m_first == 5);
        REQUIRE(range.m_last == 5);
    }

    SECTION("First and last index constructor")
    {
        isx::IndexRange range(5, 8);
        REQUIRE(range.m_first == 5);
        REQUIRE(range.m_last == 8);
    }

    SECTION("Conversion to and from string with unitary range")
    {
        isx::IndexRange range(5);
        REQUIRE(range == isx::IndexRange(range.toString()));
    }

    SECTION("Conversion to and from string with non-unitary range")
    {
        isx::IndexRange range(5, 8);
        REQUIRE(range == isx::IndexRange(range.toString()));
    }

    SECTION("Contains")
    {
        isx::IndexRange range(5, 8);
        REQUIRE(!range.contains(4));
        REQUIRE(range.contains(5));
        REQUIRE(range.contains(6));
        REQUIRE(range.contains(7));
        REQUIRE(range.contains(8));
        REQUIRE(!range.contains(9));
    }
}
