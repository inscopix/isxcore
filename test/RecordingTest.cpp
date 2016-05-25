#include "isxRecording.h"
#include "isxMovie.h"
#include "catch.hpp"
#include "isxTest.h"

#include <string>
#include <map>


TEST_CASE("RecordingTest", "[core]") {
    std::string testFileStr = g_Resources["testDataPath"] + "/recording_20160426_145041.hdf5";
    const char * testFile = testFileStr.c_str();

    SECTION("default constructor") {
        isx::Recording r;
        REQUIRE(!r.isValid());
    }

    SECTION("create movie from dataset in recording", "[core]") {
        isx::tRecording_SP r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r, "/images");
        REQUIRE(m.isValid());
    }

}
