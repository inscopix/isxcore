#include "isxImage.h"
#include "isxTest.h"
#include "catch.hpp"
#include "isxLog.h"

#include <vector>

TEST_CASE("ImageTest", "[core]") {
    //std::string testFile = g_resources["unitTestDataPath"] + "/recording_20160426_145041.hdf5";

    SECTION("default constructor") 
    {
        isx::Image i;
        char * p = 0;
        REQUIRE(i.getWidth() == 0);
        REQUIRE(i.getHeight() == 0);
        REQUIRE(i.getRowBytes() == 0);
        REQUIRE(i.getNumChannels() == 0);
        REQUIRE(i.getPixelSizeInBytes() == 0);
        REQUIRE(i.getImageSizeInBytes() == 0);
        REQUIRE(i.getPixels() == p);
    }

    SECTION("constructor") 
    {
        const isx::isize_t w = 10;
        const isx::isize_t h = 10;
        const isx::isize_t r = 128;
        const isx::isize_t c = 6;
        const isx::DataType dataType = isx::DataType::U16;
        const isx::SpacingInfo spacingInfo(isx::SizeInPixels_t(w, h));

        isx::Image i(spacingInfo, r, c, dataType);
        REQUIRE(i.getWidth() == w);
        REQUIRE(i.getHeight() == h);
        REQUIRE(i.getRowBytes() == r);
        REQUIRE(i.getNumChannels() == c);
        REQUIRE(i.getDataType() == dataType);
        REQUIRE(i.getPixelSizeInBytes() == 2 * c);
        REQUIRE(i.getImageSizeInBytes() == r * h);

        std::vector<uint16_t> buf(r * h);
        for (size_t j = 0; j < i.getImageSizeInBytes(); ++j)
        {
            buf[j] = (uint16_t) j;
        }

        // write, should have enough buffer space to not cause
        // access violation :)
        uint16_t * p = i.getPixelsAsU16();
        memcpy(p, &buf[0], buf.size());

        // read, check if data is the same as what was written
        REQUIRE(0 == memcmp(p, &buf[0], buf.size()));
    }

    SECTION("constructor with spacing information")
    {
        const int32_t r = 8640;
        const int32_t c = 3;

        isx::SizeInPixels_t numPixels(1440, 1080);
        isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
        isx::PointInMicrons_t topLeft(22, 44);
        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        isx::Image i(spacingInfo, r, c, isx::DataType::U16);

        REQUIRE(i.getSpacingInfo() == spacingInfo);
    }

}
