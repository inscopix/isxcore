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
        movie->closeForWriting();
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
        movie->closeForWriting();

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
            movie->closeForWriting();
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
        movie->closeForWriting();

        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);
            if (f == 2 || f == 4)
            {
                REQUIRE(frame->getFrameType() == isx::VideoFrame::Type::DROPPED);
            }
            else
            {
                REQUIRE(frame->getFrameType() == isx::VideoFrame::Type::VALID);
            }
            uint16_t * frameBuf = frame->getPixelsAsU16();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                if (f == 2 || f == 4)
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
        movie->closeForWriting();
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
        movie->closeForWriting();
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
            movie->closeForWriting();
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
        movie->closeForWriting();

        REQUIRE(frame->getImage().getSpacingInfo() == spacingInfo);
        REQUIRE(frame->getNumChannels() == 1);
        REQUIRE(frame->getDataType() == dataType);
        REQUIRE(frame->getTimeStamp() == timingInfo.convertIndexToStartTime(3));
        REQUIRE(frame->getFrameIndex() == 3);
    }

    SECTION("Write after calling closeForWriting.")
    {
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        {
            auto movie = std::make_shared<isx::MosaicMovie>(
                fileName, timingInfo, spacingInfo, dataType);
            
            for (isx::isize_t f = 0; f < numFrames - 1; ++f)
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
            movie->closeForWriting();
            
            const auto f = numFrames - 1;
            auto frame = std::make_shared<isx::VideoFrame>(
                spacingInfo,
                sizeof(float) * sizePixels.getWidth(),
                1,
                dataType,
                timingInfo.convertIndexToStartTime(f),
                f);
            std::fill(frame->getPixelsAsF32(), frame->getPixelsAsF32() + numPixels, 3.14159265f);
            ISX_REQUIRE_EXCEPTION(
                movie->writeFrame(frame),
                isx::ExceptionFileIO,
                "Writing frame after file was closed for writing." + fileName);
        }
    }

    isx::CoreShutdown();
}

TEST_CASE("Corrupt file", "[core-meme]")
{
    isx::CoreInitialize();

    SECTION("Header-only file")
    {
        std::string fileName = g_resources["unitTestDataPath"] + "/negative/NullDataSet_he.isxd";
        
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

TEST_CASE("MosaicMovie-croppedFrames", "[core]")
{
    isx::CoreInitialize();

    const std::string fileName = g_resources["unitTestDataPath"] + "/movie.isxd";
    std::remove(fileName.c_str());

    const isx::SpacingInfo si(isx::SizeInPixels_t(4, 3));
    const isx::DataType dataType = isx::DataType::U16;

    const isx::Time start(1970, 1, 1, 0, 0, 0);
    const isx::DurationInSeconds step(50, 1000);
    const isx::isize_t numTimes = 10;

    isx::TimingInfo ti(start, step, numTimes);

    SECTION("Single cropped frame")
    {
        const isx::IndexRanges_t ranges = {isx::IndexRange(2)};
        ti = isx::TimingInfo(start, step, numTimes, {}, ranges);
    }

    SECTION("Single and range of cropped frames")
    {
        const isx::IndexRanges_t ranges = {isx::IndexRange(2), isx::IndexRange(5, 8)};
        ti = isx::TimingInfo(start, step, numTimes, {}, ranges);
    }

    SECTION("Two ranges of cropped frames")
    {
        const isx::IndexRanges_t ranges = {isx::IndexRange(2, 3), isx::IndexRange(5, 8)};
        ti = isx::TimingInfo(start, step, numTimes, {}, ranges);
    }

    auto movie = std::make_shared<isx::MosaicMovie>(fileName, ti, si, dataType);

    const isx::isize_t numPixels = si.getTotalNumPixels();
    for (isx::isize_t t = 0; t < numTimes; ++t)
    {
        if (ti.isCropped(t))
        {
            continue;
        }
        auto frame = movie->makeVideoFrame(t);
        uint16_t * pixels = frame->getPixelsAsU16();
        std::fill(pixels, pixels + numPixels, uint16_t(t));
        movie->writeFrame(frame);
    }
    movie->closeForWriting();

    for (isx::isize_t t = 0; t < numTimes; ++t)
    {
        isx::SpVideoFrame_t frame = movie->getFrame(t);
        if (ti.isCropped(t))
        {
            REQUIRE(frame->getFrameType() == isx::VideoFrame::Type::CROPPED);
        }
        else
        {
            REQUIRE(frame->getFrameType() == isx::VideoFrame::Type::VALID);
        }
        uint16_t * frameBuf = frame->getPixelsAsU16();
        for (isx::isize_t p = 0; p < numPixels; ++p)
        {
            if (ti.isCropped(t))
            {
                REQUIRE(frameBuf[p] == 0);
            }
            else
            {
                REQUIRE(frameBuf[p] == t);
            }
        }
    }

    isx::CoreShutdown();
}

TEST_CASE("MosaicMovie-RGB888", "[core-internal]")
{
    isx::CoreInitialize();

    SECTION("Read real file.")
    {
        const std::string fileName = g_resources["unitTestDataPath"] + "/Movie_2016-02-11-08-46-14_rgb_test.isxd";

        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::isize_t numFrames = 15;
        isx::TimingInfo timingInfo(start, step, numFrames);

        isx::SizeInPixels_t sizePixels(1280, 800);
        isx::SizeInMicrons_t pixelSize(isx::Ratio(4500, 1000), isx::Ratio(4500, 1000));
        isx::PointInMicrons_t topLeft(isx::Ratio(100000, 1000), isx::Ratio(100000, 1000));
        isx::SpacingInfo spacingInfo(sizePixels, pixelSize, topLeft);

        isx::DataType dataType = isx::DataType::RGB888;

        auto movie = std::make_shared<isx::MosaicMovie>(fileName);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);
        REQUIRE(movie->getDataType() == dataType);

        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);

            // The number of channels is currently 1 as we don't really use that dimension
            // anywhere. That might need to change in the future.
            // The only place the number of channels is important is in
            // getDataTypeSizeInBytes where the factor of 3 is explicitly defined for RGB888.
            REQUIRE(frame->getNumChannels() == 1);
        }
    }

    isx::CoreShutdown();
}

TEST_CASE("MosaicMovieCreateRGB888Sample", "[core-internal][!hide]")
{
    isx::CoreInitialize();

    SECTION("Write sample file")
    {
        std::string fileName = g_resources["unitTestDataPath"] + "/movieRGB888.isxd";

        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::isize_t numFrames = 5;
        isx::TimingInfo timingInfo(start, step, numFrames);

        isx::SizeInPixels_t sizePixels(16, 10);//1200, 1024);
        isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
        isx::PointInMicrons_t topLeft(0, 0);
        isx::SpacingInfo spacingInfo(sizePixels, pixelSize, topLeft);

        isx::DataType dataType = isx::DataType::RGB888;

        auto movie = std::make_shared<isx::MosaicMovie>(
                fileName, timingInfo, spacingInfo, dataType);

        auto w  = sizePixels.getWidth();
        auto hw = w / 2;
        auto h  = sizePixels.getHeight();
        auto hh = h / 2;

        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            auto frame = std::make_shared<isx::VideoFrame>(
                spacingInfo,
                isx::getDataTypeSizeInBytes(dataType) * w,
                1,
                dataType,
                timingInfo.convertIndexToStartTime(f),
                f);

            auto p = reinterpret_cast<uint8_t *>(frame->getPixels());
            for (isx::isize_t y = 0; y < hh; ++y)
            {
                // Red
                for (isx::isize_t x = 0; x < hw; ++x)
                {
                    *p++ = 0xff;
                    *p++ = 0x00;
                    *p++ = 0x00;
                }

                // Green
                for (isx::isize_t x = hw; x < w; ++x)
                {
                    *p++ = 0x00;
                    *p++ = 0xff;
                    *p++ = 0x00;
                }
            }

            for (isx::isize_t y = hh; y < h; ++y)
            {
                // Blue
                for (isx::isize_t x = 0; x < hw; ++x)
                {
                    *p++ = 0x00;
                    *p++ = 0x00;
                    *p++ = 0xff;
                }

                // Yellow-ish
                for (isx::isize_t x = hw; x < w; ++x)
                {
                    *p++ = 0xff;
                    *p++ = (x - hw) % 256;
                    *p++ = 0x00;
                }
            }
            movie->writeFrame(frame);
        }
        movie->closeForWriting();
        REQUIRE(movie->isValid());
    }

    isx::CoreShutdown();
}
