#include "isxCore.h"
#include "isxMosaicMovie.h"
#include "catch.hpp"
#include "isxTest.h"
#include "MosaicMovieTest.h"

#include <stdio.h>
#include <algorithm>

TEST_CASE("MosaicMovieU16", "[core-internal]")
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
            auto frame = std::make_shared<isx::VideoFrame>(
                spacingInfo,
                sizeof(uint16_t) * sizePixels.getWidth(),
                1,
                dataType,
                timingInfo.convertIndexToStartTime(f),
                f);
            std::fill(frame->getPixelsAsU16(), frame->getPixelsAsU16() + numPixels, 0xCAFE);
            movie->writeFrame(frame);
        }

        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);
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
            auto movie = std::make_shared<isx::MosaicMovie>(
                    fileName, timingInfo, spacingInfo, dataType);
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
                movie->writeFrame(frame);
            }
            
        }
        auto movie = std::make_shared<isx::MosaicMovie>(fileName);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);
            uint16_t * frameBuf = frame->getPixelsAsU16();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 0xAFFE);
            }
        }
    }

    SECTION("Write frame data and verify read matches it.")
    {
        isx::SpWritableMovie_t movie = writeTestU16MovieGeneric(fileName, timingInfo, spacingInfo);
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

    SECTION("Read/Write movie with dropped frames")
    {
        std::vector<isx::isize_t> dropped{2, 4};
        isx::TimingInfo ti(start, step, numFrames, dropped);

        auto movie = std::make_shared<isx::MosaicMovie>(
                fileName, ti, spacingInfo, dataType);
    
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            if(f == 2 || f == 4)
            {
                continue;
            }

            auto frame = std::make_shared<isx::VideoFrame>(
                spacingInfo,
                sizeof(uint16_t) * sizePixels.getWidth(),
                1,
                dataType,
                timingInfo.convertIndexToStartTime(f),
                f);
            std::fill(frame->getPixelsAsU16(), frame->getPixelsAsU16() + numPixels, 0xCAFE);
            movie->writeFrame(frame);
        }

        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);
            uint16_t * frameBuf = frame->getPixelsAsU16();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                if(f == 2 || f == 4)
                {
                    REQUIRE(frameBuf[p] == 0x0000);
                }
                else
                {
                    REQUIRE(frameBuf[p] == 0xCAFE);
                }
            }
        }
    }

    isx::CoreShutdown();
}

TEST_CASE("MosaicMovieF32", "[core-internal]")
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
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        auto movie = std::make_shared<isx::MosaicMovie>(
                fileName, timingInfo, spacingInfo, dataType);
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
            movie->writeFrame(frame);
        }
        REQUIRE(movie->isValid());
        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);
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
            auto movie = std::make_shared<isx::MosaicMovie>(
                    fileName, timingInfo, spacingInfo, dataType);

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
                movie->writeFrame(frame);
            }
            
        }
        auto movie = std::make_shared<isx::MosaicMovie>(fileName);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);
            float * frameBuf = frame->getPixelsAsF32();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == 2.7182818f);
            }
        }
    }

    SECTION("Write frame data and verify read matches it.")
    {
        isx::SpWritableMovie_t movie = writeTestF32MovieGeneric(fileName, timingInfo, spacingInfo);
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

    SECTION("Make a middle video frame in a movie")
    {
        isx::SpWritableMovie_t movie = std::make_shared<isx::MosaicMovie>(
                fileName, timingInfo, spacingInfo, dataType);

        isx::SpVideoFrame_t frame = movie->makeVideoFrame(3);

        REQUIRE(frame->getImage().getSpacingInfo() == spacingInfo);
        REQUIRE(frame->getNumChannels() == 1);
        REQUIRE(frame->getDataType() == dataType);
        REQUIRE(frame->getTimeStamp() == timingInfo.convertIndexToStartTime(3));
        REQUIRE(frame->getFrameIndex() == 3);
    }

    isx::CoreShutdown();
}

TEST_CASE("Corrupt file", "[core-meme]")
{
    isx::CoreInitialize();

    SECTION("Header-only file")
    {
        std::string fileName = g_resources["unitTestDataPath"] + "/negative/NullDataSet.isxd";
        
        auto movie = std::make_shared<isx::MosaicMovie>(fileName);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getTimingInfo().getNumTimes() != 0);

        ISX_EXPECT_EXCEPTION();
        try
        {
            auto frame = movie->getFrame(0);
            FAIL("Failed to throw an exception.");
        }
        catch (const isx::ExceptionFileIO & error)
        {
            REQUIRE(std::string(error.what()) ==
                    "Error reading movie frame.");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionFileIO");
        }
    }

    isx::CoreShutdown();
}
