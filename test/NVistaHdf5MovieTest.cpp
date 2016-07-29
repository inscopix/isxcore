#include "isxNVistaHdf5Movie.h"
#include "isxCore.h"
#include "catch.hpp"
#include "isxTest.h"

#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

TEST_CASE("NVistaHdf5MovieTest", "[core-internal]") {
    std::string testFileName = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    std::shared_ptr<H5::H5File> h5File = std::make_shared<H5::H5File>(testFileName, H5F_ACC_RDONLY);
    isx::SpHdf5FileHandle_t testFile = std::make_shared<isx::Hdf5FileHandle>(h5File, H5F_ACC_RDONLY);

    isx::CoreInitialize();

    SECTION("create movie from one dataset") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaHdf5Movie>(testFile);
        REQUIRE(m->isValid());
    }

    SECTION("getTimingInfo().getNumTimes()") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaHdf5Movie>(testFile);
        REQUIRE(m->isValid());
        REQUIRE(m->getTimingInfo().getNumTimes() == 33);
    }

    SECTION("getSpacingInfo().getNumColumns") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaHdf5Movie>(testFile);
        REQUIRE(m->isValid());
        REQUIRE(m->getSpacingInfo().getNumColumns() == 500);
    }

    SECTION("getSpacingInfo().getNumRows") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaHdf5Movie>(testFile);
        REQUIRE(m->isValid());
        REQUIRE(m->getSpacingInfo().getNumRows() == 500);
    }

    SECTION("getFrame for time") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaHdf5Movie>(testFile);
        REQUIRE(m->isValid());
        auto nvf = m->getFrame(m->getTimingInfo().getStart());
        unsigned char * t = reinterpret_cast<unsigned char *>(nvf->getPixels());
        REQUIRE(t[0] == 0x43);
        REQUIRE(t[1] == 0x3);
    }

    SECTION("getFrame for frame number") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaHdf5Movie>(testFile);
        REQUIRE(m->isValid());
        auto nvf = m->getFrame(0);
        unsigned char * t = reinterpret_cast<unsigned char *>(nvf->getPixels());
        REQUIRE(t[0] == 0x43);
        REQUIRE(t[1] == 0x3);
    }

    SECTION("getTimingInfo().getDuration()") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaHdf5Movie>(testFile);
        REQUIRE(m->isValid());
        REQUIRE(m->getTimingInfo().getDuration().toDouble() == 3.168);
    }

    SECTION("toString") {
        isx::SpMovie_t m = std::make_shared<isx::NVistaHdf5Movie>(testFile);
        REQUIRE(m->toString() == "/images");
    }

    isx::CoreShutdown();
}

TEST_CASE("MovieTestAsync", "[core]") {
    std::string testFileName = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    std::shared_ptr<H5::H5File> h5File = std::make_shared<H5::H5File>(testFileName, H5F_ACC_RDONLY);
    isx::SpHdf5FileHandle_t testFile = std::make_shared<isx::Hdf5FileHandle>(h5File, H5F_ACC_RDONLY);


    isx::CoreInitialize();
    
    bool isDataCorrect = true;
    std::atomic_int doneCount(0);
    
    uint8_t f0_data[] = { 0x43, 0x03, 0x18, 0x03, 0x55, 0x03, 0x60, 0x03 };
    uint8_t f1_data[] = { 0x2D, 0x03, 0x0C, 0x03, 0x15, 0x03, 0x36, 0x03 };
    uint8_t f2_data[] = { 0xE4, 0x02, 0x0D, 0x03, 0x1D, 0x03, 0xEB, 0x02 };
    uint8_t f3_data[] = { 0xB8 ,0x02, 0xB8, 0x02, 0xAE, 0x02, 0xD0, 0x02 };
    uint8_t f4_data[] = { 0x92, 0x02, 0xA3, 0x02, 0xAD, 0x02, 0x91, 0x02 };
    
    std::vector<uint8_t *> expected = { f0_data, f1_data, f2_data, f3_data, f4_data };
    
    size_t numTestFrames = expected.size();
    size_t numTestBytesPerFrame = sizeof(f0_data);
    
    isx::SpMovie_t m = std::make_shared<isx::NVistaHdf5Movie>(testFile);
    REQUIRE(m->isValid());
    isx::MovieGetFrameCB_t cb = [&](isx::SpU16VideoFrame_t inFrame){
        size_t index = inFrame->getFrameIndex();
        unsigned char * t = reinterpret_cast<unsigned char *>(inFrame->getPixels());
        if (memcmp(t, expected[index], numTestBytesPerFrame))
        {
            isDataCorrect = false;
        }
        ++doneCount;
    };

    SECTION("get frame for time asynchronously") {
        isx::Time fetchTime = m->getTimingInfo().getStart();
        for (size_t i = 0; i < numTestFrames; ++i)
        {
            m->getFrameAsync(fetchTime, cb);
            fetchTime += m->getTimingInfo().getStep();
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
        REQUIRE(doneCount == int(numTestFrames));
        REQUIRE(isDataCorrect);
    }

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
