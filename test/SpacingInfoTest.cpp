#include "isxTest.h"
#include "isxSpacingInfo.h"
#include "catch.hpp"

TEST_CASE("SpacingInfoTest", "[core]")
{

    SECTION("Constructor with no arguments")
    {
        isx::SpacingInfo spacingInfo;

        REQUIRE(spacingInfo.getNumPixels() == isx::Point<isx::isize_t>(1440, 1080));
        REQUIRE(spacingInfo.getPixelSize() == isx::Point<isx::Ratio>(isx::Ratio(22, 10), isx::Ratio(22, 10)));
        REQUIRE(spacingInfo.getTopLeft() == isx::Point<isx::Ratio>(0, 0));
    }

    SECTION("Constructor with all arguments")
    {
        isx::Point<isx::isize_t> numPixels(720, 540);
        isx::Point<isx::Ratio> pixelSize(isx::Ratio(44, 10), isx::Ratio(44, 10));
        isx::Point<isx::Ratio> topLeft(0, 0);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        REQUIRE(spacingInfo.getNumPixels() == numPixels);
        REQUIRE(spacingInfo.getPixelSize() == pixelSize);
        REQUIRE(spacingInfo.getTopLeft() == topLeft);
    }

    SECTION("Get the total size")
    {
        isx::Point<isx::isize_t> numPixels(600, 500);
        isx::Point<isx::Ratio> pixelSize(isx::Ratio(44, 10), isx::Ratio(44, 10));
        isx::Point<isx::Ratio> topLeft(22, 44);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        isx::Point<isx::Ratio> totalSize(2640, 2200);
        REQUIRE(spacingInfo.getTotalSize() == totalSize);
    }

    SECTION("Get the bottom right corner")
    {
        isx::Point<isx::isize_t> numPixels(600, 500);
        isx::Point<isx::Ratio> pixelSize(isx::Ratio(44, 10), isx::Ratio(44, 10));
        isx::Point<isx::Ratio> topLeft(22, 44);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        isx::Point<isx::Ratio> bottomRight(2662, 2244);
        REQUIRE(spacingInfo.getBottomRight() == bottomRight);
    }

    SECTION("Get the number of rows")
    {
        isx::Point<isx::isize_t> numPixels(600, 500);
        isx::Point<isx::Ratio> pixelSize(1, 1);
        isx::Point<isx::Ratio> topLeft(0, 0);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        REQUIRE(spacingInfo.getNumRows() == 500);
    }

    SECTION("Get the number of columns")
    {
        isx::Point<isx::isize_t> numPixels(600, 500);
        isx::Point<isx::Ratio> pixelSize(1, 1);
        isx::Point<isx::Ratio> topLeft(0, 0);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        REQUIRE(spacingInfo.getNumColumns() == 600);
    }

    SECTION("Convert to a string")
    {
        isx::Point<isx::isize_t> numPixels(600, 500);
        isx::Point<isx::Ratio> pixelSize(isx::Ratio(22, 10), isx::Ratio(44, 10));
        isx::Point<isx::Ratio> topLeft(isx::Ratio(22, 1), isx::Ratio(44, 1));

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        std::string expected = "SpacingInfo(NumPixels=(600, 500), PixelSize=(22 / 10, 44 / 10), TopLeft=(22 / 1, 44 / 1))";
        REQUIRE(spacingInfo.toString() == expected);
    }

}
