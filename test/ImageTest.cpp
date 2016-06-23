#include "isxImage.h"
#include "isxTest.h"
#include "catch.hpp"
#include "isxLog.h"

#include <vector>

TEST_CASE("ImageTest", "[core]") {
    //std::string testFile = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    SECTION("default constructor") 
    {
        isx::Image<uint32_t> i;
        uint32_t * p = 0;
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

        isx::Image<uint16_t> i(w, h, r, c);
        REQUIRE(i.getWidth() == w);
        REQUIRE(i.getHeight() == h);
        REQUIRE(i.getRowBytes() == r);
        REQUIRE(i.getNumChannels() == c);
        REQUIRE(i.getPixelSizeInBytes() == 2 * c);
        REQUIRE(i.getImageSizeInBytes() == r * h);

        std::vector<uint16_t> buf(r * h);
        for (size_t j = 0; j < i.getImageSizeInBytes(); ++j)
        {
            buf[j] = (uint16_t) j;
        }

        // write, should have enough buffer space to not cause
        // access violation :)
        uint16_t * p = i.getPixels();
        memcpy(p, &buf[0], buf.size());

        // read, check if data is the same as what was written
        REQUIRE(0 == memcmp(p, &buf[0], buf.size()));
    }

    SECTION("constructor with spacing information")
    {
        const int32_t r = 8640;
        const int32_t c = 3;

        isx::Point<isx::Ratio> topLeft(22, 44);
        isx::Point<isx::Ratio> pixelSize(isx::Ratio(22, 10), isx::Ratio(44, 10));
        isx::Point<size_t> numPixels(1440, 1080);
        isx::SpacingInfo spacingInfo(topLeft, pixelSize, numPixels);

        isx::Image<uint16_t> i(spacingInfo, r, c);

        REQUIRE(i.getSpacingInfo() == spacingInfo);
    }

}
