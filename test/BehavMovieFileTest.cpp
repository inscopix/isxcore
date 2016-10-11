#include "isxCore.h"
#include "isxCoreFwd.h"
#include "catch.hpp"
#include "isxLog.h"

#include "isxBehavMovieFile.h"

TEST_CASE("BehavMovieFile", "[core]") 
{

    isx::CoreInitialize();

    SECTION("init from Noldus file") 
    {
        isx::BehavMovieFile b("/Users/aschildan/Documents/Trial9.mpg");
        b.readFrame(0);
    }

    isx::CoreShutdown();
}
