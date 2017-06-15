#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxBehavMovieFile.h"
#include "isxDataSet.h"
#include "isxLog.h"
#include "isxTest.h"

#include "catch.hpp"


#include <vector>

TEST_CASE("BehavMovieFile", "[core]") 
{
    std::string testFileName = g_resources["unitTestDataPath"] + "/trial9_OneSec.mpg";

    isx::CoreInitialize();

    SECTION("Get properties from trimmed Noldus file")
    {
        isx::DataSet::Properties props;
        bool res = isx::BehavMovieFile::getBehavMovieProperties(testFileName, props);

        int64_t gopSize = -1;
        int64_t numFrames = -1;

        if (props.find(isx::DataSet::PROP_BEHAV_GOP_SIZE) != props.end())
        {
            auto t = props.at(isx::DataSet::PROP_BEHAV_GOP_SIZE);
            gopSize = t.value<int64_t>();
        }
        if (props.find(isx::DataSet::PROP_BEHAV_NUM_FRAMES) != props.end())
        {
            auto t = props.at(isx::DataSet::PROP_BEHAV_NUM_FRAMES);
            numFrames = t.value<int64_t>();
        }
        
        REQUIRE(res);
        REQUIRE(gopSize == 10);
        REQUIRE(numFrames == 16);
    }

    SECTION("Instantiate Behavioral Movie for trimmed Noldus file given properties")
    {
        isx::DataSet::Properties props;
        props[isx::DataSet::PROP_MOVIE_START_TIME] = isx::Variant{isx::Time()};
        props[isx::DataSet::PROP_BEHAV_GOP_SIZE] = isx::Variant{int64_t(10)};
        props[isx::DataSet::PROP_BEHAV_NUM_FRAMES] = isx::Variant{int64_t(16)};

        isx::BehavMovieFile b(testFileName, props);

        REQUIRE(b.isValid());
        const isx::TimingInfo ti = b.getTimingInfo();
        REQUIRE(ti.getStart() == isx::Time());
        REQUIRE(ti.getStep() == isx::Ratio(643, 16000));
        REQUIRE(ti.getNumTimes() == 16);
    }

    isx::CoreShutdown();
}
