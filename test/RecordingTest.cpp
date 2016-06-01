#include "isxRecording.h"
#include "isxMovie.h"
#include "catch.hpp"
#include "isxTest.h"

#include <string>
#include <map>


TEST_CASE("RecordingTest", "[core]") {
    std::string testFile = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    SECTION("default constructor") {
        isx::Recording r;
        REQUIRE(!r.isValid());
    }

    SECTION("create movie from dataset in recording", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
    }

}
