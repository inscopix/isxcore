#include "isxTest.h"
#include "isxTimeStampedDataFile.h"
#include "isxPathUtils.h"
#include "catch.hpp"

TEST_CASE("TimeStampDataFileTest", "[core]")
{
    isx::CoreInitialize();

    std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/test_gpio.isxd");
    std::remove(fileName.c_str());

    SECTION("Invalid file object") 
    {
        isx::TimeStampedDataFile file; 
        REQUIRE(!file.isValid());
    }


    SECTION("Analog data file")
    {          
        isx::isize_t numSamples = 5;
        isx::TimingInfo ti(isx::Time(), isx::DurationInSeconds(1, 1), numSamples);
        std::vector<float> data{1.f, 2.f, 3.f, 4.f, 5.f};
        std::string channelName = "GPIO4_AI";
        std::string mode = "mode";
        std::string trigFollow = "trigFollow";

        // Write a file
        {
            isx::TimeStampedDataFile file(fileName, isx::TimeStampedDataFile::StoredData::GPIO, true);
            file.setTimingInfo(ti);
            file.writeChannelHeader(channelName, mode, trigFollow, data.size());

            isx::isize_t i = 0;
            for (auto & s : data)
            {
                isx::Time t = ti.convertIndexToStartTime(i++);
                uint64_t t_us = uint64_t(t.getSecsSinceEpoch().toDouble() * 1E6);
                isx::TimeStampedDataFile::DataPkt pkt(t_us, true, s);
                file.writeDataPkt(pkt);
            }

            file.closeFileForWriting();
        }

        // Test reading it
        isx::TimeStampedDataFile file(fileName); 
        REQUIRE(file.isValid());
        REQUIRE(file.isAnalog());
        REQUIRE(file.getFileName() == fileName);
        REQUIRE(file.numberOfChannels() == 1);
        const std::vector<std::string> channels = file.getChannelList();
        REQUIRE(channels.size() == 1);
        REQUIRE(channels.at(0) == channelName);

        isx::SpFTrace_t trace = file.getAnalogData(channelName);
        REQUIRE(trace);
        isx::TimingInfo traceTi = trace->getTimingInfo();
        REQUIRE(traceTi == ti);
        
        for (isx::isize_t i(0); i < data.size(); ++i)
        {
            REQUIRE(trace->getValue(i) == data.at(i));
        }
        
    }

    SECTION("Logical data file")
    {        
        isx::isize_t numSamples = 20;
        isx::TimingInfo ti(isx::Time(), isx::DurationInSeconds(10, 1), numSamples);
        std::vector<isx::isize_t> timeIndices{3, 6, 10, 11, 18};
        std::vector<float> data{1.f, 0.f, 3.f, 0.f, 5.f};
        std::vector<bool> state{true, false, true, false, true};
        std::string channelName = "test";
        std::string mode = "mode";
        std::string trigFollow = "trigFollow";

        // Write a file
        {
            isx::TimeStampedDataFile file(fileName, isx::TimeStampedDataFile::StoredData::GPIO);
            file.setTimingInfo(ti);
            file.writeChannelHeader(channelName, mode, trigFollow, data.size());

            isx::isize_t i = 0;
            for (auto & s : data)
            {
                isx::Time t = ti.convertIndexToStartTime(timeIndices[i]);
                uint64_t t_us = uint64_t(t.getSecsSinceEpoch().toDouble() * 1E6);
                isx::TimeStampedDataFile::DataPkt pkt(t_us, state[i++], s);
                file.writeDataPkt(pkt);
            }

            file.closeFileForWriting();
        }

        isx::TimeStampedDataFile file(fileName); 
        REQUIRE(file.isValid());
        REQUIRE(!file.isAnalog());
        REQUIRE(file.getFileName() == fileName);
        REQUIRE(file.numberOfChannels() == 1);
        const std::vector<std::string> channels = file.getChannelList();
        REQUIRE(channels.size() == 1);
        REQUIRE(channels.at(0) == channelName);

        isx::SpFTrace_t trace = file.getAnalogData("unexistent");
        REQUIRE(trace == nullptr);
        
        isx::SpLogicalTrace_t logicalTrace = file.getLogicalData(channelName);
        REQUIRE(logicalTrace != nullptr);

        const std::map<isx::Time, float> vals = logicalTrace->getValues();
        REQUIRE(vals.size() == data.size());

        isx::isize_t i(0);

        for (auto & pair : vals)
        {
            REQUIRE(pair.second == data.at(i++));
        }
    }

    std::remove(fileName.c_str());
    isx::CoreShutdown();
}