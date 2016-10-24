#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxBehavMovieFile.h"
#include "isxLog.h"
#include "isxTest.h"
#include "catch.hpp"


#include <vector>

TEST_CASE("BehavMovieFile", "[core]") 
{
    std::string testFileName = g_resources["unitTestDataPath"] + "/trial9_OneSec.mpg";

    isx::CoreInitialize();

    SECTION("init from trimmed Noldus file")
    {
        isx::BehavMovieFile b(testFileName);
        
        REQUIRE(b.isValid());
        const isx::TimingInfo ti = b.getTimingInfo();
        REQUIRE(ti.getStart() == isx::Time());
        REQUIRE(ti.getStep() == isx::Ratio(1, 25));
        REQUIRE(ti.getNumTimes() == 16);
    }
    

    isx::CoreShutdown();
}
