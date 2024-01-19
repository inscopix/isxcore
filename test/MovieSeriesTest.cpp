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
        for (const auto & fn: filenames)
        {
            writeTestU16Movie(fn, timingInfos[i], spacingInfo, uint16_t((i + 1) * 10));
            ++i;
        }

        const isx::TimingInfo expectedTimingInfo(isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 12);

        isx::TimingInfos_t expectedTimingInfos;
        for (const auto & fn: filenames)
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
        for (const auto & fn: filenames)
        {
            writeTestU16Movie(fn, tis[i], spacingInfo, uint16_t((i + 1) * 10));
            ++i;
        }

        const isx::TimingInfo expectedTimingInfo(isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 12);

        isx::TimingInfos_t expectedTimingInfos;
        for (const auto & fn: filenames)
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
        for (const auto & fn: filenames)
        {
            writeTestU16Movie(fn, tis[i], spacingInfo, uint16_t((i + 1) * 10));
            ++i;
        }
        const isx::TimingInfo expectedTimingInfo(isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 12);

        isx::TimingInfos_t expectedTimingInfos;
        for (const auto & fn: filenames)
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
        for (const auto & fn: filenames)
        {
            writeTestU16Movie(fn, timingInfos[i], spacingInfo, uint16_t((i + 1) * 10));
            ++i;
        }
        auto movieSeries = std::make_shared<isx::MovieSeries>(filenames);
        const auto tis = movieSeries->getTimingInfosForSeries();

        size_t mIndex = 0;
        for (const auto & fn: filenames)
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

TEST_CASE("NVisionMovieSeries", "[core-internal]")
{
    const std::vector<std::string> testFileNames = {
        g_resources["unitTestDataPath"] + "/nVision/20220412-200447-camera-100.isxb",
        g_resources["unitTestDataPath"] + "/nVision/2022-04-18-21-48-13-camera-1_dropped.isxb"
    };

    isx::CoreInitialize();

    const auto movie = std::make_shared<isx::MovieSeries>(testFileNames);

    SECTION("Is valid")
    {
        REQUIRE(movie->isValid());
    }

    SECTION("Filename")
    {
        const std::string expFileName = "MovieSeries(test_data/unit_test/nVision/20220412-200447-camera-100.isxb, test_data/unit_test/nVision/2022-04-18-21-48-13-camera-1_dropped.isxb)";
        REQUIRE(movie->getFileName() == expFileName);
    }

    SECTION("Timing info")
    {
        const isx::Time start(2022, 4, 13, 3, 8, 10, isx::DurationInSeconds(471, 1000));
        const isx::DurationInSeconds step(3732032, 112000000);
        const size_t numSamples = 1245;
        const isx::TimingInfo expTimingInfo(start, step, numSamples);

        REQUIRE(movie->getTimingInfo() == expTimingInfo);
    }

    SECTION("Timing infos")
    {
        {
            const isx::Time start(2022, 4, 13, 3, 8, 10, isx::DurationInSeconds(471, 1000));
            const isx::DurationInSeconds step(3732032, 112000000);
            const size_t numSamples = 113;
            const isx::TimingInfo expTimingInfo(start, step, numSamples);
            REQUIRE(movie->getTimingInfosForSeries()[0] == expTimingInfo);
        }

        {
            const isx::Time start(2022, 4, 19, 4, 48, 13, isx::DurationInSeconds(459, 1000));
            const isx::DurationInSeconds step(37712004, 1131000000);
            const size_t numSamples = 1132;
            const std::vector<size_t> droppedFrames = {
                186, 188, 189, 190, 192, 193, 194, 196, 197, 198, 200, 201, 202, 204, 205,
                206, 208, 209, 210, 212, 213, 214, 216, 217, 218, 220, 221, 223, 224, 226,
                227, 228, 230, 231, 232, 234, 235, 237, 238, 239, 241, 242, 244, 245, 247,
                248, 249, 251, 252, 254, 255, 256, 258, 259, 260, 262, 263, 265, 266, 267,
                269, 270, 271, 273, 274, 276, 277, 278, 280, 281, 283, 284, 286, 287, 288,
                290, 291, 293, 294, 295, 297, 298, 300, 301, 303, 304, 305, 307, 308, 309,
                311, 313, 314, 315, 317, 318, 319, 321, 322, 324, 325, 326, 328, 329, 331,
                332, 334, 335, 336, 338, 339, 340, 342, 343, 345, 346, 347, 349, 350, 352,
                353, 354, 355, 357, 358, 359, 361, 362, 364, 365, 366, 368, 369, 370, 372,
                373, 375, 376, 378, 379, 380, 382, 383, 385, 386, 387, 389, 390, 392, 393,
                394, 396, 397, 398, 400, 401, 403, 404, 405, 407, 408, 410, 411, 412, 414,
                415, 417, 418, 419, 421, 422, 423, 425, 426, 428, 429, 430, 432, 433, 435,
                436, 438, 439, 440, 442, 443, 444, 446, 447, 449, 450, 451, 453, 454, 455,
                457, 458, 459, 461, 462, 464, 465, 466, 468, 470, 471, 472, 474, 475, 477,
                478, 479, 481, 482, 484, 485, 486, 488, 489, 491, 492, 494, 495, 496, 498,
                499, 500, 502, 503, 505, 506, 507, 509, 510, 512, 513, 514, 516, 517, 519,
                520, 521, 523, 524, 526, 527, 529, 530, 531, 533, 534, 535, 537, 538
            };
            const isx::TimingInfo expTimingInfo(start, step, numSamples, droppedFrames);
            REQUIRE(movie->getTimingInfosForSeries()[1] == expTimingInfo);
        }        
    }

    SECTION("Spacing info")
    {
        const isx::SizeInPixels_t numPixels(1920, 1080);
        const isx::SpacingInfo expSpacingInfo(numPixels);

        REQUIRE(movie->getSpacingInfo() == expSpacingInfo);
    }

    SECTION("Frames")
    {
        const auto movies = movie->getMovies();
        size_t seriesIndex = 0;
        for (const auto & m : movies)
        {
            for (size_t i = 0; i < m->getTimingInfo().getNumTimes(); i++)
            {
                const auto movieFrame = m->getFrame(i);
                const auto seriesFrame = movie->getFrame(seriesIndex);
                requireEqualImages(movieFrame->getImage(), seriesFrame->getImage());
                seriesIndex++;
            }
        }
    }

    SECTION("Frame metadata")
    {
        const auto movies = movie->getMovies();
        size_t seriesIndex = 0;
        for (const auto & m : movies)
        {
            for (size_t i = 0; i < m->getTimingInfo().getNumTimes(); i++)
            {
                const auto movieFrameMetadata = m->getFrameMetadata(i);
                const auto seriesFrameMetadata = movie->getFrameMetadata(seriesIndex);
                REQUIRE(movieFrameMetadata == seriesFrameMetadata);
                seriesIndex++;
            }
        }
    }

    isx::CoreShutdown();
}
