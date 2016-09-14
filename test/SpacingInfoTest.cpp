#include "isxTest.h"
#include "isxSpacingInfo.h"
#include "catch.hpp"

TEST_CASE("SpacingInfoTest", "[core]")
{

    SECTION("Constructor with no arguments")
    {
        isx::SpacingInfo spacingInfo;
        REQUIRE(spacingInfo.isValid() == false);
    }

    SECTION("Constructor with all arguments")
    {
        isx::SizeInPixels_t numPixels(720, 540);
        isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE * 2, isx::DEFAULT_PIXEL_SIZE * 2);
        isx::PointInMicrons_t topLeft(0, 0);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        REQUIRE(spacingInfo.getNumPixels() == numPixels);
        REQUIRE(spacingInfo.getPixelSize() == pixelSize);
        REQUIRE(spacingInfo.getTopLeft() == topLeft);
    }

    SECTION("Get the total size")
    {
        isx::SizeInPixels_t numPixels(600, 500);
        isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE * 2, isx::DEFAULT_PIXEL_SIZE * 2);
        isx::PointInMicrons_t topLeft(22, 44);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        isx::SizeInMicrons_t totalSize(2640, 2200);
        REQUIRE(spacingInfo.getTotalSize() == totalSize);
    }

    SECTION("Get the bottom right corner")
    {
        isx::SizeInPixels_t numPixels(600, 500);
        isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE * 2, isx::DEFAULT_PIXEL_SIZE * 2);
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
        isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE * 2);
        isx::PointInMicrons_t topLeft(22, 44);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        std::string expected = "SpacingInfo(NumPixels=600 x 500, PixelSize=22 / 10 x 44 / 10, TopLeft=(22 / 1, 44 / 1))";
        REQUIRE(spacingInfo.toString() == expected);
    }

}

TEST_CASE("SpacingInfoTestConversion", "[core]")
{

    isx::SizeInPixels_t numPixels(8, 6);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(isx::Ratio(66, 10), isx::Ratio(44, 10));
    isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

    // Pixels to microns
    SECTION("Convert a point in pixels to a point in microns (x equals 0)")
    {
        isx::PointInPixels_t pointInPixels(0, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToMidPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(77, 10), isx::Ratio(121, 10)));
    }

    SECTION("Convert a point in pixels to a point in microns (y equals 0)")
    {
        isx::PointInPixels_t pointInPixels(4, 0);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToMidPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(165, 10), isx::Ratio(55, 10)));
    }

    SECTION("Convert a point in pixels to a point in microns (x equals max)")
    {
        isx::PointInPixels_t pointInPixels(8, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToMidPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(231, 10), isx::Ratio(121, 10)));
    }

    SECTION("Convert a point in pixels to a point in microns (y equals max)")
    {
        isx::PointInPixels_t pointInPixels(4, 6);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToMidPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(165, 10), isx::Ratio(165, 10)));
    }

    SECTION("Convert a point in pixels to a point in microns when there are no x samples")
    {
        numPixels = isx::SizeInPixels_t(0, 6);
        spacingInfo = isx::SpacingInfo(numPixels, pixelSize, topLeft);
        isx::PointInPixels_t pointInPixels(4, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToMidPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(77, 10), isx::Ratio(121, 10)));
    }

    SECTION("Convert a point in pixels to a point in microns when there are no y samples")
    {
        numPixels = isx::SizeInPixels_t(8, 0);
        spacingInfo = isx::SpacingInfo(numPixels, pixelSize, topLeft);
        isx::PointInPixels_t pointInPixels(4, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToMidPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(165, 10), isx::Ratio(55, 10)));
    }

    // Microns to pixels
    SECTION("Convert a point in microns to a point in pixels (x equals left)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(66, 10), isx::Ratio(121, 10));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(0, 3));
    }

    SECTION("Convert a point in microns to a point in pixels (y equals top)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(165, 10), isx::Ratio(44, 10));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(4, 0));
    }

    SECTION("Convert a point in microns to a point in pixels (x is closer to previous center)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(87, 10), isx::Ratio(121, 10));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(0, 3));
    }

    SECTION("Convert a point in microns to a point in pixels (x is closer to next center)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(88, 10), isx::Ratio(121, 10));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(1, 3));
    }

    SECTION("Convert a point in microns to a point in pixels (y is closer to previous center)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(165, 10), isx::Ratio(109, 10));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(4, 2));
    }

    SECTION("Convert a point in microns to a point in pixels (y is closer to next center)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(165, 10), isx::Ratio(110, 10));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(4, 3));
    }

    SECTION("Convert a point in microns to a point in pixels (x equals right)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(242, 10), isx::Ratio(121, 10));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(7, 3));
    }

    SECTION("Convert a point in microns to a point in pixels (y equals bottom)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(165, 10), isx::Ratio(176, 10));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(4, 5));
    }

    SECTION("Convert a point in microns to a point in pixels (no x samples)")
    {
        numPixels = isx::SizeInPixels_t(0, 6);
        spacingInfo = isx::SpacingInfo(numPixels, pixelSize, topLeft);
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(165, 10), isx::Ratio(121, 10));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(0, 3));
    }

    SECTION("Convert a point in microns to a point in pixels (no y samples)")
    {
        numPixels = isx::SizeInPixels_t(8, 0);
        spacingInfo = isx::SpacingInfo(numPixels, pixelSize, topLeft);
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(165, 10), isx::Ratio(121, 10));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(4, 0));
    }

}
