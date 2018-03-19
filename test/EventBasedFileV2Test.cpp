#include "isxTest.h"
#include "isxEventBasedFileV2.h"
#include "isxPathUtils.h"
#include "isxLogicalTrace.h"
#include "catch.hpp"

namespace 
{
    void compareMetrics(isx::SpTraceMetrics_t a, isx::SpTraceMetrics_t b)
    {
        REQUIRE(a->m_snr == b->m_snr);
        REQUIRE(a->m_mad == b->m_mad);
        REQUIRE(a->m_eventRate == b->m_eventRate);
        REQUIRE(a->m_eventAmpMedian == b->m_eventAmpMedian);
        REQUIRE(a->m_eventAmpSD == b->m_eventAmpSD);
        REQUIRE(a->m_riseMedian == b->m_riseMedian);
        REQUIRE(a->m_riseSD == b->m_riseSD);
        REQUIRE(a->m_decayMedian == b->m_decayMedian);
        REQUIRE(a->m_decaySD == b->m_decaySD);
    }
}

TEST_CASE("EventBasedFileV2Test", "[core]")
{
    isx::CoreInitialize();

    std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/test_gpio.isxd");
    std::remove(fileName.c_str());

    SECTION("Invalid file object") 
    {
        isx::EventBasedFileV2 file; 
        REQUIRE(!file.isValid());
    }

    SECTION("Logical data file with one channel")
    {        
        isx::isize_t numSamples = 20;
        isx::TimingInfo ti(isx::Time(), isx::DurationInSeconds(10, 1), numSamples);
        std::vector<isx::isize_t> timeIndices{3, 6, 10, 11, 18};
        std::vector<float> data{1.f, 0.f, 3.f, 0.f, 5.f};
        std::string channelName = "test";

        // Write a file
        {
            isx::EventBasedFileV2 file(fileName, isx::DataSet::Type::GPIO, true);            
            
            isx::isize_t i = 0;
            for (auto & s : data)
            {
                isx::Time t = ti.convertIndexToStartTime(timeIndices[i++]);
                t -= ti.getStart().getSecsSinceEpoch();
                uint64_t t_us = uint64_t(t.getSecsSinceEpoch().toDouble() * 1E6);
                isx::EventBasedFileV2::DataPkt pkt(t_us, s, 0);
                file.writeDataPkt(pkt);
            }
            file.setChannelList({channelName});
            file.setTimingInfo(ti.getStart(), ti.getEnd(), {isx::DurationInSeconds(0, 1)});
            file.closeFileForWriting();
        }

        isx::EventBasedFileV2 file(fileName); 
        REQUIRE(file.isValid());
        REQUIRE(file.getFileName() == fileName);
        const std::vector<std::string> channels = file.getChannelList();
        REQUIRE(channels.size() == 1);
        REQUIRE(channels.at(0) == channelName);
        
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


    SECTION("Logical data file with interleaved channels")
    {
        isx::isize_t numSamples = 20;
        isx::TimingInfo ti(isx::Time(), isx::DurationInSeconds(10, 1), numSamples);
        std::vector<isx::isize_t> timeIndices{3, 4, 6, 7, 10, 11, 13, 15, 18, 19};
        std::vector<float> data{1.f, 0.f, 3.f, 0.f, 5.f, 0.f, 7.f, 0.f, 9.f, 0.f};
        std::vector<std::string> channelNames = {"test0", "test1"};
        isx::EventMetrics_t metrics = {
            std::make_shared<isx::TraceMetrics>(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f),
            std::make_shared<isx::TraceMetrics>(9.f, 8.f, 7.f, 6.f, 5.f, 4.f, 3.f, 2.f, 1.f)};

        // Write a file
        {
            isx::EventBasedFileV2 file(fileName, isx::DataSet::Type::GPIO, true);
            
            
            isx::isize_t i = 0;
            for (auto & s : data)
            {
                isx::Time t = ti.convertIndexToStartTime(timeIndices[i]);
                t -= ti.getStart().getSecsSinceEpoch();
                uint64_t t_us = uint64_t(t.getSecsSinceEpoch().toDouble() * 1E6);
                
                int c = i % 2;
                isx::EventBasedFileV2::DataPkt pkt(t_us, s, uint64_t(c));
                file.writeDataPkt(pkt);
                i++;
            }
            file.setTraceMetrics(0, metrics[0]);
            file.setTraceMetrics(1, metrics[1]);
            file.setChannelList(channelNames);
            file.setTimingInfo(ti.getStart(), ti.getEnd(), std::vector<isx::DurationInSeconds>(2, isx::DurationInSeconds(0, 1)));     
            file.closeFileForWriting();
        }

        // Read file and verify the data is right
        isx::EventBasedFileV2 file(fileName); 
        REQUIRE(file.isValid());
        REQUIRE(file.getFileName() == fileName);
        const std::vector<std::string> channels = file.getChannelList();
        REQUIRE(channels.size() == 2);
        auto it = std::find(channels.begin(), channels.end(), channelNames[0]);
        REQUIRE(it != channels.end());
        it = std::find(channels.begin(), channels.end(), channelNames[1]);
        REQUIRE(it != channels.end());

        SECTION("Verify data correctness")
        {
            for (isx::isize_t i(0); i < channelNames.size(); ++i)
            {
                isx::SpLogicalTrace_t logicalTrace = file.getLogicalData(channelNames[i]);
                REQUIRE(logicalTrace != nullptr);

                const std::map<isx::Time, float> vals = logicalTrace->getValues();
                REQUIRE(vals.size() == data.size()/2);

                isx::isize_t j(i);

                for (auto & pair : vals)
                {
                    REQUIRE(pair.second == data.at(j));
                    j += 2;
                }
            }
        }

        SECTION("Verify metrics")
        {
            auto m0 = file.getTraceMetrics(0);
            auto m1 = file.getTraceMetrics(1);
            compareMetrics(metrics[0], m0);
            compareMetrics(metrics[1], m1);            
        }        
    }    

    std::remove(fileName.c_str());
    isx::CoreShutdown();
}