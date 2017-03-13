#include "isxMovieFactory.h"
#include "isxCore.h"
#include "catch.hpp"
#include "isxTest.h"

#include <vector>
#include <thread>

namespace
{

void
writeTestData(isx::SpWritableMovie_t & inMovie)
{
    isx::TimingInfo timingInfo = inMovie->getTimingInfo();
    isx::SpacingInfo spacingInfo = inMovie->getSpacingInfo();

    isx::isize_t numFrames = timingInfo.getNumTimes();
    isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
    isx::isize_t rowSizeInBytes = sizeof(uint16_t) * spacingInfo.getNumColumns();

    for (isx::isize_t f = 0; f < numFrames; ++f)
    {
        isx::SpVideoFrame_t frame = std::make_shared<isx::VideoFrame>(
                spacingInfo,
                rowSizeInBytes,
                1, // numChannels
                inMovie->getDataType(),
                timingInfo.convertIndexToStartTime(f),
                f);

        uint16_t * pixelArray = frame->getPixelsAsU16();
        for (isx::isize_t p = 0; p < numPixels; ++p)
        {
            pixelArray[p] = uint16_t((f * numPixels) + p);
        }
        inMovie->writeFrame(frame);
    }
}

}

TEST_CASE("MovieFactoryReadMovieXml", "[core]")
{
    isx::CoreInitialize();

    SECTION("Read an XML file that wraps around one HDF5 files")
    {
        std::string fileName = g_resources["unitTestDataPath"] + "/recording_20160426_145041.xml";

        isx::Time start(2016, 4, 26, 14, 50, 41, isx::DurationInSeconds(930, 1000));
        isx::DurationInSeconds step(100, 1002);
        isx::isize_t numTimes = 33;
        isx::TimingInfo timingInfo(start, step, numTimes);

        isx::SizeInPixels_t numPixels(500, 500);
        isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
        isx::PointInMicrons_t topLeft(0, 0);
        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        isx::SpMovie_t movie = isx::readMovie(fileName);

        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        isx::SpVideoFrame_t firstFrame = movie->getFrame(0);
        uint16_t * firstFrameArray = firstFrame->getPixelsAsU16();
        REQUIRE(firstFrameArray[0] == 835);

        isx::SpVideoFrame_t lastFrame = movie->getFrame(numTimes - 1);
        uint16_t * lastFrameArray = lastFrame->getPixelsAsU16();
        REQUIRE(lastFrameArray[0] == 809);
    }

    SECTION("Read an XML file that wraps around two HDF5 files")
    {
        std::string fileName = g_resources["unitTestDataPath"] + "/recording_20160706_132714.xml";

        isx::Time start(2016, 7, 6, 13, 27, 23, isx::DurationInSeconds(999, 1000));
        isx::DurationInSeconds step(100, 1001);
        isx::isize_t numTimes = 82;
        isx::TimingInfo timingInfo(start, step, numTimes);

        isx::SizeInPixels_t numPixels(500, 500);
        isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
        isx::PointInMicrons_t topLeft(0, 0);
        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        isx::SpMovie_t movie = isx::readMovie(fileName);

        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        isx::SpVideoFrame_t firstFrame = movie->getFrame(0);
        uint16_t * firstFrameArray = firstFrame->getPixelsAsU16();
        REQUIRE(firstFrameArray[0] == 1416);

        isx::SpVideoFrame_t lastFrame = movie->getFrame(numTimes - 1);
        uint16_t * lastFrameArray = lastFrame->getPixelsAsU16();
        REQUIRE(lastFrameArray[0] == 721);
    }

    isx::CoreShutdown();
}

TEST_CASE("MovieFactoryReadMovieHdf5", "[core]")
{
    isx::CoreInitialize();

    SECTION("Read an HDF5 file")
    {
        std::string fileName = g_resources["unitTestDataPath"] + "/recording_20160426_145041.hdf5";

        isx::Time start(2016, 4, 26, 21, 50, 41, isx::DurationInSeconds(0, 1000));
        isx::DurationInSeconds step(96, 1000);
        isx::isize_t numTimes = 33;
        isx::TimingInfo timingInfo(start, step, numTimes);

        isx::SizeInPixels_t numPixels(500, 500);
        isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
        isx::PointInMicrons_t topLeft(0, 0);
        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        isx::SpMovie_t movie = isx::readMovie(fileName);

        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        isx::SpVideoFrame_t firstFrame = movie->getFrame(0);
        uint16_t * firstFrameArray = firstFrame->getPixelsAsU16();
        REQUIRE(firstFrameArray[0] == 835);

        isx::SpVideoFrame_t lastFrame = movie->getFrame(numTimes - 1);
        uint16_t * lastFrameArray = lastFrame->getPixelsAsU16();
        REQUIRE(lastFrameArray[0] == 809);
    }

    isx::CoreShutdown();
}

TEST_CASE("MovieFactoryMosaicMovie", "[core]")
{
    isx::CoreInitialize();

    std::string fileName = g_resources["unitTestDataPath"] + "/movie.isxd";
    std::remove(fileName.c_str());

    isx::Time start;
    isx::DurationInSeconds step(50, 1000);
    isx::isize_t numFrames = 5;
    isx::TimingInfo timingInfo(start, step, numFrames);

    isx::SizeInPixels_t numPixels(4, 3);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

    isx::DataType dataType = isx::DataType::U16;

    SECTION("Write a Mosaic movie file")
    {
        isx::SpWritableMovie_t movie = isx::writeMosaicMovie(fileName, timingInfo, spacingInfo, dataType);
        movie->closeForWriting();
        REQUIRE(movie->isValid());
    }

    SECTION("Read a Mosaic movie file")
    {
        {
            isx::SpWritableMovie_t movie = isx::writeMosaicMovie(fileName, timingInfo, spacingInfo, dataType);
            writeTestData(movie);
            movie->closeForWriting();
        }

        isx::SpMovie_t movie = isx::readMovie(fileName);

        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            isx::SpVideoFrame_t frame = movie->getFrame(f);
            uint16_t * pixelArray = frame->getPixelsAsU16();
            isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(pixelArray[p] == uint16_t((f * numPixels) + p));
            }
        }
    }

    isx::CoreShutdown();
}

TEST_CASE("MovieFactoryReadNVokeMovie", "[core]")
{
    isx::CoreInitialize();

    SECTION("Read an nVoke XML file")
    {
        const std::string fileName = g_resources["unitTestDataPath"] + "/nVoke/recording_20170130_165221.xml";

        const isx::Time start(2017, 1, 30, 16, 52, 21, isx::DurationInSeconds(754, 1000));
        const isx::DurationInSeconds step(100, 2001);
        const isx::isize_t numTimes = 3;
        const isx::TimingInfo timingInfo(start, step, numTimes);

        const isx::SizeInPixels_t numPixels(1440, 1080);
        const isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
        const isx::PointInMicrons_t topLeft(0, 0);
        const isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        const isx::SpMovie_t movie = isx::readMovie(fileName);

        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        isx::SpVideoFrame_t firstFrame = movie->getFrame(0);
        uint16_t * firstFrameArray = firstFrame->getPixelsAsU16();
        REQUIRE(firstFrameArray[0] == 178);

        isx::SpVideoFrame_t lastFrame = movie->getFrame(numTimes - 1);
        uint16_t * lastFrameArray = lastFrame->getPixelsAsU16();
        REQUIRE(lastFrameArray[0] == 179);
    }

    SECTION("Read a downsampled nVoke XML file")
    {
        const std::string fileName = g_resources["unitTestDataPath"] + "/nVoke/recording_20170130_165321.xml";

        const isx::Time start(2017, 1, 30, 16, 53, 21, isx::DurationInSeconds(95, 1000));
        const isx::DurationInSeconds step(100, 2001);
        const isx::isize_t numTimes = 4;
        const isx::TimingInfo timingInfo(start, step, numTimes);

        const isx::SizeInPixels_t numPixels(720, 540);
        const isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE * 2, isx::DEFAULT_PIXEL_SIZE * 2);
        const isx::PointInMicrons_t topLeft(0, 0);
        const isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        const isx::SpMovie_t movie = isx::readMovie(fileName);

        REQUIRE(movie->getTimingInfo() == timingInfo);
        REQUIRE(movie->getSpacingInfo() == spacingInfo);

        isx::SpVideoFrame_t firstFrame = movie->getFrame(0);
        uint16_t * firstFrameArray = firstFrame->getPixelsAsU16();
        REQUIRE(firstFrameArray[0] == 181);

        isx::SpVideoFrame_t lastFrame = movie->getFrame(numTimes - 1);
        uint16_t * lastFrameArray = lastFrame->getPixelsAsU16();
        REQUIRE(lastFrameArray[0] == 183);
    }


    isx::CoreShutdown();
}
