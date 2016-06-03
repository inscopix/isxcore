#include "isxImage.h"
#include "isxTime.h"
#include "isxTest.h"
#include "catch.hpp"
#include "isxLog.h"

#include <vector>

TEST_CASE("ImageTest", "[core]") {
    //std::string testFile = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    SECTION("default constructor") 
    {
        isx::Image i;
        REQUIRE(i.getFormat() == isx::Image::FORMAT_UNDEFINED);
        REQUIRE(i.getRowBytes() == 0);
        REQUIRE(i.getWidth() == 0);
        REQUIRE(i.getHeight() == 0);
        REQUIRE(i.getPixelSizeInBytes() == 0);
        REQUIRE(i.getImageSizeInBytes() == 0);
        REQUIRE(i.hasTimeStamp() == false);
        REQUIRE(i.getTimeStamp() == isx::Time());
        REQUIRE(i.getPixels() == 0);
    }

    SECTION("constructor") 
    {
        const int32_t w = 10;
        const int32_t h = 10;
        const int32_t r = 32;

        isx::Image i(isx::Image::FORMAT_UINT16_1, w, h, r);
        REQUIRE(i.getFormat() == isx::Image::FORMAT_UINT16_1);
        REQUIRE(i.getRowBytes() == r);
        REQUIRE(i.getWidth() == w);
        REQUIRE(i.getHeight() == h);
        REQUIRE(i.getPixelSizeInBytes() == 2);
        REQUIRE(i.getImageSizeInBytes() == r * h);
        REQUIRE(i.hasTimeStamp() == false);
        REQUIRE(i.getTimeStamp() == isx::Time());

        std::vector<uint8_t> buf(r * h);
        for (int j = 0; j < i.getImageSizeInBytes(); ++j)
        {
            buf[j] = (uint8_t) j;
        }

        // write, should have enough buffer space to not cause
        // access violation :)
        uint8_t * p = (uint8_t *) i.getPixels();
        memcpy(p, &buf[0], buf.size());

        // read, check if data is the same as what was written
        REQUIRE(0 == memcmp(p, &buf[0], buf.size()));
    }

    SECTION("constructor with time stamp")
    {
        const int32_t w = 10;
        const int32_t h = 10;
        const int32_t r = 32;

        isx::Time t = isx::Time::now();

        isx::Image i(isx::Image::FORMAT_UINT16_1, w, h, r, t);
        REQUIRE(i.getFormat() == isx::Image::FORMAT_UINT16_1);
        REQUIRE(i.getRowBytes() == r);
        REQUIRE(i.getWidth() == w);
        REQUIRE(i.getHeight() == h);
        REQUIRE(i.getPixelSizeInBytes() == 2);
        REQUIRE(i.getImageSizeInBytes() == r * h);
        REQUIRE(i.hasTimeStamp() == true);
        REQUIRE(i.getTimeStamp() == t);
    }

}
