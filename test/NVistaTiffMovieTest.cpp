#include "isxNVistaTiffMovie.h"
#include "isxCore.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxRecording.h"


#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <numeric>

TEST_CASE("NVistaTiffMovieTest", "[core-internal]") {
    std::string testFileName = g_resources["unitTestDataPath"] + "/recording_20161104_145443.tif";
    std::string testFileNameXML = g_resources["unitTestDataPath"] + "/recording_20161104_145443.xml";

    isx::CoreInitialize();

    SECTION("create movie from one dataset") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaTiffMovie>(testFileName, testFileName);
        REQUIRE(m->isValid());
        REQUIRE(m->getDataType() == isx::DataType::U16);
    }

    SECTION("getTimingInfo().getNumTimes()") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaTiffMovie>(testFileName, testFileName);
        REQUIRE(m->isValid());
        REQUIRE(m->getTimingInfo().getNumTimes() == 39);
    }

    SECTION("getSpacingInfo().getNumColumns") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaTiffMovie>(testFileName, testFileName);
        REQUIRE(m->isValid());
        REQUIRE(m->getSpacingInfo().getNumColumns() == 1440);
    }

    SECTION("getSpacingInfo().getNumRows") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaTiffMovie>(testFileName, testFileName);
        REQUIRE(m->isValid());
        REQUIRE(m->getSpacingInfo().getNumRows() == 1080);
    }

    SECTION("getFrame for frame number") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaTiffMovie>(testFileName, testFileName);
        REQUIRE(m->isValid());
        isx::SpVideoFrame_t nvf = m->getFrame(0);
        unsigned char * t = reinterpret_cast<unsigned char *>(nvf->getPixels());
        REQUIRE(t[0] == 0xF2);
        REQUIRE(t[1] == 0x00);
    }

    SECTION("getTimingInfo().getDuration()") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaTiffMovie>(testFileName, testFileName);
        REQUIRE(m->isValid());
        REQUIRE(m->getTimingInfo().getDuration().toDouble() == 1.95);
    }

    SECTION("toString") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaTiffMovie>(testFileName, testFileName);
        REQUIRE(m->toString() == testFileName);
    }

    SECTION("Get frames corresponding to dropped frames", "[core]")
    {
        std::string testXML = g_resources["unitTestDataPath"] + "/recording_20161104_145443.xml";
        isx::SpRecording_t rXml  = std::make_shared<isx::Recording>(testXML);
        isx::SpMovie_t mov  = rXml->getMovie();

        isx::TimingInfo movTimingInfo = mov->getTimingInfo();
        std::vector<isx::isize_t> dropped = movTimingInfo.getDroppedFrames();

        REQUIRE(dropped.size());
        isx::SpVideoFrame_t vf = mov->getFrame(dropped[0]);

        REQUIRE(vf);
        uint16_t * pixels = vf->getPixelsAsU16();
        isx::isize_t totalNumPixels = mov->getSpacingInfo().getTotalNumPixels();
        std::vector<uint16_t> pixelVec(pixels, pixels + totalNumPixels);
        uint16_t sum = std::accumulate(pixelVec.begin(), pixelVec.end(), uint16_t(0));
        REQUIRE(sum == 0);
    }

    isx::CoreShutdown();
}

TEST_CASE("TiffMovieTestAsync", "[core]") {
    std::string testFileName = g_resources["unitTestDataPath"] + "/recording_20161104_145443.tif";

    isx::CoreInitialize();
    
    bool isDataCorrect = true;
    std::atomic_int doneCount(0);
    
    uint8_t f0_data[] = { 0xf2, 0x00, 0xec, 0x00, 0xea, 0x00, 0xeb, 0x00 };
    uint8_t f1_data[] = { 0xe4, 0x00, 0xeb, 0x00, 0xe7, 0x00, 0xdf, 0x00 };
    uint8_t f2_data[] = { 0xee, 0x00, 0xdf, 0x00, 0xe9, 0x00, 0xe8, 0x00 };
    uint8_t f3_data[] = { 0xeb, 0x00, 0xe2, 0x00, 0xe8, 0x00, 0xe3, 0x00 };
    uint8_t f4_data[] = { 0xe8, 0x00, 0xdf, 0x00, 0xe6, 0x00, 0xe0, 0x00 };
    
    std::vector<uint8_t *> expected = { f0_data, f1_data, f2_data, f3_data, f4_data };
    
    size_t numTestFrames = expected.size();
    size_t numTestBytesPerFrame = sizeof(f0_data);
    
    isx::SpMovie_t m = std::make_shared<isx::NVistaTiffMovie>(testFileName, testFileName);
    REQUIRE(m->isValid());
    isx::MovieGetFrameCB_t cb = [&](isx::AsyncTaskResult<isx::SpVideoFrame_t> inAsyncTaskResult){
        REQUIRE(!inAsyncTaskResult.getException());
        size_t index = inAsyncTaskResult.get()->getFrameIndex();
        unsigned char * t = reinterpret_cast<unsigned char *>(inAsyncTaskResult.get()->getPixels());
        if (memcmp(t, expected[index], numTestBytesPerFrame))
        {
            isDataCorrect = false;
        }
        ++doneCount;
    };

    SECTION("get frame for frame number asynchronously") {
        for (size_t i = 0; i < numTestFrames; ++i)
        {
            m->getFrameAsync(i, cb);
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
