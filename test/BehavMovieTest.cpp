#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxMovieFactory.h"
#include "isxLog.h"
#include "isxTest.h"
#include "catch.hpp"


#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstring>

TEST_CASE("BehavMovie", "[core]") 
{
    std::string testFileName = g_resources["unitTestDataPath"] + "/trial9_OneSec.mpg";

    isx::CoreInitialize();

    SECTION("read async and verify frame data")
    {
        bool isDataCorrect = true;
        std::atomic_int doneCount(0);

        uint8_t f0_data[] = { 0x1F, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F };
        uint8_t f1_data[] = { 0x1F, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F };
        uint8_t f2_data[] = { 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F };

        std::vector<uint8_t *> expected = { f0_data, f1_data, f2_data };

        size_t numTestFrames = expected.size();
        size_t numTestBytesPerFrame = sizeof(f0_data);
        
        isx::DataSet::Properties props;
        props[isx::DataSet::PROP_MOVIE_START_TIME]  = isx::Variant(isx::Time());
        props[isx::DataSet::PROP_BEHAV_GOP_SIZE]    = isx::Variant(int64_t(10));
        props[isx::DataSet::PROP_BEHAV_NUM_FRAMES]  = isx::Variant(int64_t(16));

        isx::SpMovie_t movie = isx::readBehavioralMovie(testFileName, props);

        isx::MovieGetFrameCB_t cb = [&](isx::AsyncTaskResult<isx::SpVideoFrame_t> inAsyncTaskResult){
            REQUIRE(!inAsyncTaskResult.getException());
            size_t index = inAsyncTaskResult.get()->getFrameIndex();
            unsigned char * t = reinterpret_cast<unsigned char *>(inAsyncTaskResult.get()->getPixels());
            if (std::memcmp(t, expected[index], numTestBytesPerFrame))
            {
                isDataCorrect = false;
            }
            ++doneCount;
        };

        for (size_t i = 0; i < numTestFrames; ++i)
        {
            movie->getFrameAsync(i, cb);
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numTestFrames))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == numTestFrames);
        REQUIRE(isDataCorrect);
        
    }

    isx::CoreShutdown();
}
