#include "isxWritableEvents.h"
#include "isxTest.h"
#include "isxPathUtils.h"
#include "isxException.h"

#include "catch.hpp"

#include <fstream>
#include <algorithm>


void writeEventsTestFile(const std::string & inFileName)
{
    isx::SpWritableEvents_t f = isx::writeEvents(inFileName);

    isx::TimingInfo ti(isx::Time(), isx::DurationInSeconds(5, 1), 100);

    std::vector<std::string> cellNames{"C0", "C1"};
    std::vector<std::vector<float>> values{{2.f, 1.5f}, {1.f, 4.f, 3.f}};
    std::vector<std::vector<isx::isize_t>> timeIndices{{15, 25}, {35, 50, 75}};

    f->setTimingInfo(ti);

    isx::isize_t cell = 0;
    for (auto & name : cellNames)
    {
        auto & v = values.at(cell);
        auto & t = timeIndices.at(cell);

        f->writeCellHeader(name, v.size());

        isx::isize_t j = 0;
        for (auto & e : v)
        {
            isx::Time time = ti.convertIndexToStartTime(t.at(j));
            uint64_t time_us = uint64_t(time.getSecsSinceEpoch().toDouble() * 1E6);

            f->writeDataPkt(time_us, e);

            j++;
        }
        cell++;
    }

    f->closeForWriting();

}

TEST_CASE("EventsDataTest", "[core]")
{
    isx::CoreInitialize();
    
    std::string outputDir = isx::getAbsolutePath(g_resources["unitTestDataPath"]);
    std::string fileName = outputDir + "/eventsTest.isxd";
    std::remove(fileName.c_str());


    SECTION("Write events and test reading them") 
    {
        writeEventsTestFile(fileName);

        isx::SpEvents_t f = isx::readEvents(fileName);

        /// Expected values
        std::vector<std::string> cellNames{"C0", "C1"};
        std::vector<std::vector<float>> values{{2.f, 1.5f}, {1.f, 4.f, 3.f}};
        std::vector<std::vector<isx::isize_t>> timeIndices{{15, 25}, {35, 50, 75}};
        isx::TimingInfo ti(isx::Time(), isx::DurationInSeconds(5, 1), 100);

        REQUIRE(f);
        REQUIRE(f->getFileName() == fileName);
        REQUIRE(f->numberOfCells() == cellNames.size());
        REQUIRE(f->getCellNamesList() == cellNames);
        REQUIRE(f->getTimingInfo() == ti);

        isx::isize_t i = 0;
        for (auto & n : cellNames)
        {
            isx::SpLogicalTrace_t data = f->getLogicalData(n);
            REQUIRE(data);
            auto & expected = values.at(i);
            auto & result = data->getValues();
            REQUIRE(result.size() == expected.size());

            isx::isize_t j(0);

            for (auto & pair : result)
            {
                REQUIRE(pair.second == expected.at(j++));
            }
            ++i;
        }    
        
    }


    isx::CoreShutdown();
    std::remove(fileName.c_str());
}
