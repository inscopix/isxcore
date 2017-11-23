#include "isxVideoFrame.h"
#include "isxTest.h"
#include "catch.hpp"
#include "isxLog.h"

#include <vector>
#include <cstring>

TEST_CASE("VideoFrameTest", "[core]") {
    //std::string testFile = g_resources["unitTestDataPath"] + "/recording_20160426_145041.hdf5";

    SECTION("default constructor") 
    {
        isx::VideoFrame v;
        char * p = 0;
        REQUIRE(v.getWidth() == 0);
        REQUIRE(v.getPixels() == p);
        REQUIRE(v.getTimeStamp() == isx::Time());
        REQUIRE(v.getFrameIndex() == 0);
    }

    SECTION("constructor") 
    {
        const int32_t w = 10;
        const int32_t h = 10;
        const int32_t r = 128;
        const int32_t c = 6;
        const isx::Time t = isx::Time::now();
        const size_t f = 42;
        const isx::DataType dataType = isx::DataType::U16;
        const isx::SpacingInfo spacingInfo(isx::SizeInPixels_t(w, h));

        isx::VideoFrame v(spacingInfo, r, c, dataType, t, f);
        REQUIRE(v.getWidth() == w);
        REQUIRE(v.getDataType() == dataType);
        REQUIRE(v.getTimeStamp() == t);
        REQUIRE(v.getFrameIndex() == f);

        std::vector<uint16_t> buf(r * h);
        for (size_t j = 0; j < v.getImageSizeInBytes(); ++j)
        {
            buf[j] = (uint16_t) j;
        }

        // write, should have enough buffer space to not cause
        // access violation :)
        uint16_t * p = v.getPixelsAsU16();
        std::memcpy(p, &buf[0], buf.size());

        // read, check if data is the same as what was written
        REQUIRE(0 == std::memcmp(p, &buf[0], buf.size()));
    }

    SECTION("constructor with spacing information")
    {
        const int32_t r = 8640;
        const int32_t c = 3;
        const isx::DataType dataType = isx::DataType::U16;

        isx::SizeInPixels_t numPixels(1440, 1080);
        isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE * 2);
        isx::PointInMicrons_t topLeft(22, 44);
        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        isx::Time time;
        isx::VideoFrame v(spacingInfo, r, c, dataType, time, 0);

        REQUIRE(v.getImage().getSpacingInfo() == spacingInfo);
    }

}
