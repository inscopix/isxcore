#include "isxVideoFrame.h"
#include "isxTest.h"
#include "catch.hpp"
#include "isxLog.h"

#include <vector>

TEST_CASE("VideoFrameTest", "[core]") {
    //std::string testFile = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    SECTION("default constructor") 
    {
        isx::VideoFrame<uint32_t> v;
        uint32_t * p = 0;
        REQUIRE(v.getImage().getWidth() == 0);
        REQUIRE(v.getImage().getPixels() == p);
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

        isx::VideoFrame<uint16_t> v(w, h, r, c, t, f);
        REQUIRE(v.getImage().getWidth() == w);
        REQUIRE(v.getTimeStamp() == t);
        REQUIRE(v.getFrameIndex() == f);

        std::vector<uint16_t> buf(r * h);
        for (size_t j = 0; j < v.getImage().getImageSizeInBytes(); ++j)
        {
            buf[j] = (uint16_t) j;
        }

        // write, should have enough buffer space to not cause
        // access violation :)
        uint16_t * p = v.getImage().getPixels();
        memcpy(p, &buf[0], buf.size());

        // read, check if data is the same as what was written
        REQUIRE(0 == memcmp(p, &buf[0], buf.size()));
    }

}
