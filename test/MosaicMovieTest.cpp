#include "isxMosaicMovie.h"
#include "catch.hpp"
#include "isxTest.h"
#include <stdio.h>
#include <algorithm>

TEST_CASE("MosaicMovie", "[core-internal]")
{
    std::string fileName = g_resources["testDataPath"] + "/movie.isxd";

    isx::Time start;
    isx::DurationInSeconds step(50, 1000);
    isx::isize_t numFrames = 5;
    isx::TimingInfo timingInfo(start, step, numFrames);

    isx::SizeInPixels_t numPixels(4, 3);
    isx::SizeInMicrons_t pixelSize(isx::Ratio(22, 10), isx::Ratio(22, 10));
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

    SECTION("Empty constructor")
    {
        isx::MosaicMovie movie;
        REQUIRE(!movie.isValid());
    }

    SECTION("Write constructor.")
    {
        isx::MosaicMovie movie(fileName, timingInfo, spacingInfo);
        REQUIRE(movie.isValid());
    }

    SECTION("Read constructor after writing.")
    {
        {
            isx::MosaicMovie movie(fileName, timingInfo, spacingInfo);
        }
        isx::MosaicMovie movie(fileName);
        REQUIRE(movie.isValid());
        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
        isx::isize_t numPixels = spacingInfo.getTotalNumPixels();
        isx::SpU16VideoFrame_t frame;
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            frame = movie.getFrame(f);
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
        isx::isize_t frameSizeInBytes = sizeof(uint16_t) * spacingInfo.getTotalNumPixels();
        {
            isx::MosaicMovie movie(fileName, timingInfo, spacingInfo);
            for (isx::isize_t f = 0; f < numFrames; ++f)
            {
                isx::Time frameTime = timingInfo.convertIndexToTime(f);
                isx::SpU16VideoFrame_t frame = std::make_shared<isx::U16VideoFrame_t>(
                    spacingInfo,
                    rowSizeInBytes,
                    1, // numChannels
                    frameTime, f);

                std::vector<uint16_t> frameBuf(frame->getPixels(), frame->getPixels() + numPixels);
                for (isx::isize_t p = 0; p < numPixels; ++p)
                {
                    frameBuf[p] = uint16_t((f * numPixels) + p);
                }

                std::copy(frameBuf.data(), frameBuf.data() + numPixels, frame->getPixels());

                movie.writeFrame(frame);
            }
        }

        isx::MosaicMovie movie(fileName);
        REQUIRE(movie.isValid());
        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
        isx::SpU16VideoFrame_t frame;
        for (isx::isize_t f = 0; f < numFrames; ++f)
        {
            frame = movie.getFrame(f);
            std::vector<uint16_t> frameBuf(frame->getPixels(), frame->getPixels() + numPixels);
            for (isx::isize_t p = 0; p < numPixels; ++p)
            {
                REQUIRE(frameBuf[p] == (f * numPixels) + p);
            }
        }
    }
}
