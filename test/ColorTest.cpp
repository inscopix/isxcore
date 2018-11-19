#include "isxColor.h"
#include "catch.hpp"
#include "isxLog.h"

TEST_CASE("Color", "[core][color]")
{
    const uint8_t r = 234;
    const uint8_t g = 29;
    const uint8_t b = 91;
    const uint8_t a = 0;

    isx::Color c(r, g, b, a);

    REQUIRE(c.getRed() == r);
    REQUIRE(c.getGreen() == g);
    REQUIRE(c.getBlue() == b);
    REQUIRE(!c.isGray());
}
