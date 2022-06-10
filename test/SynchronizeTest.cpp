#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxTest.h"
#include "isxSynchronize.h"
#include "isxPathUtils.h"
#include "isxMovieFactory.h"

#include "catch.hpp"
#include <json.hpp>

// Copied from PreprocessMovieTest.cpp
void
setHeaderTsc(uint16_t * inHeader, const uint64_t inTsc)
{
    std::array<uint16_t, 8> tscPixels;
    for (size_t i = 0; i < tscPixels.size(); ++i)
    {
        const uint64_t tscComp = (inTsc >> (8 * i)) & 0x00000000000000FF;
        tscPixels[i] = uint16_t(tscComp << 4);
    }

    std::memcpy(inHeader + 1272, &(tscPixels[0]), sizeof(tscPixels));
}

void
writeIsxdMovieWithEarlyDroppedFrames(
    const std::string & inFileName,
    const isx::TimingInfo & inTimingInfo,
    const isx::SpacingInfo & inSpacingInfo,
    const uint64_t inFirstTsc)
{
    isx::SpWritableMovie_t movie = isx::writeMosaicMovie(inFileName, inTimingInfo, inSpacingInfo, isx::DataType::U16, true);
    const isx::isize_t numFrames = inTimingInfo.getNumTimes();
    const isx::isize_t numPixels = inSpacingInfo.getTotalNumPixels();
    const uint64_t stepSizeUs = inTimingInfo.getStep().toMicroseconds();

    constexpr size_t numHeaderValues = 1280 * 2;
    std::unique_ptr<uint16_t[]> header(new uint16_t[numHeaderValues]);
    std::unique_ptr<uint16_t[]> pixels(new uint16_t[numPixels]);
    std::unique_ptr<uint16_t[]> footer(new uint16_t[numHeaderValues]);

    uint64_t currentTsc = inFirstTsc;
    for (isx::isize_t f = 0; f < numFrames; ++f)
    {
        if (!inTimingInfo.isIndexValid(f))
        {
            continue;
        }
        for (isx::isize_t p = 0; p < numPixels; ++p)
        {
            pixels[p] = uint16_t(f + p);
        }
        setHeaderTsc(header.get(), currentTsc);
        movie->writeFrameWithHeaderFooter(header.get(), pixels.get(), footer.get());
        currentTsc += stepSizeUs;
    }

    // fake extra props so that frame timestamps can be read from movie later
    using json = nlohmann::json;
    json extraProps;
    extraProps["producer"]["version"] = "1.9.0";
    movie->setExtraProperties(extraProps.dump());
    movie->closeForWriting();
}


TEST_CASE("SynchronizeStartTimes-Invalid", "[core]") 
{
    isx::CoreInitialize();

    SECTION("invalid data type for timing reference")
    {
        const std::string refFilename = g_resources["unitTestDataPath"] + "/imu/2020-02-13-18-43-21_video.imu";
        const std::vector<std::string> relFilenames = {g_resources["unitTestDataPath"] + "/nVision/20220412-200447-camera-100.isxb"};

        ISX_REQUIRE_EXCEPTION(
            isx::synchronizeStartTimes(
                refFilename,
                relFilenames
            ),
            isx::ExceptionUserInput,
            "Unsupported data type - only gpio files, isxd movies, and isxb movies are supported as a timing reference."
        );
    }

    SECTION("invalid data type for files to align to timing reference")
    {
        const std::string refFilename = g_resources["unitTestDataPath"] + "/gpio/2020-05-20-10-33-22_video.gpio";
        const std::vector<std::string> relFilenames = {g_resources["unitTestDataPath"] + "/cell_metrics/cell_metrics_movie-PCA-ICA.isxd"};

        ISX_REQUIRE_EXCEPTION(
            isx::synchronizeStartTimes(
                refFilename,
                relFilenames
            ),
            isx::ExceptionUserInput,
            "Unsupported data type - only isxd movies and isxb movies are supported as input files to align to a timing reference."
        );
    }

    SECTION("no frame timestamps in movie")
    {
        const std::string refFilename = g_resources["unitTestDataPath"] + "/gpio/2020-05-20-10-33-22_video.gpio";
        const std::vector<std::string> relFilenames = {g_resources["unitTestDataPath"] + "/cnmfe-cpp/movie_128x128x1000.isxd"};

        ISX_REQUIRE_EXCEPTION(
            isx::synchronizeStartTimes(
                refFilename,
                relFilenames
            ),
            isx::ExceptionUserInput,
            "Cannot get first tsc from movie with no frame timestamps."
        );
    }

    // TODO: implement test case when recording UUID is implemented in IDAS
    // SECTION("files from different recording")
    // {
    // }

    isx::CoreShutdown();
}

TEST_CASE("SynchronizeStartTimes", "[core]") 
{
    isx::CoreInitialize();

    const std::string testDir = g_resources["unitTestDataPath"] + "/nVision/paired";
    const std::string gpioFilename = testDir + "/miniscope/nVision_sync_ver_202205-06_1404.gpio";
    const std::string isxdFilename = testDir + "/miniscope/nVision_sync_ver_202205-06_1404.isxd";
    const std::string isxbFilename = testDir + "/behavior/nVision_sync_ver_202205-06_1404-camera-1.isxb";

    // Copy test isxb and isxd files to modify.
    const std::string isxdFilenameCopy = testDir + "/miniscope/nVision_sync_ver_202205-06_1404-mod.isxd";
    const std::string isxbFilenameCopy = testDir + "/behavior/nVision_sync_ver_202205-06_1404-camera-1-mod.isxb";
    isx::copyFile(isxdFilename, isxdFilenameCopy);
    isx::copyFile(isxbFilename, isxbFilenameCopy);

    // Verify the timing info of the modified isxd file
    SECTION("isxd timing info")
    {
        isx::synchronizeStartTimes(
            gpioFilename,
            {isxdFilenameCopy, isxbFilenameCopy}
        );

        const auto originalMovie = isx::readMovie(isxdFilename);
        const auto modifiedMovie = isx::readMovie(isxdFilenameCopy);

        // The calculated start time of an isxd file based on a gpio reference generally equals the start time
        // stored in the original isxd file because the isxd and gpio file originate from the same hardware system.
        // However this is not necessarily guaranteed to be the case which is why the isxd start time is recomputed just in case.
        REQUIRE(modifiedMovie->getTimingInfo() == originalMovie->getTimingInfo());
    }

    // Verify the timing info of the modified isxb file
    SECTION("isxb timing info")
    {
        isx::synchronizeStartTimes(
            gpioFilename,
            {isxdFilenameCopy, isxbFilenameCopy}
        );

        const auto originalMovie = isx::readMovie(isxbFilename);
        const auto modifiedMovie = isx::readMovie(isxbFilenameCopy);

        const auto originalTimingInfo = originalMovie->getTimingInfo();
        // The recomputed start time is 400 ms greater than the start time in the original isxb file
        // Generally there is a greater delay in the start of isxb recording because it's on a separate hardware system from the gpio and isxd files
        const isx::TimingInfo expTimingInfo(
            originalTimingInfo.getStart() + isx::DurationInSeconds::fromMilliseconds(400),
            originalTimingInfo.getStep(),
            originalTimingInfo.getNumTimes(),
            originalTimingInfo.getDroppedFrames(),
            originalTimingInfo.getCropped(),
            originalTimingInfo.getBlankFrames()
        );
        REQUIRE(modifiedMovie->getTimingInfo() == expTimingInfo);
    }

    // Ensure the frame data in the file is not corrupted due to the operation.
    SECTION("isxd frame data")
    {
        isx::synchronizeStartTimes(
            gpioFilename,
            {isxdFilenameCopy, isxbFilenameCopy}
        );

        const auto originalMovie = isx::readMovie(isxdFilename);
        const auto modifiedMovie = isx::readMovie(isxdFilenameCopy);

        for (size_t i = 0; i < originalMovie->getTimingInfo().getNumTimes(); i++)
        {
            const auto originalMovieFrame = originalMovie->getFrame(i);
            const auto modifiedMovieFrame = modifiedMovie->getFrame(i);
            requireEqualImages(originalMovieFrame->getImage(), modifiedMovieFrame->getImage());
        }
    }

    // Ensure the frame data in the file is not corrupted due to the operation.
    SECTION("isxb frame data")
    {
        isx::synchronizeStartTimes(
            gpioFilename,
            {isxdFilenameCopy, isxbFilenameCopy}
        );

        const auto originalMovie = isx::readMovie(isxbFilename);
        const auto modifiedMovie = isx::readMovie(isxbFilenameCopy);

        for (size_t i = 0; i < originalMovie->getTimingInfo().getNumTimes(); i++)
        {
            const auto originalMovieFrame = originalMovie->getFrame(i);
            const auto modifiedMovieFrame = modifiedMovie->getFrame(i);
            requireEqualImages(originalMovieFrame->getImage(), modifiedMovieFrame->getImage());
        }
    }

    // Ensure the json metadata is not corrupted due to the operation.
    SECTION("isxd extra props")
    {
        isx::synchronizeStartTimes(
            gpioFilename,
            {isxdFilenameCopy, isxbFilenameCopy}
        );

        const auto originalMovie = isx::readMovie(isxdFilename);
        const auto modifiedMovie = isx::readMovie(isxdFilenameCopy);
        REQUIRE(modifiedMovie->getExtraProperties() == originalMovie->getExtraProperties());
    }

    // Ensure the json metadata is not corrupted due to the operation.
    SECTION("isxb extra props")
    {
        isx::synchronizeStartTimes(
            gpioFilename,
            {isxdFilenameCopy, isxbFilenameCopy}
        );

        const auto originalMovie = isx::readMovie(isxbFilename);
        const auto modifiedMovie = isx::readMovie(isxbFilenameCopy);
        REQUIRE(modifiedMovie->getExtraProperties() == originalMovie->getExtraProperties());
    }

    isx::CoreShutdown();

    std::remove(isxdFilenameCopy.c_str());
    std::remove(isxbFilenameCopy.c_str());
}

TEST_CASE("SynchronizeStartTimes-EarlyDroppedFrames", "[core]") 
{
    isx::CoreInitialize();

    const std::string testDir = g_resources["unitTestDataPath"] + "/nVision/paired";
    const std::string gpioFilename = testDir + "/miniscope/nVision_sync_ver_202205-06_1404.gpio";
    const std::string isxdFilename = testDir + "/synthetic.isxd";
    const std::string isxbFilename = testDir + "/behavior/nVision_sync_ver_202205-06_1404-camera-1.isxb";

    // Copy test isxb file to modify.
    const std::string isxbFilenameCopy = testDir + "/behavior/nVision_sync_ver_202205-06_1404-camera-1-mod.isxb";
    isx::copyFile(isxbFilename, isxbFilenameCopy);

    SECTION("First frame of isxd movie is dropped")
    {
        // Create synthetic isxd movie with early dropped frames
        const uint64_t gpioFirstTsc = 4223827383891;
        const uint64_t isxdFirstValidTsc = gpioFirstTsc + uint64_t(500 * 1e3); // 500 ms greater than gpioFirstTsc
        writeIsxdMovieWithEarlyDroppedFrames(
            isxdFilename,
            isx::TimingInfo(isx::Time(), isx::DurationInSeconds(1, 10), 19, {0}),
            isx::SpacingInfo(isx::SizeInPixels_t(4, 3)),
            isxdFirstValidTsc
        );

        const uint64_t gpioStartTimestamp = 1651842274268;

        // First frame is dropped
        // TSC value of first frame is estimated to be: isxdFirstValidTsc - stepSize = isxdFirstValidTsc - 100 ms
        // which means start time of isxd movie is estimated to be 400 ms after the gpio start time
        const uint64_t expIsxdStartTimestamp = gpioStartTimestamp + 400;

        isx::synchronizeStartTimes(
            gpioFilename,
            {isxdFilename, isxbFilenameCopy}
        );

        const auto movie = isx::readMovie(isxdFilename);
        const uint64_t isxdStartTimestamp = uint64_t(movie->getTimingInfo().getStart().getSecsSinceEpoch().getNum());
        REQUIRE(isxdStartTimestamp == expIsxdStartTimestamp);
    }

    SECTION("First three frames of isxd movie are dropped")
    {
        // Create synthetic isxd movie with early dropped frames
        const uint64_t gpioFirstTsc = 4223827383891;
        const uint64_t isxdFirstValidTsc = gpioFirstTsc + uint64_t(500 * 1e3); // 500 ms greater than gpioFirstTsc
        writeIsxdMovieWithEarlyDroppedFrames(
            isxdFilename,
            isx::TimingInfo(isx::Time(), isx::DurationInSeconds(1, 10), 19, {0, 1, 2}),
            isx::SpacingInfo(isx::SizeInPixels_t(4, 3)),
            isxdFirstValidTsc
        );

        const uint64_t gpioStartTimestamp = 1651842274268;

        // First three frames are dropped
        // TSC value of first frame is estimated to be: isxdFirstValidTsc - stepSize = isxdFirstValidTsc - 300 ms
        // which means start time of isxd movie is estimated to be 200 ms after the gpio start time
        const uint64_t expIsxdStartTimestamp = gpioStartTimestamp + 200;

        isx::synchronizeStartTimes(
            gpioFilename,
            {isxdFilename, isxbFilenameCopy}
        );

        const auto movie = isx::readMovie(isxdFilename);
        const uint64_t isxdStartTimestamp = uint64_t(movie->getTimingInfo().getStart().getSecsSinceEpoch().getNum());
        REQUIRE(isxdStartTimestamp == expIsxdStartTimestamp);
    }

    isx::CoreShutdown();

    std::remove(isxdFilename.c_str());
    std::remove(isxbFilenameCopy.c_str());
}
