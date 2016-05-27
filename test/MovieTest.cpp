#include "isxRecording.h"
#include "isxMovie.h"
#include "catch.hpp"

#include "isxTest.h"

#include <vector>

TEST_CASE("MovieTest", "[core]") {
    std::string testFile = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    SECTION("default constructor") {
        isx::Movie m;
        REQUIRE(!m.isValid());
    }

    SECTION("create movie from dataset in recording", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r, "/images");
        REQUIRE(m.isValid());
    }

    SECTION("getNumFrames", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r, "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getNumFrames() == 33);
    }

    SECTION("getFrameWidth", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r, "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getFrameWidth() == 500);
    }

    SECTION("getFrameHeight", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r, "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getFrameHeight() == 500);
    }

    SECTION("getFrameSizeInBytes", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r, "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getFrameSizeInBytes() == 500000);
    }

    SECTION("getFrame", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r, "/images");
        REQUIRE(m.isValid());
        size_t s = m.getFrameSizeInBytes();
        std::vector<unsigned char> t(s);
        m.getFrame(0, &t[0], s);
        REQUIRE(t[0] == 0x43);
        REQUIRE(t[1] == 0x3);
    }

    SECTION("getDurationInSeconds", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r, "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getDurationInSeconds() == 1.1);
    }

}
