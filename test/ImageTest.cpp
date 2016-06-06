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
        isx::Image<uint32_t> i;
        REQUIRE(i.getWidth() == 0);
        REQUIRE(i.getHeight() == 0);
        REQUIRE(i.getRowBytes() == 0);
        REQUIRE(i.getNumChannels() == 0);
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
        const int32_t r = 128;
        const int32_t c = 6;

        isx::Image<uint16_t> i(w, h, r, c);
        REQUIRE(i.getWidth() == w);
        REQUIRE(i.getHeight() == h);
        REQUIRE(i.getRowBytes() == r);
        REQUIRE(i.getNumChannels() == c);
        REQUIRE(i.getPixelSizeInBytes() == 2 * c);
        REQUIRE(i.getImageSizeInBytes() == r * h);
        REQUIRE(i.hasTimeStamp() == false);
        REQUIRE(i.getTimeStamp() == isx::Time());

        std::vector<uint16_t> buf(r * h);
        for (int j = 0; j < i.getImageSizeInBytes(); ++j)
        {
            buf[j] = (uint16_t) j;
        }

        // write, should have enough buffer space to not cause
        // access violation :)
        uint16_t * p = (uint16_t *) i.getPixels();
        memcpy(p, &buf[0], buf.size());

        // read, check if data is the same as what was written
        REQUIRE(0 == memcmp(p, &buf[0], buf.size()));
    }

    SECTION("constructor with time stamp")
    {
        const int32_t w = 10;
        const int32_t h = 10;
        const int32_t r = 32;
        const int32_t c = 1;

        isx::Time t = isx::Time::now();

        isx::Image<uint16_t> i(w, h, r, c, t);
        REQUIRE(i.getWidth() == w);
        REQUIRE(i.getHeight() == h);
        REQUIRE(i.getRowBytes() == r);
        REQUIRE(i.getNumChannels() == c);
        REQUIRE(i.getPixelSizeInBytes() == 2 * c);
        REQUIRE(i.getImageSizeInBytes() == r * h);
        REQUIRE(i.hasTimeStamp() == true);
        REQUIRE(i.getTimeStamp() == t);
    }

}
