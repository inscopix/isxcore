#include "isxMosaicMovieFile.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxPathUtils.h"

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
    movie.closeForWriting();
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
    movie.closeForWriting();
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
        movie.closeForWriting();
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
            movie.closeForWriting();
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
            movie.closeForWriting();
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
        movie.closeForWriting();
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
            movie.closeForWriting();
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
            movie.closeForWriting();
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
    
    SECTION("Write after calling closeForWriting.")
    {
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        {
            isx::MosaicMovieFile movie(fileName, timingInfo, spacingInfo, dataType);           
            
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
                movie.writeFrame(frame);
            }
            movie.closeForWriting();
            
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
                movie.writeFrame(frame),
                isx::ExceptionFileIO,
                "Writing frame after file was closed for writing." + fileName);
        }
    }
}

TEST_CASE("MosaicMovieFileU16-withTimeStamps", "[core-internal]")
{
    const std::string outputDirPath = g_resources["unitTestDataPath"] + "/MosaicMovieFile";
    isx::makeDirectory(outputDirPath);

    const std::string filePath = outputDirPath + "/movie.isxd";

    const isx::Time start;
    const isx::DurationInSeconds step(50, 1000);
    const isx::isize_t numFrames = 5;
    const isx::TimingInfo timingInfo(start, step, numFrames);

    const std::vector<isx::Time> timeStamps =
    {
        start + isx::DurationInSeconds(1, 1000),
        start + isx::DurationInSeconds(49, 1000),
        start + isx::DurationInSeconds(102, 1000),
        start + isx::DurationInSeconds(148, 1000),
        start + isx::DurationInSeconds(207, 1000),
    };
    REQUIRE(timeStamps.size() == numFrames);

    const isx::SpacingInfo spacingInfo(isx::SizeInPixels_t(4, 3));
    const isx::isize_t totalNumPixels = spacingInfo.getTotalNumPixels();

    const isx::DataType dataType = isx::DataType::U16;

    SECTION("Read time stamps after writing them")
    {
        {
            isx::MosaicMovieFile movie(filePath, timingInfo, spacingInfo, dataType, true);
            for (isx::isize_t f = 0; f < numFrames; ++f)
            {
                isx::SpVideoFrame_t frame = movie.makeVideoFrame(f, timeStamps.at(f));
                REQUIRE(frame->getTimeStamp() == timeStamps.at(f));
                uint16_t * pixels = frame->getPixelsAsU16();
                for (isx::isize_t p = 0; p < totalNumPixels; ++p)
                {
                    pixels[p] = uint16_t(p);
                }
                movie.writeFrame(frame);
            }
            movie.closeForWriting();
        }

        isx::MosaicMovieFile movie(filePath);
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie.readFrame(f, true);
            uint16_t * pixels = frame->getPixelsAsU16();
            for (isx::isize_t p = 0; p < totalNumPixels; ++p)
            {
                REQUIRE(pixels[p] == uint16_t(p));
            }
            REQUIRE(frame->getTimeStamp() == timeStamps.at(f));
        }
    }

    isx::removeDirectory(outputDirPath);
    isx::CoreShutdown();
}
