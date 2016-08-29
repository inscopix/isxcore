#include "isxCore.h"
#include "isxMosaicMovie.h"
#include "catch.hpp"
#include "isxTest.h"

#include <stdio.h>
#include <algorithm>

namespace
{

isx::SpWritableMovie_t
writeTestU16Movie(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo)
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
            inTimingInfo.convertIndexToTime(f),
            f);

        uint16_t * pPixels = frame->getPixelsAsU16();
        for (isx::isize_t p = 0; p < numPixels; ++p)
        {
            *pPixels = uint16_t((f * numPixels) + p);
            pPixels++;
        }

        movie->writeFrame(frame);
    }
    return movie;
}

isx::SpWritableMovie_t
writeTestF32Movie(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo)
{
    isx::isize_t numFrames = inTimingInfo.getNumTimes();
    isx::isize_t numPixels = inSpacingInfo.getTotalNumPixels();
    isx::isize_t rowSizeInBytes = sizeof(float) * inSpacingInfo.getNumColumns();

    isx::SpWritableMovie_t movie = std::make_shared<isx::MosaicMovie>(
            inFileName, inTimingInfo, inSpacingInfo, isx::DataType::F32);
    for (isx::isize_t f = 0; f < numFrames; ++f)
    {
        isx::SpVideoFrame_t frame = std::make_shared<isx::VideoFrame>(
            inSpacingInfo,
            rowSizeInBytes,
            1, // numChannels
            isx::DataType::F32,
            inTimingInfo.convertIndexToTime(f),
            f);

        float * pPixels = frame->getPixelsAsF32();
        for (isx::isize_t p = 0; p < numPixels; ++p)
        {
            *pPixels = float((f * numPixels) + p);
            pPixels++;
        }

        movie->writeFrame(frame);
    }
    return movie;
}

} // namespace

TEST_CASE("MosaicMovieU16", "[core-internal]")
{
    std::string fileName = g_resources["testDataPath"] + "/movie.isxd";

    isx::Time start;
    isx::DurationInSeconds step(50, 1000);
    isx::isize_t numFrames = 5;
    isx::TimingInfo timingInfo(start, step, numFrames);

    isx::SizeInPixels_t numPixels(4, 3);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

    isx::DataType dataType = isx::DataType::U16;

    isx::CoreInitialize();

    SECTION("Empty constructor")
    {
        auto movie = std::make_shared<isx::MosaicMovie>();
        REQUIRE(!movie->isValid());
    }

    SECTION("Write constructor.")
    {
        auto movie = std::make_shared<isx::MosaicMovie>(
                fileName, timingInfo, spacingInfo, dataType);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getDataType() == dataType);
    }

    SECTION("Read after writing.")
    {
        auto movie = std::make_shared<isx::MosaicMovie>(
                fileName, timingInfo, spacingInfo, dataType);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);
            uint16_t * frameBuf = frame->getPixelsAsU16();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 0);
            }
        }
    }

    SECTION("Read constructor after writing.")
    {
        {
            auto movie = std::make_shared<isx::MosaicMovie>(
                    fileName, timingInfo, spacingInfo, dataType);
        }
        auto movie = std::make_shared<isx::MosaicMovie>(fileName);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);
            uint16_t * frameBuf = frame->getPixelsAsU16();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 0);
            }
        }
    }

    SECTION("Write frame data and verify read matches it.")
    {
        isx::SpWritableMovie_t movie = writeTestU16Movie(fileName, timingInfo, spacingInfo);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);
            uint16_t * frameBuf = frame->getPixelsAsU16();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == (f * numPixels) + p);
            }
        }
    }

    isx::CoreShutdown();
}

TEST_CASE("MosaicMovieF32", "[core-internal]")
{
    std::string fileName = g_resources["testDataPath"] + "/movie.isxd";

    isx::Time start;
    isx::DurationInSeconds step(50, 1000);
    isx::isize_t numFrames = 5;
    isx::TimingInfo timingInfo(start, step, numFrames);

    isx::SizeInPixels_t numPixels(4, 3);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

    isx::DataType dataType = isx::DataType::F32;

    isx::CoreInitialize();

    SECTION("Write constructor.")
    {
        auto movie = std::make_shared<isx::MosaicMovie>(
                fileName, timingInfo, spacingInfo, dataType);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getDataType() == isx::DataType::F32);
    }

    SECTION("Read after writing.")
    {
        auto movie = std::make_shared<isx::MosaicMovie>(
                fileName, timingInfo, spacingInfo, dataType);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);
            float * frameBuf = frame->getPixelsAsF32();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 0);
            }
        }
    }

    SECTION("Read constructor after writing.")
    {
        {
            auto movie = std::make_shared<isx::MosaicMovie>(
                    fileName, timingInfo, spacingInfo, dataType);
        }
        auto movie = std::make_shared<isx::MosaicMovie>(fileName);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);
            float * frameBuf = frame->getPixelsAsF32();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 0);
            }
        }
    }

    SECTION("Write frame data and verify read matches it.")
    {
        isx::SpWritableMovie_t movie = writeTestF32Movie(fileName, timingInfo, spacingInfo);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);
            float * frameBuf = frame->getPixelsAsF32();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == (f * numPixels) + p);
            }
        }
    }

    isx::CoreShutdown();
}
