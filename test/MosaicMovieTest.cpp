#include "isxCore.h"
#include "isxMosaicMovie.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxPathUtils.h"
#include "isxMovieFactory.h"
#include "MosaicMovieTest.h"

#include <stdio.h>
#include <algorithm>

#include "json.hpp"
using json = nlohmann::json;

TEST_CASE("MosaicMovieU16", "[core-internal][mosaic_movie]")
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
        REQUIRE(movie->getOriginalSpacingInfo() == isx::SpacingInfo::getDefault());

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

TEST_CASE("MosaicMovieF32", "[core-internal][mosaic_movie]")
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
            ISX_REQUIRE_EXCEPTION(movie->writeFrame(frame), isx::ExceptionFileIO, "");
        }
    }

    isx::CoreShutdown();
}

TEST_CASE("Corrupt file", "[core][mosaic_movie]")
{
    isx::CoreInitialize();

    SECTION("Header-only file")
    {
        std::string fileName = g_resources["unitTestDataPath"] + "/negative/NullDataSet_he.isxd";

        auto movie = std::make_shared<isx::MosaicMovie>(fileName);
        REQUIRE(movie->isValid());
        REQUIRE(movie->getTimingInfo().getNumTimes() != 0);

        ISX_REQUIRE_EXCEPTION(movie->getFrame(0), isx::ExceptionFileIO, "");
    }

    isx::CoreShutdown();
}

TEST_CASE("MosaicMovie-croppedFrames", "[core][mosaic_movie]")
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

TEST_CASE("MosaicMovie-RGB888", "[core-internal][mosaic_movie]")
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

TEST_CASE("MosaicMovieU16-forTheHub", "[core][mosaic_movie]")
{
    const std::string outputDirPath = g_resources["unitTestDataPath"] + "/MosaicMovieFile";
    isx::makeDirectory(outputDirPath);

    isx::CoreInitialize();

    SECTION("Read frame, frame header/footer and properties after writing")
    {
        const std::string filePath = outputDirPath + "/movie.isxd";

        const isx::Time start;
        const isx::DurationInSeconds step(50, 1000);
        const isx::isize_t numFrames = 3;
        const isx::TimingInfo timingInfo(start, step, numFrames);

        isx::SizeInPixels_t numPixels(1280, 800);

        SECTION("1280x800 - full resolution")
        {
            numPixels = isx::SizeInPixels_t(1280, 800);
        }

        SECTION("640x400 - downsampled by a factor of 2")
        {
            numPixels = isx::SizeInPixels_t(640, 400);
        }

        SECTION("567x272 - downsampled by a factor of 2 and cropped")
        {
            numPixels = isx::SizeInPixels_t(567, 272);
        }

        SECTION("426x266 - downsampled by a factor of 3")
        {
            numPixels = isx::SizeInPixels_t(426, 266);
        }

        SECTION("320x200 - downsampled by a factor of 4")
        {
            numPixels = isx::SizeInPixels_t(320, 200);
        }

        const isx::SpacingInfo spacingInfo(numPixels);
        const isx::isize_t totalNumPixels = spacingInfo.getTotalNumPixels();
        const size_t frameSizeInBytes = totalNumPixels * sizeof(uint16_t);
        const isx::DataType dataType = isx::DataType::U16;

        // The header, frame, and footer values are all some function of
        // the frame and pixel index.
        // Frame values are bounded by the 12-bit sensor, whereas header
        // and footer values are only bounded by the uint16_t type.
        const size_t maxImageValue = 4095;
        const size_t maxHeaderFooterValue = 65535;
        const size_t numHeaderValues = 2 * 1280;
        const size_t headerSizeInBytes = numHeaderValues * sizeof(uint16_t);
        const size_t numFooterValues = numHeaderValues;
        const size_t footerSizeInBytes = numFooterValues * sizeof(uint16_t);

        const uint16_t headerValue = 137;
        const uint16_t footerValue = 92;

        std::vector<std::vector<uint16_t>> headers(numFrames);
        std::vector<std::vector<uint16_t>> frames(numFrames);
        std::vector<std::vector<uint16_t>> footers(numFrames);
        for (size_t f = 0; f < numFrames; ++f)
        {
            headers.at(f) = std::vector<uint16_t>(numHeaderValues, headerValue);

            frames.at(f) = std::vector<uint16_t>(totalNumPixels);
            for (isx::isize_t p = 0; p < totalNumPixels; ++p)
            {
                frames.at(f).at(p) = uint16_t(hashFrameAndPixelIndex(f, p, maxImageValue));
            }

            footers.at(f) = std::vector<uint16_t>(numFooterValues, footerValue);
        }

        using json = nlohmann::json;
        json extraProperties;
        extraProperties["probe"]["name"] = "ISX3821092";
        extraProperties["probe"]["type"] = "straight";
        extraProperties["probe"]["length"] = 8;
        extraProperties["probe"]["diameter"] = 0.6;
        extraProperties["microscope"]["EX-LED power"] = 9000;
        extraProperties["microscope"]["Spatial downsample"] = 2;
        extraProperties["microscope"]["FOV"]["width"] = numPixels.getWidth();
        extraProperties["microscope"]["FOV"]["height"] = numPixels.getHeight();
        const std::string extraPropertiesStr = extraProperties.dump();

        // Write the frames with headers and footers to the file.
        std::unique_ptr<uint16_t[]> buffer(new uint16_t[numHeaderValues + totalNumPixels + numFooterValues]);
        {
            isx::SpWritableMovie_t movie = isx::writeMosaicMovie(filePath, timingInfo, spacingInfo, dataType, true);
            for (isx::isize_t f = 0; f < numFrames; ++f)
            {
                if (numPixels == isx::SizeInPixels_t(1280, 800))
                {
                    std::memcpy(buffer.get(), headers.at(f).data(), headerSizeInBytes);
                    std::memcpy(buffer.get() + numHeaderValues, frames.at(f).data(), frameSizeInBytes);
                    std::memcpy(buffer.get() + numHeaderValues + totalNumPixels, footers.at(f).data(), footerSizeInBytes);
                    movie->writeFrameWithHeaderFooter(buffer.get());
                }
                else
                {
                    movie->writeFrameWithHeaderFooter(headers.at(f).data(), frames.at(f).data(), footers.at(f).data());
                }
            }
            movie->setExtraProperties(extraProperties.dump());
            movie->closeForWriting();
        }

        // Check we get the frame data, header, and footer back.
        const isx::SpMovie_t movie = isx::readMovie(filePath);
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            const isx::SpVideoFrame_t frame = movie->getFrame(f);
            REQUIRE(frame->getImage().getSpacingInfo() == spacingInfo);
            REQUIRE(std::memcmp(frame->getPixelsAsU16(), frames.at(f).data(), frameSizeInBytes) == 0);
            REQUIRE(movie->getFrameHeader(f) == headers.at(f));
            REQUIRE(movie->getFrameFooter(f) == footers.at(f));
        }

        REQUIRE(movie->getOriginalSpacingInfo() == isx::SpacingInfo::getDefaultForNVista3());

        // Finally check the extra properties
        REQUIRE(movie->getExtraProperties() == extraPropertiesStr);
    }

    SECTION("Read movie written by version 0.1 of the hub (no start yet).")
    {
        const std::string movieFilePath = g_resources["unitTestDataPath"] + "/hub/2018-03-27-11-15-57_video.isxd";

        const isx::SpMovie_t movie = isx::readMovie(movieFilePath);

        const isx::TimingInfo expTi = isx::TimingInfo(isx::Time(), isx::DurationInSeconds(3, 1000), 7);
        const isx::SpacingInfo expSi = isx::SpacingInfo(isx::SizeInPixels_t(1280, 800));

        REQUIRE(movie->getTimingInfo() == expTi);
        REQUIRE(movie->getSpacingInfo() == expSi);
        REQUIRE(movie->getDataType() == isx::DataType::U16);

        const size_t width = expSi.getNumPixels().getWidth();

        // Check frame 0 and its header.
        {
            const isx::SpVideoFrame_t frame = movie->getFrame(0);
            REQUIRE(frame->getImage().getSpacingInfo() == expSi);
            const uint16_t * pixels = frame->getPixelsAsU16();
            REQUIRE(pixels[0] == 365);
            REQUIRE(pixels[79 + 101*width] == 1247);

            const std::vector<uint16_t> header = movie->getFrameHeader(0);
            REQUIRE(header[0] == 160);
            REQUIRE(header[52] == 2128);
        }

        // Check frame 6 and its header.
        {
            const isx::SpVideoFrame_t frame = movie->getFrame(6);
            REQUIRE(frame->getImage().getSpacingInfo() == expSi);
            const uint16_t * pixels = frame->getPixelsAsU16();
            REQUIRE(pixels[0] == 361);
            REQUIRE(pixels[79 + 101*width] == 1283);

            const std::vector<uint16_t> header = movie->getFrameHeader(6);
            REQUIRE(header[0] == 160);
            REQUIRE(header[52] == 2128);
        }

        REQUIRE(movie->getOriginalSpacingInfo() == isx::SpacingInfo::getDefaultForNVista3());

        // Finally check some of the extra properties
        json extraProps = json::parse(movie->getExtraProperties());
        REQUIRE(extraProps["animal"]["sex"] == "m");
        REQUIRE(extraProps["animal"]["weight"] == "20");
        REQUIRE(extraProps["date"] == "Tue Mar 27 2018 11:10:18 GMT-0700 (Pacific Daylight Time)");
        REQUIRE(extraProps["gpio"][0]["mode"] == "disabled");
        REQUIRE(extraProps["gpio"][0]["type"] == "analog");
        REQUIRE(extraProps["microscope"]["binMode"] == "2");
        REQUIRE(extraProps["microscope"]["type"] == "nVista");
        REQUIRE(extraProps["probe"]["diameter"] == "0.5");
        REQUIRE(extraProps["probe"]["type"] == "Straight Lens");
        REQUIRE(extraProps["name"] == "Session 20180327111018");
    }

    isx::CoreShutdown();
    isx::removeDirectory(outputDirPath);
}

TEST_CASE("MosaicMovie-negative", "[core][mosaic_movie]")
{
    isx::CoreInitialize();

    const std::string inputDirPath = g_resources["unitTestDataPath"] + "/negative";

    SECTION("Empty file")
    {
        const std::string inputFilePath = inputDirPath + "/empty.isxd";
        ISX_REQUIRE_EXCEPTION(isx::readMovie(inputFilePath), isx::Exception, "");
    }

    isx::CoreShutdown();
}
