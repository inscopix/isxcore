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

TEST_CASE("MosaicMovieFileU16", "[core-internal][mosaic_movie_file]")
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

TEST_CASE("MosaicMovieFileF32", "[core-internal][mosaic_movie_file]")
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
            ISX_REQUIRE_EXCEPTION(movie.writeFrame(frame), isx::ExceptionFileIO, "");
        }
    }
}

TEST_CASE("MosaicMovieFileTimingInfo[MOS-1838]", "[core-internal][mosaic_movie_file]")
{
    std::string fileName = g_resources["unitTestDataPath"] + "/MOS-1838/50_frames_39999_to_39959.isxd";

    SECTION("Step modified according to tsc")
    {
        std::fstream fileStream(fileName, std::ios::binary | std::ios_base::in);
        if (!fileStream.good() || !fileStream.is_open())
        {
            ISX_THROW(isx::ExceptionFileIO, "Failed to open movie file for reading: ", fileName);
        }

        std::ios::pos_type headerPos;
        isx::json j = isx::readJsonHeaderAtEnd(fileStream, headerPos);
        isx::DurationInSeconds prevStep = isx::convertJsonToRatio(j["timingInfo"].at("period"));
        REQUIRE(prevStep == isx::DurationInSeconds::fromMicroseconds(39999));

        isx::MosaicMovieFile movie(fileName);
        isx::DurationInSeconds modStep = movie.getTimingInfo().getStep();
        REQUIRE(modStep == isx::DurationInSeconds(1957997, 49000000));
    }
}

TEST_CASE("MosaicMovieFileFrameHeaderMetadata", "[core-internal][mosaic_movie_file]")
{
    isx::CoreInitialize();

    SECTION("uint16 dual-color movie")
    {
        std::string u16MovieFileName = g_resources["unitTestDataPath"] + "/dual_color/DualColorMultiplexingMovie.isxd";
        isx::MosaicMovieFile movie(u16MovieFileName);
        REQUIRE(movie.isValid());
        REQUIRE(movie.getDataType() == isx::DataType::U16);

        std::unordered_map<std::string, uint64_t> actualFirstFrameMetadata = movie.readFrameHeaderMetadata(0);
        const std::unordered_map<std::string, uint64_t> expectedFirstFrameMetadata({
            {"led2VF", 99},
            {"led2Power", 0},
            {"frame_count", 8015257},
            {"write_enable", 173},
            {"led1VF", 15632},
            {"efocus", 470},
            {"led1Power", 2},
            {"tsc", 258308031709},
            {"color_id", 7}
        });
        REQUIRE(expectedFirstFrameMetadata == actualFirstFrameMetadata);

        std::unordered_map<std::string, uint64_t> actualSecondFrameMetadata = movie.readFrameHeaderMetadata(1);
        const std::unordered_map<std::string, uint64_t> expectedSecondFrameMetadata({
            {"led2VF", 17393},
            {"led2Power", 16},
            {"frame_count", 8015258},
            {"write_enable", 173},
            {"led1VF", 12947},
            {"efocus", 575},
            {"led1Power", 0},
            {"tsc", 258308048343},
            {"color_id", 7}
        });
        REQUIRE(expectedSecondFrameMetadata == actualSecondFrameMetadata);

        REQUIRE((actualSecondFrameMetadata.at("frame_count") - actualFirstFrameMetadata.at("frame_count")) == 1);
        REQUIRE(actualSecondFrameMetadata.at("tsc") > actualFirstFrameMetadata.at("tsc"));
    }

    isx::CoreShutdown();
}
