#include "isxTest.h"
#include "isxIMUFile.h"
#include "isxEventBasedFileV2.h"
#include "isxPathUtils.h"
#include "catch.hpp"

TEST_CASE("IMUFileTest", "[core]")
{
    isx::CoreInitialize();

    std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/imu/2020-02-13-18-43-21_video.imu");
    std::string outputDir = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/imu");
    std::string outputFileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/imu/2020-02-13-18-43-21_video_imu.isxd");
    std::remove(outputFileName.c_str());

    SECTION("Use accelerometer sample rate if no magnetometer data")
    {
        isx::IMUFile imuFile(fileName, outputDir);
        REQUIRE(imuFile.getFileName() == fileName);

        imuFile.parse();
        REQUIRE(imuFile.getOutputFileName() == outputFileName);

        isx::EventBasedFileV2 ebFile(outputFileName);
        REQUIRE(ebFile.isValid());

        const std::vector<std::string> channels = ebFile.getChannelList();
        REQUIRE(channels.size() == 9);

        isx::TimingInfo timingInfo = ebFile.getTimingInfo();
        REQUIRE(timingInfo.getStart() == isx::Time(isx::DurationInSeconds(1581619401465, 1000)));
        REQUIRE(timingInfo.getStep() == isx::Ratio(20570732, 1000000000));
        REQUIRE(timingInfo.getNumTimes() == 205);
    }

    std::remove(outputFileName.c_str());
    isx::CoreShutdown();
}
