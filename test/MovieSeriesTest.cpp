#include "isxCore.h"
#include "isxMosaicMovie.h"
#include "isxMovieSeries.h"
#include "catch.hpp"
#include "isxTest.h"

#include <stdio.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <cstdio>

namespace
{

isx::SpWritableMovie_t
writeTestU16Movie(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        uint16_t inValue)
{
    isx::isize_t numFrames = inTimingInfo.getNumTimes();
    isx::isize_t numPixels = inSpacingInfo.getTotalNumPixels();
    isx::isize_t rowSizeInBytes = sizeof(uint16_t) * inSpacingInfo.getNumColumns();

    isx::SpWritableMovie_t movie = std::make_shared<isx::MosaicMovie>(
            inFileName, inTimingInfo, inSpacingInfo, isx::DataType::U16);
    for (isx::isize_t f = 0; f < numFrames; ++f)
    {
        isx::SpVideoFrame_t frame = std::make_shared<isx::VideoFrame>(
            inSpacingInfo,
            rowSizeInBytes,
            1, // numChannels
            isx::DataType::U16,
            inTimingInfo.convertIndexToStartTime(f),
            f);

        uint16_t * pPixels = frame->getPixelsAsU16();
        for (isx::isize_t p = 0; p < numPixels; ++p)
        {
            *pPixels = inValue + uint16_t(f);
            pPixels++;
        }
        movie->writeFrame(frame);
    }
    movie->closeForWriting();
    return movie;
}
} // namespace

TEST_CASE("MovieSeriesU16", "[core-internal]")
{
    std::array<const char *, 3> names = 
    { {
        "seriesMovie0.isxd",
        "seriesMovie1.isxd",
        "seriesMovie2.isxd"
    } };
    std::vector<std::string> filenames;

    for (auto n: names)
    {
        filenames.push_back(g_resources["unitTestDataPath"] + "/" + n);
    }
    
    for (const auto & fn: filenames)
    {
        std::remove(fn.c_str());
    }

    // NOTE sweet : use 1000 for the denom, so that flooring has flooring to that
    // is so lossy. See MOS-805 for some context.
    std::array<isx::TimingInfo, 3> timingInfos =
    { {
        { isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1000)), isx::Ratio(1, 20), 3 },
        { isx::Time(2222, 4, 1, 3, 1, 0, isx::DurationInSeconds(0, 1000)), isx::Ratio(1, 20), 4 },
        { isx::Time(2222, 4, 1, 3, 2, 0, isx::DurationInSeconds(0, 1000)), isx::Ratio(1, 20), 5 }
    } };

    isx::SizeInPixels_t sizePixels(4, 3);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(sizePixels, pixelSize, topLeft);

    isx::DataType dataType = isx::DataType::U16;

    isx::CoreInitialize();

    SECTION("Empty constructor")
    {
        auto movie = std::make_shared<isx::MovieSeries>();
        REQUIRE(!movie->isValid());
    }
    
    SECTION("Compatible set of movies")
    {
        isx::isize_t i = 0;
        for (const auto fn: filenames)
        {
            writeTestU16Movie(fn, timingInfos[i], spacingInfo, uint16_t((i + 1) * 10));
            ++i;
        }

        const isx::TimingInfo expectedTimingInfo(isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 12);

        isx::TimingInfos_t expectedTimingInfos;
        for (const auto fn: filenames)
        {
            auto m = std::make_shared<isx::MosaicMovie>(fn);
            expectedTimingInfos.push_back(m->getTimingInfo());
        }
        
        auto movie = std::make_shared<isx::MovieSeries>(filenames);
        
        REQUIRE(movie->isValid());
        REQUIRE(movie->getSpacingInfo() == spacingInfo);
        REQUIRE(movie->getTimingInfo() == expectedTimingInfo);
        REQUIRE(movie->getTimingInfosForSeries() == expectedTimingInfos);
        REQUIRE(movie->getDataType() == isx::DataType::U16);
    }
    
    SECTION("Compatible set of movies one with un-aligned start time")
    {
        std::vector<isx::TimingInfo> tis(timingInfos.begin(), timingInfos.end());
        tis[2] = isx::TimingInfo(isx::Time(2222, 4, 1, 3, 2, 0, isx::DurationInSeconds(1, 40)), isx::Ratio(1, 20), 5);
        isx::isize_t i = 0;
        for (const auto fn: filenames)
        {
            writeTestU16Movie(fn, tis[i], spacingInfo, uint16_t((i + 1) * 10));
            ++i;
        }

        const isx::TimingInfo expectedTimingInfo(isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 12);

        isx::TimingInfos_t expectedTimingInfos;
        for (const auto fn: filenames)
        {
            auto m = std::make_shared<isx::MosaicMovie>(fn);
            expectedTimingInfos.push_back(m->getTimingInfo());
        }
        expectedTimingInfos[2] = isx::TimingInfo(isx::Time(2222, 4, 1, 3, 2, 0, isx::DurationInSeconds(1, 40)), isx::Ratio(1, 20), 5);
        
        auto movie = std::make_shared<isx::MovieSeries>(filenames);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getSpacingInfo() == spacingInfo);
        REQUIRE(movie->getTimingInfo() == expectedTimingInfo);
        REQUIRE(movie->getTimingInfosForSeries() == expectedTimingInfos);
        REQUIRE(movie->getDataType() == isx::DataType::U16);
    }

    SECTION("Compatible set of movies not sequential in time")
    {
        std::vector<isx::TimingInfo> tis(timingInfos.begin(), timingInfos.end());
        std::swap(tis[1], tis[2]);
        isx::isize_t i = 0;
        for (const auto fn: filenames)
        {
            writeTestU16Movie(fn, tis[i], spacingInfo, uint16_t((i + 1) * 10));
            ++i;
        }
        const isx::TimingInfo expectedTimingInfo(isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 12);

        isx::TimingInfos_t expectedTimingInfos;
        for (const auto fn: filenames)
        {
            auto m = std::make_shared<isx::MosaicMovie>(fn);
            expectedTimingInfos.push_back(m->getTimingInfo());
        }
        std::swap(expectedTimingInfos[1], expectedTimingInfos[2]);
        
        auto movie = std::make_shared<isx::MovieSeries>(filenames);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getSpacingInfo() == spacingInfo);
        REQUIRE(movie->getTimingInfo() == expectedTimingInfo);
        REQUIRE(movie->getTimingInfosForSeries() == expectedTimingInfos);
        REQUIRE(movie->getDataType() == isx::DataType::U16);
    }

    SECTION("getFrame")
    {
        isx::isize_t i = 0;
        for (const auto fn: filenames)
        {
            writeTestU16Movie(fn, timingInfos[i], spacingInfo, uint16_t((i + 1) * 10));
            ++i;
        }
        auto movieSeries = std::make_shared<isx::MovieSeries>(filenames);
        const auto tis = movieSeries->getTimingInfosForSeries();

        size_t mIndex = 0;
        for (const auto fn: filenames)
        {
            auto m = std::make_shared<isx::MosaicMovie>(fn);
            const auto numFrames = m->getTimingInfo().getNumTimes();
            for (isx::isize_t i = 0; i < numFrames; ++i)
            {
                const auto frameTime = m->getTimingInfo().convertIndexToStartTime(i);
                const auto origFrame = m->getFrame(i);

                const auto globalIndex = isx::getGlobalIndex(tis, std::pair<isx::isize_t, isx::isize_t>(mIndex, i));
                const auto testFrame = movieSeries->getFrame(globalIndex);

                REQUIRE(origFrame->getDataType() == testFrame->getDataType());
                REQUIRE(origFrame->getWidth() == testFrame->getWidth());
                REQUIRE(origFrame->getHeight() == testFrame->getHeight());
                REQUIRE(origFrame->getNumChannels() == testFrame->getNumChannels());
                REQUIRE(origFrame->getImageSizeInBytes() == testFrame->getImageSizeInBytes());
                REQUIRE(0 == std::memcmp(origFrame->getPixels(), testFrame->getPixels(), origFrame->getImageSizeInBytes()));
            }
            ++mIndex;
        }
    }

    for (const auto & fn: filenames)
    {
        std::remove(fn.c_str());
    }

    isx::CoreShutdown();
}
