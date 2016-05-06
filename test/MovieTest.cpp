#include "isxRecording.h"
#include "isxMovie.h"
#include "catch.hpp"

TEST_CASE("MovieTest", "[core]") {

    SECTION("valid usage of constructor") {
        //! Tests valid usage of constructor.
        isx::Movie m;
        REQUIRE(!m.isValid());
    }

    SECTION("create movie from dataset in recording", "[core]") {
        //! Tests constructor from dataset in recording
        isx::tRecording_SP r = std::make_shared<isx::Recording>("test_data/recording_20160426_145041.hdf5");
        REQUIRE(r->isValid());
        isx::Movie m(r, "/images");
        REQUIRE(m.isValid());
    }
}
