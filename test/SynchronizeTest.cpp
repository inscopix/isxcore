#if ISX_WITH_ALGOS
#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxTest.h"
#include "isxSynchronize.h"
#include "isxPathUtils.h"
#include "isxMovieFactory.h"
// #include "isxPreprocessMovie.h"

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
    const uint64_t inFirstTsc,
    const std::string inRecordingUUID)
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
    extraProps["processingInterface"]["recordingUUID"] = inRecordingUUID;
    movie->setExtraProperties(extraProps.dump());
    movie->closeForWriting();
}


TEST_CASE("AlignStartTimes-Invalid", "[core]") 
{
    isx::CoreInitialize();

    std::string processedFilename;

    SECTION("invalid data type for timing reference")
    {
        const std::string refFilename = g_resources["unitTestDataPath"] + "/imu/2020-02-13-18-43-21_video.imu";
        const std::vector<std::string> relFilenames = {g_resources["unitTestDataPath"] + "/nVision/20220412-200447-camera-100.isxb"};

        ISX_REQUIRE_EXCEPTION(
            isx::alignStartTimes(
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
            isx::alignStartTimes(
                refFilename,
                relFilenames
            ),
            isx::ExceptionUserInput,
            "Unsupported data type - only isxd movies and isxb movies are supported as input files to align to a timing reference."
        );
    }

    SECTION("no frame timestamps in movie")
    {
        const std::string testDataDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-synchronized/manual";
        const std::string refFilename = testDataDir + "/2022-06-08-23-53-41_video.gpio";
        const std::string relFilename = testDataDir + "/2022-06-08-23-53-41_video.isxd";
        processedFilename = testDataDir + "/2022-06-08-23-53-41_video-PP.isxd";

        // preprocess movie to strip the movie of frame timestamps
        const auto movie = isx::readMovie(relFilename);
        const size_t temporalDsFactor = 2; // temporally downsample by 2x to strip movie of timestamps
        const size_t spatialDsFactor = 1;
        const isx::PreprocessMovieParams preprocessParams(
                movie,
                processedFilename,
                temporalDsFactor,
                spatialDsFactor,
                movie->getSpacingInfo().getFovInPixels(),
                true);
        auto outputParams = std::make_shared<isx::PreprocessMovieOutputParams>();
        isx::preprocessMovie(preprocessParams, outputParams, [](float){return false;});

        ISX_REQUIRE_EXCEPTION(
            isx::alignStartTimes(
                refFilename,
                {processedFilename}
            ),
            isx::ExceptionUserInput,
            "Cannot get first tsc from movie with no frame timestamps."
        );
    }

    SECTION("timing reference with no recording UUID metadata")
    {
        const std::string refFilename = g_resources["unitTestDataPath"] + "/gpio/2020-05-20-10-33-22_video.gpio";
        const std::vector<std::string> relFilenames = {g_resources["unitTestDataPath"] + "/cnmfe-cpp/movie_128x128x1000.isxd"};

        ISX_REQUIRE_EXCEPTION(
            isx::alignStartTimes(
                refFilename,
                relFilenames
            ),
            isx::ExceptionUserInput,
            "Cannot determine if files are paired and synchronized - no recording UUID in timing reference file metadata."
        );
    }

    SECTION("paired and unsynchronized")
    {
        const std::string testDataDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-unsynchronized";
        const std::string refFilename = testDataDir + "/2022-06-08-23-57-41_video.gpio";
        const std::vector<std::string> relFilenames = {testDataDir + "/2022-06-08-23-57-43-camera-1.isxb"};

        ISX_REQUIRE_EXCEPTION(
            isx::alignStartTimes(
                refFilename,
                relFilenames
            ),
            isx::ExceptionUserInput,
            "Files are not paired and synchronized - recording UUID of align file (AC-00111111-l4R4GRt28o-1654732663355) does not match recording UUID of timing reference file (AC-00111111-l4R4GRt28o-1654732661796)."
        );
    }

    SECTION("standalone")
    {
        const std::string testDataDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/";
        const std::string refFilename = testDataDir + "/standalone-miniscope/2022-06-08-23-58-43_video.gpio";
        const std::vector<std::string> relFilenames = {testDataDir + "/standalone-behavior/2022-06-08-23-58-51-camera-1.isxb"};

        ISX_REQUIRE_EXCEPTION(
            isx::alignStartTimes(
                refFilename,
                relFilenames
            ),
            isx::ExceptionUserInput,
            "Files are not paired and synchronized - recording UUID of align file (GA-21807233-0000000000-1654732731777) does not match recording UUID of timing reference file (AC-00111111-0000000000-1654732723918)."
        );
    }


    SECTION("input filenames")
    {
        const std::string testDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-synchronized/manual";
        const std::string gpioFilename = testDir + "/2022-06-08-23-53-41_video.gpio";
        const std::string isxbFilename = testDir + "/2022-06-08-23-53-41_video-camera-1.isxb";

        processedFilename = testDir + "/2022-06-08-23-53-41_video-camera-1-mod.isxb";
        isx::copyFile(isxbFilename, processedFilename);

        isx::alignStartTimes(
            gpioFilename,
            {gpioFilename, processedFilename, processedFilename}
        );
    }

    if (!processedFilename.empty())
    {
        std::remove(processedFilename.c_str());
    }

    isx::CoreShutdown();
}

TEST_CASE("AlignStartTimes-GpioRef", "[core]") 
{
    isx::CoreInitialize();

    const std::string testDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-synchronized/manual";
    const std::string gpioFilename = testDir + "/2022-06-08-23-53-41_video.gpio";
    const std::string isxdFilename = testDir + "/2022-06-08-23-53-41_video.isxd";
    const std::string isxbFilename = testDir + "/2022-06-08-23-53-41_video-camera-1.isxb";

    // Copy test isxb and isxd files to modify.
    const std::string isxdFilenameCopy = testDir + "/2022-06-08-23-53-41_video-mod.isxd";
    const std::string isxbFilenameCopy = testDir + "/2022-06-08-23-53-41_video-camera-1-mod.isxb";
    isx::copyFile(isxdFilename, isxdFilenameCopy);
    isx::copyFile(isxbFilename, isxbFilenameCopy);

    // Verify the timing info of the modified isxd file
    SECTION("isxd timing info")
    {
        isx::alignStartTimes(
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
        isx::alignStartTimes(
            gpioFilename,
            {isxdFilenameCopy, isxbFilenameCopy}
        );

        const auto originalMovie = isx::readMovie(isxbFilename);
        const auto modifiedMovie = isx::readMovie(isxbFilenameCopy);

        const auto originalTimingInfo = originalMovie->getTimingInfo();
        // The recomputed start time is 541 ms greater than the start time in the original isxb file
        // Generally there is a greater delay in the start of isxb recording because it's on a separate hardware system from the gpio and isxd files
        const isx::TimingInfo expTimingInfo(
            originalTimingInfo.getStart() + isx::DurationInSeconds::fromMilliseconds(541),
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
        isx::alignStartTimes(
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
        isx::alignStartTimes(
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
        isx::alignStartTimes(
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
        isx::alignStartTimes(
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

TEST_CASE("AlignStartTimes-IsxdRef", "[core]") 
{
    isx::CoreInitialize();

    const std::string testDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-synchronized/manual";
    const std::string isxdFilename = testDir + "/2022-06-08-23-53-41_video.isxd";
    const std::string isxbFilename = testDir + "/2022-06-08-23-53-41_video-camera-1.isxb";

    // Copy test isxb and isxd files to modify.
    const std::string isxbFilenameCopy = testDir + "/2022-06-08-23-53-41_video-camera-1-mod.isxb";
    isx::copyFile(isxbFilename, isxbFilenameCopy);

    // Verify the timing info of the modified isxb file
    SECTION("isxb timing info")
    {
        isx::alignStartTimes(
            isxdFilename,
            {isxbFilenameCopy}
        );

        const auto originalMovie = isx::readMovie(isxbFilename);
        const auto modifiedMovie = isx::readMovie(isxbFilenameCopy);

        const auto originalTimingInfo = originalMovie->getTimingInfo();
        // The recomputed start time is 541 ms greater than the start time in the original isxb file
        const isx::TimingInfo expTimingInfo(
            originalTimingInfo.getStart() + isx::DurationInSeconds::fromMilliseconds(541),
            originalTimingInfo.getStep(),
            originalTimingInfo.getNumTimes(),
            originalTimingInfo.getDroppedFrames(),
            originalTimingInfo.getCropped(),
            originalTimingInfo.getBlankFrames()
        );
        REQUIRE(modifiedMovie->getTimingInfo() == expTimingInfo);
    }

    isx::CoreShutdown();

    std::remove(isxbFilenameCopy.c_str());
}

TEST_CASE("AlignStartTimes-EarlyDroppedFrames", "[core]") 
{
    isx::CoreInitialize();

    const std::string testDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-synchronized/manual";
    const std::string gpioFilename = testDir + "/2022-06-08-23-53-41_video.gpio";
    const std::string isxdFilename = testDir + "/synthetic.isxd";
    const std::string recordingUUID = "AC-00111111-l4R4GRt28o-1654732421577";

    SECTION("First frame of isxd movie is dropped")
    {
        // Create synthetic isxd movie with early dropped frames
        const uint64_t gpioFirstTsc = 459472480691;
        const uint64_t isxdFirstValidTsc = gpioFirstTsc + uint64_t(500 * 1e3); // 500 ms greater than gpioFirstTsc
        writeIsxdMovieWithEarlyDroppedFrames(
            isxdFilename,
            isx::TimingInfo(isx::Time(), isx::DurationInSeconds(1, 10), 19, {0}),
            isx::SpacingInfo(isx::SizeInPixels_t(4, 3)),
            isxdFirstValidTsc,
            recordingUUID
        );

        const uint64_t gpioStartTimestamp = 1654732421836;

        // First frame is dropped
        // TSC value of first frame is estimated to be: isxdFirstValidTsc - stepSize = isxdFirstValidTsc - 100 ms
        // which means start time of isxd movie is estimated to be 400 ms after the gpio start time
        const uint64_t expIsxdStartTimestamp = gpioStartTimestamp + 400;

        isx::alignStartTimes(
            gpioFilename,
            {isxdFilename}
        );

        const auto movie = isx::readMovie(isxdFilename);
        const uint64_t isxdStartTimestamp = uint64_t(movie->getTimingInfo().getStart().getSecsSinceEpoch().getNum());
        REQUIRE(isxdStartTimestamp == expIsxdStartTimestamp);
    }

    SECTION("First three frames of isxd movie are dropped")
    {
        // Create synthetic isxd movie with early dropped frames
        const uint64_t gpioFirstTsc = 459472480691;
        const uint64_t isxdFirstValidTsc = gpioFirstTsc + uint64_t(500 * 1e3); // 500 ms greater than gpioFirstTsc
        writeIsxdMovieWithEarlyDroppedFrames(
            isxdFilename,
            isx::TimingInfo(isx::Time(), isx::DurationInSeconds(1, 10), 19, {0, 1, 2}),
            isx::SpacingInfo(isx::SizeInPixels_t(4, 3)),
            isxdFirstValidTsc,
            recordingUUID
        );

        const uint64_t gpioStartTimestamp = 1654732421836;

        // First three frames are dropped
        // TSC value of first frame is estimated to be: isxdFirstValidTsc - stepSize = isxdFirstValidTsc - 300 ms
        // which means start time of isxd movie is estimated to be 200 ms after the gpio start time
        const uint64_t expIsxdStartTimestamp = gpioStartTimestamp + 200;

        isx::alignStartTimes(
            gpioFilename,
            {isxdFilename}
        );

        const auto movie = isx::readMovie(isxdFilename);
        const uint64_t isxdStartTimestamp = uint64_t(movie->getTimingInfo().getStart().getSecsSinceEpoch().getNum());
        REQUIRE(isxdStartTimestamp == expIsxdStartTimestamp);
    }

    isx::CoreShutdown();

    std::remove(isxdFilename.c_str());
}

TEST_CASE("AlignStartTimes-Series", "[core]") 
{
    isx::CoreInitialize();

    const std::string testDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-synchronized/scheduled";
    const std::string refFilename = testDir + "/2022-06-09-12-33-38_video_sched_0.gpio";
    const std::vector<std::string> alignFilenames = {
        testDir + "/2022-06-09-12-33-38_video_sched_0-camera-1.isxb",
        testDir + "/2022-06-09-12-33-38_video_sched_1-camera-1.isxb",
        testDir + "/2022-06-09-12-33-38_video_sched_2-camera-1.isxb"
    };

    // Copy test isxb and isxd files to modify.
    const std::vector<std::string> alignCopyFilenames = {
        testDir + "/2022-06-09-12-33-38_video_sched_0-camera-1-mod.isxb",
        testDir + "/2022-06-09-12-33-38_video_sched_1-camera-1-mod.isxb",
        testDir + "/2022-06-09-12-33-38_video_sched_2-camera-1-mod.isxb"
    };

    for (size_t i = 0; i < 3; i++)
    {
        isx::copyFile(alignFilenames[i], alignCopyFilenames[i]);
    }

    // Verify the timing info of the modified isxb files
    SECTION("isxb timing info")
    {
        isx::alignStartTimes(
            refFilename,
            alignCopyFilenames
        );

        const std::vector<int64_t> expTsDiffs = {524, 10428, 20433};
        for (size_t i = 0; i < 3; i++)
        {
            const auto originalMovie = isx::readMovie(alignFilenames[i]);
            const auto modifiedMovie = isx::readMovie(alignCopyFilenames[i]);

            const auto originalTimingInfo = originalMovie->getTimingInfo();
            const isx::TimingInfo expTimingInfo(
                originalTimingInfo.getStart() + isx::DurationInSeconds::fromMilliseconds(expTsDiffs[i]),
                originalTimingInfo.getStep(),
                originalTimingInfo.getNumTimes(),
                originalTimingInfo.getDroppedFrames(),
                originalTimingInfo.getCropped(),
                originalTimingInfo.getBlankFrames()
            );
            REQUIRE(modifiedMovie->getTimingInfo() == expTimingInfo);
        }
    }

    isx::CoreShutdown();

    for (size_t i = 0; i < 3; i++)
    {
        std::remove(alignCopyFilenames[i].c_str());
    }
}

TEST_CASE("ExportAlignedTimestamps-Invalid", "[core]") 
{
    isx::CoreInitialize();

    isx::ExportAlignedTimestampsParams inputParams;
    auto outputParams = std::make_shared<isx::ExportAlignedTimestampsOutputParams>();

    std::string processedFilename;

    SECTION("invalid data type for timing reference")
    {
        inputParams.m_refSeriesFilenames = {g_resources["unitTestDataPath"] + "/imu/2020-02-13-18-43-21_video.imu"};
        inputParams.m_alignSeriesFilenames = {{g_resources["unitTestDataPath"] + "/nVision/20220412-200447-camera-100.isxb"}};
        inputParams.m_refSeriesName = "imu";
        inputParams.m_alignSeriesNames = {"isxb"};

        ISX_REQUIRE_EXCEPTION(
            isx::exportAlignedTimestamps(inputParams, outputParams, [](float) {return false; }),
            isx::ExceptionUserInput,
            "Unsupported data type - only gpio files, isxd movies, and isxb movies are supported as a timing reference."
        );
    }

    SECTION("invalid data type for files to align to timing reference")
    {
        inputParams.m_refSeriesFilenames = {g_resources["unitTestDataPath"] + "/gpio/2020-05-20-10-33-22_video.gpio"};
        inputParams.m_alignSeriesFilenames = {{g_resources["unitTestDataPath"] + "/cell_metrics/cell_metrics_movie-PCA-ICA.isxd"}};
        inputParams.m_refSeriesName = "gpio";
        inputParams.m_alignSeriesNames = {"isxd"};

        ISX_REQUIRE_EXCEPTION(
            isx::exportAlignedTimestamps(inputParams, outputParams, [](float) {return false; }),
            isx::ExceptionUserInput,
            "Unsupported data type - only isxd movies and isxb movies are supported as input files to align to a timing reference."
        );
    }

    SECTION("no frame timestamps in movie")
    {
        const std::string testDataDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-synchronized/manual";
        inputParams.m_refSeriesFilenames = {testDataDir + "/2022-06-08-23-53-41_video.gpio"};
        const std::string alignFilename = testDataDir + "/2022-06-08-23-53-41_video.isxd";
        processedFilename = testDataDir + "/2022-06-08-23-53-41_video-PP.isxd";
        inputParams.m_alignSeriesFilenames = {{processedFilename}};
        inputParams.m_refSeriesName = "gpio";
        inputParams.m_alignSeriesNames = {"isxd"};

        // preprocess movie to strip the movie of frame timestamps
        const auto movie = isx::readMovie(alignFilename);
        const size_t temporalDsFactor = 2; // temporally downsample by 2x to strip movie of timestamps
        const size_t spatialDsFactor = 1;
        const isx::PreprocessMovieParams preprocessParams(
                movie,
                processedFilename,
                temporalDsFactor,
                spatialDsFactor,
                movie->getSpacingInfo().getFovInPixels(),
                true);
        auto preprocessOutputParams = std::make_shared<isx::PreprocessMovieOutputParams>();
        isx::preprocessMovie(preprocessParams, preprocessOutputParams, [](float){return false;});

        ISX_REQUIRE_EXCEPTION(
            isx::exportAlignedTimestamps(inputParams, outputParams, [](float) {return false; }),
            isx::ExceptionUserInput,
            "No frame timestamps stored in movie file to export."
        );
    }

    SECTION("timing reference with no recording UUID metadata")
    {
        inputParams.m_refSeriesFilenames = {g_resources["unitTestDataPath"] + "/gpio/2020-05-20-10-33-22_video.gpio"};
        inputParams.m_alignSeriesFilenames = {{g_resources["unitTestDataPath"] + "/cnmfe-cpp/movie_128x128x1000.isxd"}};
        inputParams.m_refSeriesName = "gpio";
        inputParams.m_alignSeriesNames = {"isxd"};

        ISX_REQUIRE_EXCEPTION(
            isx::exportAlignedTimestamps(inputParams, outputParams, [](float) {return false; }),
            isx::ExceptionUserInput,
            "Cannot determine if files are paired and synchronized - no recording UUID in timing reference file metadata."
        );
    }

    SECTION("paired and unsynchronized")
    {
        const std::string testDataDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-unsynchronized";
        inputParams.m_refSeriesFilenames = {testDataDir + "/2022-06-08-23-57-41_video.gpio"};
        inputParams.m_alignSeriesFilenames = {{testDataDir + "/2022-06-08-23-57-43-camera-1.isxb"}};
        inputParams.m_refSeriesName = "gpio";
        inputParams.m_alignSeriesNames = {"isxb"};

        ISX_REQUIRE_EXCEPTION(
            isx::exportAlignedTimestamps(inputParams, outputParams, [](float) {return false; }),
            isx::ExceptionUserInput,
            "Files are not paired and synchronized - recording UUID of align file (AC-00111111-l4R4GRt28o-1654732663355) does not match recording UUID of timing reference file (AC-00111111-l4R4GRt28o-1654732661796)."
        );
    }

    SECTION("standalone")
    {
        const std::string testDataDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/";
        inputParams.m_refSeriesFilenames = {testDataDir + "/standalone-miniscope/2022-06-08-23-58-43_video.gpio"};
        inputParams.m_alignSeriesFilenames = {{testDataDir + "/standalone-behavior/2022-06-08-23-58-51-camera-1.isxb"}};
        inputParams.m_refSeriesName = "gpio";
        inputParams.m_alignSeriesNames = {"isxb"};

        ISX_REQUIRE_EXCEPTION(
            isx::exportAlignedTimestamps(inputParams, outputParams, [](float) {return false; }),
            isx::ExceptionUserInput,
            "Files are not paired and synchronized - recording UUID of align file (GA-21807233-0000000000-1654732731777) does not match recording UUID of timing reference file (AC-00111111-0000000000-1654732723918)."
        );
    }

    SECTION("mismatching names")
    {
        const std::string testDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-synchronized/manual";
        const std::string refFilename = testDir + "/2022-06-08-23-53-41_video.gpio";
        const std::vector<std::vector<std::string>> alignFilenames = {
            {testDir + "/2022-06-08-23-53-41_video-camera-1.isxb"},
            {testDir + "/2022-06-08-23-53-41_video.isxd"}
        };

        const std::string outputFilename = testDir + "/exportAlignedTimestamps.csv";

        inputParams.m_refSeriesFilenames = {refFilename};
        inputParams.m_alignSeriesFilenames = alignFilenames;
        inputParams.m_refSeriesName = "Gpio Ref";
        inputParams.m_alignSeriesNames = {"Only One Align"};
        inputParams.m_outputFilename = outputFilename;

        ISX_REQUIRE_EXCEPTION(
            isx::exportAlignedTimestamps(inputParams, outputParams, [](float) {return false; }),
            isx::ExceptionUserInput,
            "Number of align file paths does not match number of align names."
        );
    }

    if (!processedFilename.empty())
    {
        std::remove(processedFilename.c_str());
    }

    isx::CoreShutdown();
}

TEST_CASE("ExportAlignedTimestamps-GpioRef", "[core]") 
{
    isx::CoreInitialize();

    const std::string testDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-synchronized/manual";
    const std::string refFilename = testDir + "/2022-06-08-23-53-41_video.gpio";
    const std::vector<std::vector<std::string>> alignFilenames = {
        {testDir + "/2022-06-08-23-53-41_video-camera-1.isxb"},
        {testDir + "/2022-06-08-23-53-41_video.isxd"}
    };

    const std::string outputFilename = testDir + "/exportAlignedTimestamps.csv";

    isx::ExportAlignedTimestampsParams inputParams(
        {refFilename},
        alignFilenames,
        "Gpio Ref",
        {"Isxb Align", "Isxd Align"},
        outputFilename,
        isx::WriteTimeRelativeTo::FIRST_DATA_ITEM
    );
    auto outputParams = std::make_shared<isx::ExportAlignedTimestampsOutputParams>();

    SECTION("check first and last line of output csv")
    {
        REQUIRE(isx::exportAlignedTimestamps(inputParams, outputParams, [](float) {return false; }) == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(outputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "Gpio Ref Timestamp (s),Gpio Ref Channel,Isxb Align Timestamp (s),Isxd Align Timestamp (s)";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "0.000000,Digital GPI 0,0.282199,0.052248";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = "1.951800,BNC Trigger Input,,";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);    
    }

    isx::CoreShutdown();

    std::remove(outputFilename.c_str());
}

TEST_CASE("ExportAlignedTimestamps-IsxdRef", "[core]") 
{
    isx::CoreInitialize();

    const std::string testDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-synchronized/manual";
    const std::string refFilename = testDir + "/2022-06-08-23-53-41_video.isxd";
    const std::vector<std::vector<std::string>> alignFilenames = {
        {testDir + "/2022-06-08-23-53-41_video-camera-1.isxb"}
    };

    const std::string outputFilename = testDir + "/exportAlignedTimestamps.csv";

    isx::ExportAlignedTimestampsParams inputParams(
        {refFilename},
        alignFilenames,
        "Isxd Ref",
        {"Isxb Align"},
        outputFilename,
        isx::WriteTimeRelativeTo::FIRST_DATA_ITEM
    );
    auto outputParams = std::make_shared<isx::ExportAlignedTimestampsOutputParams>();

    SECTION("check first and last line of output csv")
    {
        REQUIRE(isx::exportAlignedTimestamps(inputParams, outputParams, [](float) {return false; }) == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(outputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "Isxd Ref Timestamp (s),Isxb Align Timestamp (s)";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "0.000000,0.229951";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = ",1.965928";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);    
    }

    isx::CoreShutdown();

    std::remove(outputFilename.c_str());
}

TEST_CASE("ExportAlignedTimestamps-Series", "[core]") 
{
    isx::CoreInitialize();

    const std::string testDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-synchronized/scheduled";
    const std::string refFilename = testDir + "/2022-06-09-12-33-38_video_sched_0.gpio";
    const std::vector<std::vector<std::string>> alignFilenames = {
        {testDir + "/2022-06-09-12-33-38_video_sched_0-camera-1.isxb",
        testDir + "/2022-06-09-12-33-38_video_sched_1-camera-1.isxb",
        testDir + "/2022-06-09-12-33-38_video_sched_2-camera-1.isxb"}
    };

    const std::string outputFilename = testDir + "/exportAlignedTimestamps.csv";

    isx::ExportAlignedTimestampsParams inputParams(
        {refFilename},
        alignFilenames,
        "Ref",
        {"Align"},
        outputFilename,
        isx::WriteTimeRelativeTo::FIRST_DATA_ITEM
    );
    auto outputParams = std::make_shared<isx::ExportAlignedTimestampsOutputParams>();

    SECTION("check first and last line of output csv")
    {
        REQUIRE(isx::exportAlignedTimestamps(inputParams, outputParams, [](float) {return false; }) == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(outputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "Ref Timestamp (s),Ref Channel,Align Timestamp (s)";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "0.000000,Digital GPI 0,0.265064";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = "33.959800,BNC Trigger Input,";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);    
    }

    std::remove(outputFilename.c_str());
}

TEST_CASE("ExportAlignedTimestamps-Unix", "[core]") 
{
    isx::CoreInitialize();

    const std::string testDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-synchronized/manual";
    const std::string refFilename = testDir + "/2022-06-08-23-53-41_video.isxd";
    const std::vector<std::vector<std::string>> alignFilenames = {
        {testDir + "/2022-06-08-23-53-41_video-camera-1.isxb"}
    };

    const std::string outputFilename = testDir + "/exportAlignedTimestamps.csv";

    isx::ExportAlignedTimestampsParams inputParams(
        {refFilename},
        alignFilenames,
        "Isxd Ref",
        {"Isxb Align"},
        outputFilename,
        isx::WriteTimeRelativeTo::UNIX_EPOCH
    );
    auto outputParams = std::make_shared<isx::ExportAlignedTimestampsOutputParams>();

    SECTION("check first and last line of output csv")
    {
        REQUIRE(isx::exportAlignedTimestamps(inputParams, outputParams, [](float) {return false; }) == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(outputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "Isxd Ref Timestamp (s),Isxb Align Timestamp (s)";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "1654732421.888000,1654732422.117951";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = ",1654732423.853928";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);    
    }

    isx::CoreShutdown();

    std::remove(outputFilename.c_str());
}

TEST_CASE("ExportAlignedTimestamps-Tsc", "[core]") 
{
    isx::CoreInitialize();

    const std::string testDir = g_resources["unitTestDataPath"] + "/nVision/recordingUUID/paired-synchronized/manual";
    const std::string refFilename = testDir + "/2022-06-08-23-53-41_video.isxd";
    const std::vector<std::vector<std::string>> alignFilenames = {
        {testDir + "/2022-06-08-23-53-41_video-camera-1.isxb"}
    };

    const std::string outputFilename = testDir + "/exportAlignedTimestamps.csv";

    isx::ExportAlignedTimestampsParams inputParams(
        {refFilename},
        alignFilenames,
        "Isxd Ref",
        {"Isxb Align"},
        outputFilename,
        isx::WriteTimeRelativeTo::TSC
    );
    auto outputParams = std::make_shared<isx::ExportAlignedTimestampsOutputParams>();

    SECTION("check first and last line of output csv")
    {
        REQUIRE(isx::exportAlignedTimestamps(inputParams, outputParams, [](float) {return false; }) == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(outputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "Isxd Ref Timestamp (s),Isxb Align Timestamp (s)";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "459472532939,459472762890";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = ",459474498867";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);    
    }

    isx::CoreShutdown();

    std::remove(outputFilename.c_str());
}
#endif