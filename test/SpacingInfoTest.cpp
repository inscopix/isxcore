#include "isxTest.h"
#include "isxSpacingInfo.h"
#include "catch.hpp"

TEST_CASE("SpacingInfoTest", "[core]")
{

    SECTION("Constructor with no arguments")
    {
        isx::SpacingInfo spacingInfo;

        REQUIRE(spacingInfo.getTopLeft() == isx::Point<isx::Ratio>(0, 0));
        REQUIRE(spacingInfo.getPixelSize() == isx::Point<isx::Ratio>(isx::Ratio(22, 10), isx::Ratio(22, 10)));
        REQUIRE(spacingInfo.getNumPixels() == isx::Point<size_t>(1440, 1080));
    }

    SECTION("Constructor with all arguments")
    {
        isx::Point<isx::Ratio> topLeft(0, 0);
        isx::Point<isx::Ratio> pixelSize(isx::Ratio(44, 10), isx::Ratio(44, 10));
        isx::Point<size_t> numPixels(720, 540);

        isx::SpacingInfo spacingInfo(topLeft, pixelSize, numPixels);

        REQUIRE(spacingInfo.getTopLeft() == topLeft);
        REQUIRE(spacingInfo.getPixelSize() == pixelSize);
        REQUIRE(spacingInfo.getNumPixels() == numPixels);
    }

    SECTION("Get the total size")
    {
        isx::Point<isx::Ratio> topLeft(22, 44);
        isx::Point<isx::Ratio> pixelSize(isx::Ratio(44, 10), isx::Ratio(44, 10));
        isx::Point<size_t> numPixels(600, 500);

        isx::SpacingInfo spacingInfo(topLeft, pixelSize, numPixels);

        isx::Point<isx::Ratio> totalSize(2640, 2200);
        REQUIRE(spacingInfo.getTotalSize() == totalSize);
    }

    SECTION("Get the bottom right corner")
    {
        isx::Point<isx::Ratio> topLeft(22, 44);
        isx::Point<isx::Ratio> pixelSize(isx::Ratio(44, 10), isx::Ratio(44, 10));
        isx::Point<size_t> numPixels(600, 500);

        isx::SpacingInfo spacingInfo(topLeft, pixelSize, numPixels);

        isx::Point<isx::Ratio> bottomRight(2662, 2244);
        REQUIRE(spacingInfo.getBottomRight() == bottomRight);
    }

}
