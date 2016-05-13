#include "isxRecording.h"
#include "isxMovie.h"
#include "catch.hpp"

TEST_CASE("RecordingTest", "[core]") {
    const char * testFile = "test_data/recording_20160426_145041.hdf5";

    SECTION("default constructor") {
        isx::Recording r;
        REQUIRE(!r.isValid());
    }

    SECTION("create movie from dataset in recording", "[core]") {
        isx::tRecording_SP r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r, "/image");
        REQUIRE(m.isValid());
    }

}
