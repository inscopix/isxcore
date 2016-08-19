#include "isxMosaicMovieFile.h"
#include "catch.hpp"
#include "isxTest.h"

#include <stdio.h>
#include <algorithm>

TEST_CASE("MosaicMovieFileU16", "[core-internal]")
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

    SECTION("Empty constructor")
    {
        isx::MosaicMovieFile movie;
        REQUIRE(!movie.isValid());
    }

    SECTION("Write constructor.")
    {
        isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo);
        REQUIRE(movie.isValid());
    }

    SECTION("Read after writing.")
    {
        isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo);
        REQUIRE(movie.isValid());
        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        isx::SpU16VideoFrame_t frame;
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            frame = movie.readFrame(f);
            std::vector<uint16_t> frameBuf(frame->getPixels(), frame->getPixels() + numPixels);
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 0);
            }
        }
    }

    SECTION("Read constructor after writing.")
    {
        {
            isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo);
        }
        isx::MosaicMovieFile movie(fileName);
        REQUIRE(movie.isValid());
        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        isx::SpU16VideoFrame_t frame;
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            frame = movie.readFrame(f);
            std::vector<uint16_t> frameBuf(frame->getPixels(), frame->getPixels() + numPixels);
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 0);
            }
        }
    }

    SECTION("Write frame data and verify read matches it.")
    {
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        isx::isize_t rowSizeInBytes = sizeof(uint16_t) * spacingInfo.getNumColumns();

        isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo);
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::Time frameTime = timingInfo.convertIndexToTime(f);
            isx::SpU16VideoFrame_t frame = std::make_shared<isx::U16VideoFrame_t>(
                spacingInfo,
                rowSizeInBytes,
                1, // numChannels
                frameTime, f);
            
            uint16_t * pPixels = frame->getPixels();
            
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                *pPixels = uint16_t((f * numPixels) + p);
                pPixels++;
            }

            movie.writeFrame(frame);
        }

        REQUIRE(movie.isValid());
        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
        isx::SpU16VideoFrame_t frame;
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            frame = movie.readFrame(f);
            std::vector<uint16_t> frameBuf(frame->getPixels(), frame->getPixels() + numPixels);
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == (f * numPixels) + p);
            }
        }
    }
}

TEST_CASE("MosaicMovieFileF32AsU16", "[core-internal]")
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

    SECTION("Write constructor.")
    {
        isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo, dataType);
        REQUIRE(movie.isValid());
        REQUIRE(movie.getFileName() == fileName);
        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
        REQUIRE(movie.getDataType() == dataType);
    }

    SECTION("Read frame after writing.")
    {
        isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo, dataType);
        REQUIRE(movie.isValid());
        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
        REQUIRE(movie.getDataType() == dataType);
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        isx::SpU16VideoFrame_t frame;
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            frame = movie.readFrame(f);
            std::vector<uint16_t> frameBuf(frame->getPixels(), frame->getPixels() + numPixels);
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 0);
            }
        }
    }

    SECTION("Read constructor after writing.")
    {
        {
            isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo, dataType);
        }
        isx::MosaicMovieFile movie(fileName);
        REQUIRE(movie.isValid());
        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
        REQUIRE(movie.getDataType() == dataType);
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        isx::SpU16VideoFrame_t frame;
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            frame = movie.readFrame(f);
            std::vector<uint16_t> frameBuf(frame->getPixels(), frame->getPixels() + numPixels);
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 0);
            }
        }
    }

    SECTION("Write frame data and verify read matches it.")
    {
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        isx::isize_t rowSizeInBytes = sizeof(uint16_t) * spacingInfo.getNumColumns();

        isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo, dataType);
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::Time frameTime = timingInfo.convertIndexToTime(f);
            isx::SpU16VideoFrame_t frame = std::make_shared<isx::U16VideoFrame_t>(
                spacingInfo,
                rowSizeInBytes,
                1, // numChannels
                frameTime, f);

            uint16_t * pPixels = frame->getPixels();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                *pPixels = uint16_t((f * numPixels) + p);
                pPixels++;
            }

            movie.writeFrame(frame);
        }

        REQUIRE(movie.isValid());
        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
        isx::SpU16VideoFrame_t frame;
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            frame = movie.readFrame(f);
            std::vector<uint16_t> frameBuf(frame->getPixels(), frame->getPixels() + numPixels);
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == (f * numPixels) + p);
            }
        }
    }
}
