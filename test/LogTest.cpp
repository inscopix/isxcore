#include "isxLog.h"
#include "isxTest.h"
#include "catch.hpp"

TEST_CASE("LogTest", "[core]")
{

    SECTION("logging general message to string stream") {
        std::ostringstream strm;
        isx::Log::instance()->ostream(strm);
        std::string msg = "log general message";

        isx::log() << msg;

        REQUIRE(strm.str() == msg);
    }

    SECTION("logging info message to string stream") {
        std::ostringstream strm;
        isx::Log::instance()->ostream(strm);
        std::string msg = "log info message";

        isx::info() << msg;

        REQUIRE(strm.str() == "INFO: " + msg);
    }

    SECTION("logging warning message to string stream") {
        std::ostringstream strm;
        isx::Log::instance()->ostream(strm);
        std::string msg = "log warning message";

        isx::warning() << msg;

        REQUIRE(strm.str() == "WARNING: " + msg);
    }

    SECTION("logging error message to string stream") {
        std::ostringstream strm;
        isx::Log::instance()->ostream(strm);
        std::string msg = "log warning message";

        isx::error() << msg;

        REQUIRE(strm.str() == "ERROR: " + msg);
    }

}
