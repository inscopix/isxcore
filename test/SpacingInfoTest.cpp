#include "isxTest.h"
#include "isxSpacingInfo.h"
#include "catch.hpp"

TEST_CASE("SpacingInfoTest", "[core]")
{

    SECTION("Constructor with no arguments")
    {
        isx::SpacingInfo spacingInfo;

        REQUIRE(spacingInfo.getNumPixels() == isx::SizeInPixels_t(1440, 1080));
        REQUIRE(spacingInfo.getPixelSize() == isx::SizeInMicrons_t(isx::Ratio(22, 10), isx::Ratio(22, 10)));
        REQUIRE(spacingInfo.getTopLeft() == isx::PointInMicrons_t(0, 0));
    }

    SECTION("Constructor with all arguments")
    {
        isx::SizeInPixels_t numPixels(720, 540);
        isx::SizeInMicrons_t pixelSize(isx::Ratio(44, 10), isx::Ratio(44, 10));
        isx::PointInMicrons_t topLeft(0, 0);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        REQUIRE(spacingInfo.getNumPixels() == numPixels);
        REQUIRE(spacingInfo.getPixelSize() == pixelSize);
        REQUIRE(spacingInfo.getTopLeft() == topLeft);
    }

    SECTION("Get the total size")
    {
        isx::SizeInPixels_t numPixels(600, 500);
        isx::SizeInMicrons_t pixelSize(isx::Ratio(44, 10), isx::Ratio(44, 10));
        isx::PointInMicrons_t topLeft(22, 44);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        isx::SizeInMicrons_t totalSize(2640, 2200);
        REQUIRE(spacingInfo.getTotalSize() == totalSize);
    }

    SECTION("Get the bottom right corner")
    {
        isx::SizeInPixels_t numPixels(600, 500);
        isx::SizeInMicrons_t pixelSize(isx::Ratio(44, 10), isx::Ratio(44, 10));
        isx::PointInMicrons_t topLeft(22, 44);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        isx::PointInMicrons_t bottomRight(2662, 2244);
        REQUIRE(spacingInfo.getBottomRight() == bottomRight);
    }

    SECTION("Get the number of rows")
    {
        isx::SizeInPixels_t numPixels(600, 500);
        isx::SizeInMicrons_t pixelSize(1, 1);
        isx::PointInMicrons_t topLeft(0, 0);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        REQUIRE(spacingInfo.getNumRows() == 500);
    }

    SECTION("Get the number of columns")
    {
        isx::SizeInPixels_t numPixels(600, 500);
        isx::SizeInMicrons_t pixelSize(1, 1);
        isx::PointInMicrons_t topLeft(0, 0);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        REQUIRE(spacingInfo.getNumColumns() == 600);
    }

    SECTION("Convert to a string")
    {
        isx::SizeInPixels_t numPixels(600, 500);
        isx::SizeInMicrons_t pixelSize(isx::Ratio(22, 10), isx::Ratio(44, 10));
        isx::PointInMicrons_t topLeft(22, 44);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        std::string expected = "SpacingInfo(NumPixels=(600, 500), PixelSize=(22 / 10, 44 / 10), TopLeft=(22 / 1, 44 / 1))";
        REQUIRE(spacingInfo.toString() == expected);
    }

}

TEST_CASE("SpacingInfoTestConversion", "[core]")
{

    isx::SizeInPixels_t numPixels(8, 6);
    isx::SizeInMicrons_t pixelSize(isx::Ratio(22, 10), isx::Ratio(22, 10));
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

    SECTION("Convert a point in pixels to a point in microns (x and y equal 0)")
    {
        isx::PointInPixels_t pointInPixels(0, 0);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPointInPixelsToMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(11, 10), isx::Ratio(11, 10)));
    }

    SECTION("Convert a point in pixels to a point in microns (x equals 0)")
    {
        isx::PointInPixels_t pointInPixels(0, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPointInPixelsToMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(11, 10), isx::Ratio(77, 10)));
    }

    SECTION("Convert a point in pixels to a point in microns (y equals 0)")
    {
        isx::PointInPixels_t pointInPixels(4, 0);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPointInPixelsToMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(99, 10), isx::Ratio(11, 10)));
    }

    SECTION("Convert a point in pixels to a point in microns (x and y within samples)")
    {
        isx::PointInPixels_t pointInPixels(1, 4);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPointInPixelsToMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(33, 10), isx::Ratio(99, 10)));
    }

    SECTION("Convert a point in pixels to a point in microns (x exceeds samples)")
    {
        isx::PointInPixels_t pointInPixels(8, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPointInPixelsToMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(165, 10), isx::Ratio(77, 10)));
    }

    SECTION("Convert a point in pixels to a point in microns (y exceeds samples)")
    {
        isx::PointInPixels_t pointInPixels(4, 6);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPointInPixelsToMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(99, 10), isx::Ratio(121, 10)));
    }

}
