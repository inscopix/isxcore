#include "isxMosaicMovieFile.h"
#include "catch.hpp"
#include "isxTest.h"

#include <stdio.h>
#include <algorithm>
#include <cstring>

namespace
{

void
writeTestU16Movie(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo)
{
    isx::isize_t numFrames = inTimingInfo.getNumTimes();
    isx::isize_t numPixels = inSpacingInfo.getTotalNumPixels();
    isx::isize_t rowSizeInBytes = sizeof(uint16_t) * inSpacingInfo.getNumColumns();

    isx::MosaicMovieFile movie(inFileName, inTimingInfo, inSpacingInfo, isx::DataType::U16);
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
            *pPixels = uint16_t((f * numPixels) + p);
            pPixels++;
        }

        movie.writeFrame(frame);
    }
}

void
writeTestF32Movie(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo)
{
    isx::isize_t numFrames = inTimingInfo.getNumTimes();
    isx::isize_t numPixels = inSpacingInfo.getTotalNumPixels();
    isx::isize_t rowSizeInBytes = sizeof(float) * inSpacingInfo.getNumColumns();

    isx::MosaicMovieFile movie(inFileName, inTimingInfo, inSpacingInfo, isx::DataType::F32);
    for (isx::isize_t f = 0; f < numFrames; ++f)
    {
        isx::SpVideoFrame_t frame = std::make_shared<isx::VideoFrame>(
            inSpacingInfo,
            rowSizeInBytes,
            1, // numChannels
            isx::DataType::F32,
            inTimingInfo.convertIndexToStartTime(f),
            f);

        float * pPixels = frame->getPixelsAsF32();
        for (isx::isize_t p = 0; p < numPixels; ++p)
        {
            *pPixels = float((f * numPixels) + p);
            pPixels++;
        }

        movie.writeFrame(frame);
    }
}

} // namespace

TEST_CASE("MosaicMovieFileU16", "[core-internal]")
{
    std::string fileName = g_resources["unitTestDataPath"] + "/movie.isxd";

    isx::Time start;
    isx::DurationInSeconds step(50, 1000);
    isx::isize_t numFrames = 5;
    isx::TimingInfo timingInfo(start, step, numFrames);

    isx::SizeInPixels_t sizePixels(4, 3);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(sizePixels, pixelSize, topLeft);

    const isx::DataType dataType = isx::DataType::U16;

    SECTION("Empty constructor")
    {
        isx::MosaicMovieFile movie;
        REQUIRE(!movie.isValid());
    }

    SECTION("Write constructor.")
    {
        isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo, dataType);
        REQUIRE(movie.isValid());
        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
        REQUIRE(movie.getDataType() == dataType);
    }

    SECTION("Read after writing.")
    {
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        {
            isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo, dataType);            
            
            for (isx::isize_t f = 0; f < numFrames; ++f)
            {
                auto frame = std::make_shared<isx::VideoFrame>(
                    spacingInfo,
                    sizeof(uint16_t) * sizePixels.getWidth(),
                    1,
                    dataType,
                    timingInfo.convertIndexToStartTime(f),
                    f);
                std::fill(frame->getPixelsAsU16(), frame->getPixelsAsU16() + numPixels, 0xCAFE);
                movie.writeFrame(frame);
            }
        }
        
        isx::MosaicMovieFile movie(fileName);
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie.readFrame(f);
            uint16_t * frameBuf = frame->getPixelsAsU16();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 0xCAFE);
            }
        }
    }

    SECTION("Read constructor after writing.")
    {
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        {
            isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo, dataType);
            
            for (isx::isize_t f = 0; f < numFrames; ++f)
            {
                auto frame = std::make_shared<isx::VideoFrame>(
                    spacingInfo,
                    sizeof(uint16_t) * sizePixels.getWidth(),
                    1,
                    dataType,
                    timingInfo.convertIndexToStartTime(f),
                    f);
                std::fill(frame->getPixelsAsU16(), frame->getPixelsAsU16() + numPixels, 0xAFFE);
                void * p = frame->getPixels();
                movie.writeFrame(frame);
            }
        }

        isx::MosaicMovieFile movie(fileName);
        REQUIRE(movie.isValid());
        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
        REQUIRE(movie.getDataType() == dataType);
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie.readFrame(f);
            uint16_t * frameBuf = frame->getPixelsAsU16();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 0xAFFE);
            }
        }
    }

    SECTION("Write frame data and verify read matches it.")
    {
        ::writeTestU16Movie(fileName, timingInfo, spacingInfo);
        isx::MosaicMovieFile movie(fileName);
        REQUIRE(movie.isValid());
        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
        REQUIRE(movie.getDataType() == dataType);

        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie.readFrame(f);
            uint16_t * frameBuf = frame->getPixelsAsU16();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == (f * numPixels) + p);
            }
        }
    }

}

TEST_CASE("MosaicMovieFileF32", "[core-internal]")
{
    std::string fileName = g_resources["unitTestDataPath"] + "/movie.isxd";

    isx::Time start;
    isx::DurationInSeconds step(50, 1000);
    isx::isize_t numFrames = 5;
    isx::TimingInfo timingInfo(start, step, numFrames);

    isx::SizeInPixels_t sizePixels(4, 3);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(sizePixels, pixelSize, topLeft);

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
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        {
            isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo, dataType);           
            
            for (isx::isize_t f = 0; f < numFrames; ++f)
            {
                auto frame = std::make_shared<isx::VideoFrame>(
                    spacingInfo,
                    sizeof(float) * sizePixels.getWidth(),
                    1,
                    dataType,
                    timingInfo.convertIndexToStartTime(f),
                    f);
                std::fill(frame->getPixelsAsF32(), frame->getPixelsAsF32() + numPixels, 3.14159265f);
                movie.writeFrame(frame);
            }
        }
        
        isx::MosaicMovieFile movie(fileName);
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie.readFrame(f);
            float * frameBuf = frame->getPixelsAsF32();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 3.14159265f);
            }
        }
    }

    SECTION("Read constructor after writing.")
    {
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        {
            isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo, dataType);
            
            for (isx::isize_t f = 0; f < numFrames; ++f)
            {
                auto frame = std::make_shared<isx::VideoFrame>(
                    spacingInfo,
                    sizeof(float) * sizePixels.getWidth(),
                    1,
                    dataType,
                    timingInfo.convertIndexToStartTime(f),
                    f);
                std::fill(frame->getPixelsAsF32(), frame->getPixelsAsF32() + numPixels, 2.7182818f);
                void * p = frame->getPixels();
                movie.writeFrame(frame);
            }
        }

        isx::MosaicMovieFile movie(fileName);
        REQUIRE(movie.isValid());
        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
        REQUIRE(movie.getDataType() == dataType);
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie.readFrame(f);
            float * frameBuf = frame->getPixelsAsF32();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 2.7182818f);
            }
        }
    }

    SECTION("Write frame data and verify read matches it.")
    {
        ::writeTestF32Movie(fileName, timingInfo, spacingInfo);
        isx::MosaicMovieFile movie(fileName);

        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie.readFrame(f);
            float * frameBuf = frame->getPixelsAsF32();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == (f * numPixels) + p);
            }
        }
    }
}
