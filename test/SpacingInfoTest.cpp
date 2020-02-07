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

        isx::SizeInMicrons_t totalSize(3600, 3000);
        REQUIRE(spacingInfo.getTotalSize() == totalSize);
    }

    SECTION("Get the bottom right corner")
    {
        isx::SizeInPixels_t numPixels(600, 500);
        isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE * 2, isx::DEFAULT_PIXEL_SIZE * 2);
        isx::PointInMicrons_t topLeft(22, 44);

        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        isx::PointInMicrons_t bottomRight(3622, 3044);
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

        std::string expected = "SpacingInfo(NumPixels=600 x 500, PixelSize=3 / 1 x 6 / 1, TopLeft=(22 / 1, 44 / 1))";
        REQUIRE(spacingInfo.toString() == expected);
    }

}

TEST_CASE("SpacingInfoTestConversion", "[core]")
{

    isx::SizeInPixels_t numPixels(8, 6);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(isx::Ratio(9, 1), isx::Ratio(6, 1));
    isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

    // Pixels to mid point in microns
    SECTION("Convert a point in pixels to a mid point in microns (x equals 0)")
    {
        isx::PointInPixels_t pointInPixels(0, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToMidPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(21, 2), isx::Ratio(33, 2)));
    }

    SECTION("Convert a point in pixels to a point in microns (y equals 0)")
    {
        isx::PointInPixels_t pointInPixels(4, 0);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToMidPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(45, 2), isx::Ratio(15, 2)));
    }

    SECTION("Convert a point in pixels to a point in microns (x equals max)")
    {
        isx::PointInPixels_t pointInPixels(8, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToMidPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(63, 2), isx::Ratio(33, 2)));
    }

    SECTION("Convert a point in pixels to a point in microns (y equals max)")
    {
        isx::PointInPixels_t pointInPixels(4, 6);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToMidPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(45, 2), isx::Ratio(45, 2)));
    }

    SECTION("Convert a point in pixels to a point in microns when there are no x samples")
    {
        numPixels = isx::SizeInPixels_t(0, 6);
        spacingInfo = isx::SpacingInfo(numPixels, pixelSize, topLeft);
        isx::PointInPixels_t pointInPixels(4, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToMidPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(21, 2), isx::Ratio(33, 2)));
    }

    SECTION("Convert a point in pixels to a point in microns when there are no y samples")
    {
        numPixels = isx::SizeInPixels_t(8, 0);
        spacingInfo = isx::SpacingInfo(numPixels, pixelSize, topLeft);
        isx::PointInPixels_t pointInPixels(4, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToMidPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(45, 2), isx::Ratio(15, 2)));
    }

    // Pixels to top left corner in microns
    SECTION("Convert a point in pixels to the top left corner of that pixel in microns (x equals 0)")
    {
        isx::PointInPixels_t pointInPixels(0, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(9, 1), isx::Ratio(15, 1)));
    }

    SECTION("Convert a point in pixels to the top left corner of that pixel in microns (y equals 0)")
    {
        isx::PointInPixels_t pointInPixels(4, 0);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(21, 1), isx::Ratio(6, 1)));
    }

    SECTION("Convert a point in pixels to the top left corner of that pixel in microns (x equals max)")
    {
        isx::PointInPixels_t pointInPixels(8, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(30, 1), isx::Ratio(15, 1)));
    }

    SECTION("Convert a point in pixels to the top left corner of that pixel in microns (y equals max)")
    {
        isx::PointInPixels_t pointInPixels(4, 6);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(21, 1), isx::Ratio(21, 1)));
    }

    SECTION("Convert a point in pixels to the top left corner of that pixel in microns when there are no x samples")
    {
        numPixels = isx::SizeInPixels_t(0, 6);
        spacingInfo = isx::SpacingInfo(numPixels, pixelSize, topLeft);
        isx::PointInPixels_t pointInPixels(4, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(9, 1), isx::Ratio(15, 1)));
    }

    SECTION("Convert a point in pixels to the top left corner of that pixel in microns when there are no y samples")
    {
        numPixels = isx::SizeInPixels_t(8, 0);
        spacingInfo = isx::SpacingInfo(numPixels, pixelSize, topLeft);
        isx::PointInPixels_t pointInPixels(4, 3);
        isx::PointInMicrons_t pointInMicrons = spacingInfo.convertPixelsToPointInMicrons(pointInPixels);

        REQUIRE(pointInMicrons == isx::PointInMicrons_t(isx::Ratio(21, 1), isx::Ratio(6, 1)));
    }

    // Microns to pixels
    SECTION("Convert a point in microns to a point in pixels (x equals left)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(9, 1), isx::Ratio(33, 2));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(0, 3));
    }

    SECTION("Convert a point in microns to a point in pixels (y equals top)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(45, 2), isx::Ratio(6, 1));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(4, 0));
    }

    SECTION("Convert a point in microns to a point in pixels (x is closer to previous center)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(23, 2), isx::Ratio(33, 2));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(0, 3));
    }

    SECTION("Convert a point in microns to a point in pixels (x is closer to next center)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(24, 2), isx::Ratio(33, 2));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(1, 3));
    }

    SECTION("Convert a point in microns to a point in pixels (y is closer to previous center)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(45, 2), isx::Ratio(29, 2));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(4, 2));
    }

    SECTION("Convert a point in microns to a point in pixels (y is closer to next center)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(45, 2), isx::Ratio(30, 2));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(4, 3));
    }

    SECTION("Convert a point in microns to a point in pixels (x equals right)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(33, 1), isx::Ratio(33, 2));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(7, 3));
    }

    SECTION("Convert a point in microns to a point in pixels (y equals bottom)")
    {
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(45, 2), isx::Ratio(24, 1));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(4, 5));
    }

    SECTION("Convert a point in microns to a point in pixels (no x samples)")
    {
        numPixels = isx::SizeInPixels_t(0, 6);
        spacingInfo = isx::SpacingInfo(numPixels, pixelSize, topLeft);
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(45, 2), isx::Ratio(33, 2));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(0, 3));
    }

    SECTION("Convert a point in microns to a point in pixels (no y samples)")
    {
        numPixels = isx::SizeInPixels_t(8, 0);
        spacingInfo = isx::SpacingInfo(numPixels, pixelSize, topLeft);
        isx::PointInMicrons_t pointInMicrons(isx::Ratio(45, 2), isx::Ratio(33, 2));
        isx::PointInPixels_t pointInPixels = spacingInfo.convertMidPointInMicronsToPixels(pointInMicrons);

        REQUIRE(pointInPixels == isx::PointInPixels_t(4, 0));
    }

}
