#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxNVisionMovieFile.h"
#include "isxTest.h"
#include "isxMovieFactory.h"
#include "isxPathUtils.h"
#include "catch.hpp"

TEST_CASE("NVisionMovieFile", "[core]") 
{
    const std::string testFileName = g_resources["unitTestDataPath"] + "/nVision/20220401-022845-KTM-RQEHB_10_secs.isxb";

    isx::CoreInitialize();

    isx::NVisionMovieFile file(testFileName);

    SECTION("Is valid")
    {
        REQUIRE(file.isValid());
    }

    SECTION("File name")
    {
        REQUIRE(file.getFileName() == testFileName);
    }

    SECTION("Timing info")
    {
        const isx::Time start(2022, 4, 1, 7, 27, 06, isx::DurationInSeconds(332, 1000));
        const isx::DurationInSeconds step(9968014, 299000000);
        const size_t numSamples = 300;
        const isx::TimingInfo expTimingInfo(start, step, numSamples);

        REQUIRE(file.getTimingInfo() == expTimingInfo);
        REQUIRE(file.getTimingInfosForSeries() == std::vector<isx::TimingInfo>{expTimingInfo});
    }

    SECTION("Spacing info")
    {
        const isx::SizeInPixels_t numPixels(1280, 720);
        const isx::SpacingInfo expSpacingInfo(numPixels);

        REQUIRE(file.getSpacingInfo() == expSpacingInfo);
    }

    SECTION("Data type")
    {
        REQUIRE(file.getDataType() == isx::DataType::U8);
    }

    SECTION("Frames")
    {
        // Verify movie data by computing sum of entire movie
        const size_t numFrames = file.getTimingInfo().getNumTimes();
        // Results of codec are slightly different between windows and linux/mac, but images look very similar
#if ISX_OS_WIN32
        const uint64_t expSum = 11687268770;
#else
        const uint64_t expSum = 11687253109;
#endif
        uint64_t sum = 0;
        for (size_t i = 0; i < numFrames; i++)
        {
            sum += computeFrameSum(file.readFrame(i));
        }
        REQUIRE(sum == expSum);   
    }

    SECTION("Frame timestamps")
    {
        const uint64_t startTsc = file.readFrameTimestamp(0);
        const uint64_t endTsc = file.readFrameTimestamp(file.getTimingInfo().getNumTimes() - 1);

        const uint64_t expStartTsc = 215738669569;
        const uint64_t expEndTsc = 215748637583;

        REQUIRE(startTsc == expStartTsc);
        REQUIRE(endTsc == expEndTsc);
    }

    SECTION("Frame out of bounds")
    {
        const size_t numFrames = file.getTimingInfo().getNumTimes();

        ISX_REQUIRE_EXCEPTION(
            file.readFrame(numFrames),
            isx::ExceptionUserInput, "Failed to read frame from file. Index is out of bounds.");
        

        ISX_REQUIRE_EXCEPTION(
            file.readFrameTimestamp(numFrames),
            isx::ExceptionUserInput, "Failed to read frame timestamp from file. Index is out of bounds.");
    }

    SECTION("Acquisition info")
    {
        using json = nlohmann::json;
        json extraProps = json::parse(file.getExtraProperties());

        const char * expExtraProps = "{\"cameraName\":\"camera-1\",\"hardwareInterface\":{\"cameras\":[{\"cameraAlias\":\"Top view camera\",\"cameraConnected\":true,\"cameraFrameFormat\":\"jpeg\","
        "\"cameraIndex\":0,\"cameraName\":\"camera-1\",\"cameraSerial\":\"11139104\",\"cameraType\":\"usb\",\"enabled\":false,\"focus\":{\"focus\":0},\"fps\":{\"fps\":30},\"recordFov\":{\"height"
        "\":1080,\"originx\":0,\"originy\":0,\"width\":1920},\"sensorFov\":{\"height\":1080,\"originx\":0,\"originy\":0,\"width\":1920},\"sensorParams\":{\"backlightCompensation\":{"
        "\"default\":1,\"max\":2,\"min\":0,\"step\":1,\"value\":1},\"brightness\":{\"default\":0,\"max\":64,\"min\":-64,\"step\":1,\"value\":0},\"contrast\":{\"default\":32,\"max\":64,\"min\":0,\"step\":1,\"value\":32},"
        "\"exposure\":{\"auto\":{\"default\":3,\"enabled\":true,\"max\":3,\"min\":0,\"step\":1,\"value\":3},\"manual\":{\"default\":156,\"enabled\":false,\"max\":5000,\"min\":1,\"step\":1,\"value\":156},\"type\":\"auto\"},"
        "\"gain\":{\"default\":0,\"max\":48,\"min\":0,\"step\":1,\"value\":0},\"gamma\":{\"default\":100,\"max\":500,\"min\":72,\"step\":1,\"value\":100},\"hue\":{\"default\":0,\"max\":40,\"min\":-40,\"step\":1,\"value\":0},"
        "\"saturation\":{\"default\":64,\"max\":128,\"min\":0,\"step\":1,\"value\":64},\"sharpness\":{\"default\":3,\"max\":6,\"min\":0,\"step\":1,\"value\":3},\"whiteBalance\":{\"auto\":{\"enabled\":true},\"manual\":{\"default"
        "\":4600,\"enabled\":false,\"max\":9300,\"min\":2800,\"step\":1,\"value\":4112},\"type\":\"auto\"}},\"systemAlias\":\"nVision box 1\",\"systemName\":\"nVision\",\"systemSerial\":\"BH-11139104\"},{\"cameraAlias\":"
        "\"Side view camera\",\"cameraConnected\":true,\"cameraFrameFormat\":\"jpeg\",\"cameraIndex\":1,\"cameraName\":\"camera-2\",\"cameraSerial\":\"11139105\",\"cameraType\":\"usb\",\"enabled\":false,\"focus\":{\"focus"
        "\":0},\"fps\":{\"fps\":20},\"recordFov\":{\"height\":1080,\"originx\":0,\"originy\":0,\"width\":1920},\"sensorFov\":{\"height\":1080,\"originx\":0,\"originy\":0,\"width\":1920},\"sensorParams\":{\"backlightCompensation"
        "\":{\"default\":1,\"max\":2,\"min\":0,\"step\":1,\"value\":1},\"brightness\":{\"default\":0,\"max\":64,\"min\":-64,\"step\":1,\"value\":0},\"contrast\":{\"default\":32,\"max\":64,\"min\":0,\"step\":1,\"value\":32},"
        "\"exposure\":{\"auto\":{\"default\":3,\"enabled\":true,\"max\":3,\"min\":0,\"step\":1,\"value\":3},\"manual\":{\"default\":156,\"enabled\":false,\"max\":5000,\"min\":1,\"step\":1,\"value\":156},\"type\":\"auto\"},"
        "\"gain\":{\"default\":0,\"max\":48,\"min\":0,\"step\":1,\"value\":0},\"gamma\":{\"default\":100,\"max\":500,\"min\":72,\"step\":1,\"value\":100},\"hue\":{\"default\":0,\"max\":40,\"min\":-40,\"step\":1,\"value\":0},"
        "\"saturation\":{\"default\":64,\"max\":128,\"min\":0,\"step\":1,\"value\":64},\"sharpness\":{\"default\":3,\"max\":6,\"min\":0,\"step\":1,\"value\":3},\"whiteBalance\":{\"auto\":{\"enabled\":true},\"manual\":{\"default"
        "\":4600,\"enabled\":false,\"max\":9300,\"min\":2800,\"step\":1,\"value\":4112},\"type\":\"auto\"}},\"systemAlias\":\"nVision box 1\",\"systemName\":\"nVision\",\"systemSerial\":\"BH-11139105\"}],\"fileFormat\":{"
        "\"container\":\"avi\",\"encoding\":\"jpeg\",\"metadata\":\"json\"}},\"processingInterface\":{\"camera-1\":{\"cameraAlias\":\"camera-1\",\"cameraFrameFormat\":\"jpeg\",\"cameraSerial\":\"KTM-RQEHB\",\"cameraType"
        "\":\"usb\",\"enabled\":true,\"focus\":{\"focus\":0},\"recordFov\":{\"height\":720,\"originx\":0,\"originy\":0,\"width\":1280},\"sensorParams\":{\"backlightCompensation\":1,\"brightness\":0,\"contrast\":32,\"exposure"
        "\":{\"type\":\"auto\"},\"fps\":30,\"gain\":0,\"gamma\":100,\"hue\":0,\"saturation\":64,\"sensorFov\":{\"height\":720,\"originx\":0,\"originy\":0,\"width\":1280},\"sharpness\":3,\"whiteBalance\":{\"type\":\"auto\"}}},"
        "\"camera-2\":{\"cameraAlias\":\"camera-2\",\"cameraFrameFormat\":\"jpeg\",\"cameraSerial\":\"11139105\",\"cameraType\":\"usb\",\"enabled\":true,\"focus\":{\"focus\":0},\"recordFov\":{\"height\":1080,\"originx\":0,"
        "\"originy\":0,\"width\":1920},\"sensorParams\":{\"backlightCompensation\":1,\"brightness\":128,\"contrast\":128,\"exposure\":{\"type\":\"manual\",\"value\":156},\"fps\":20,\"gain\":128,\"gamma\":100,\"hue\":0,"
        "\"saturation\":128,\"sensorFov\":{\"height\":1080,\"originx\":0,\"originy\":0,\"width\":1920},\"sharpness\":3,\"whiteBalance\":{\"type\":\"manual\",\"value\":4112}}},\"fileFormat\":{\"container\":\"mjpeg\","
        "\"encoding\":\"jpeg\",\"metadata\":\"json\"},\"manualTags\":{\"grooming\":\"G\",\"standing\":\"S\"},\"recordingSet\":{\"behaviour\":{\"gpio\":[\"2021-11-23-19-50-58_video_sched_0.gpio\"],\"video"
        "\":[\"2021-11-23-19-50-58_video_sched_0.isxb\",\"2021-11-23-19-50-58_video_sched_1.isxb\"]},\"calcium\":{\"gpio\":[\"2021-11-23-19-50-58_video_sched_0.gpio\"],\"imu\":[\"2021-11-23-19-50-58_video_sched_0.imu\"],"
        "\"video\":[\"2021-11-23-19-50-58_video_sched_0.isxd\",\"2021-11-23-19-50-58_video_sched_1.isxd\"]}},\"system\":{\"systemAlias\":\"nVision box 1\",\"systemName\":\"nVision\",\"systemSerial\":\"BH-11139104\","
        "\"systemVersion\":{\"backend\":\"1.0.0-9fd5720\",\"behaviorIdas\":\"1.0.0\",\"buildTimestamp\":\"Fri Dec 10 18:14:54 PST 2021\",\"frontend\":\"1.0.0-dd057c1\"}}},\"userInterface\":{\"animal\":{\"description\":\"\","
        "\"dob\":\"\",\"id\":\"\",\"sex\":\"m\",\"species\":\"\",\"weight\":0},\"customSession\":false,\"date\":\"2021-12-03T19:45:54.489Z\",\"description\":\"\",\"fileFormat\":{\"container\":\"avi\",\"encoding\":\"jpeg"
        "\",\"metadata\":\"json\"},\"manualTags\":{\"grooming\":\"G\",\"standing\":\"S\"},\"sessionName\":\"Session-20211203-114554\",\"sessionPath\":\"\",\"system\":{\"behavior\":{\"linked\":true,\"network\":{"
        "\"defaultGateway\":\"10.10.32.10\",\"dns1\":\"10.10.30.6\",\"dns2\":\"10.10.30.5\",\"ipAddress\":\"10.10.32.30\",\"macAddress\":\"00:04:4b:c6:de:1c\",\"networkMode\":\"auto\",\"subnetMask\":\"255.255.255.0\"},"
        "\"share\":{\"enabled\":true,\"sharePath\":\"\\\\\\\\10.10.32.30\\\\BH-11139104\"},\"software\":{\"backend\":\"1.0.0-9fd5720\",\"behaviorIdas\":\"1.0.0\",\"buildTimestamp\":\"Fri Dec 10 18:14:54 PST 2021\",\"frontend"
        "\":\"1.0.0-dd057c1\"},\"system\":{\"cameraSerial\":\"AC-11139105\",\"sessionName\":\"Session-20211203-114554\",\"sessionPath\":\"\",\"systemSerial\":\"AC-11139105\"}},\"calcium\":{\"linked\":true,\"network\":{"
        "\"defaultGateway\":\"10.10.32.10\",\"dns1\":\"10.10.30.6\",\"dns2\":\"10.10.30.5\",\"ipAddress\":\"10.10.32.30\",\"macAddress\":\"00:04:4b:c6:de:1c\",\"networkMode\":\"auto\",\"subnetMask\":\"255.255.255.0\"},"
        "\"share\":{\"enabled\":true,\"sharePath\":\"\\\\\\\\10.10.32.31\\\\AC-11139104\"},\"software\":{\"backend\":\"2.0.0-9fd5721\",\"buildTimestamp\":\"Fri Dec 10 18:14:54 PST 2021\",\"calciumIdas\":\"2.0.0\","
        "\"fpga\":{\"fpga-major\":\"0x16031050\",\"fpga-minor\":\"0x000b0073\",\"uBlaze-0\":\"0x0171b221\",\"uBlaze-1\":\"0x00030302\"},\"frontend\":\"2.0.0-dd057c2\"},\"system\":{\"miniscopeSerial\":\"AC-11139105\","
        "\"sessionName\":\"Session-20211203-114554\",\"sessionPath\":\"\",\"systemSerial\":\"AC-11139105\"}},\"hostPc\":{\"browser\":{\"name\":\"Google chrome\",\"version\":\"Version 96.0.4664.93 (Official Build) (64-bit)\"},"
        "\"network\":{\"defaultGateway\":\"10.10.32.10\",\"dns1\":\"10.10.30.6\",\"dns2\":\"10.10.30.5\",\"ipAddress\":\"10.10.32.30\",\"macAddress\":\"00:04:4b:c6:de:1d\",\"networkMode\":\"auto\",\"subnetMask\":\"255.255.255.0\"},"
        "\"operatingSystem\":{\"build\":\"192042.1348\",\"name\":\"Windows 10 Pro\",\"version\":\"20H2\"},\"share\":{\"enabled\":true,\"sharePath\":\"\\\\10.10.32.31\\test-pc\\\\share\"}}}}}";

        REQUIRE(extraProps.dump() == expExtraProps);
    }

    SECTION("Per-frame metadata")
    {
        using json = nlohmann::json;

        const auto actualFirstFrameMetadata = json::parse(file.readFrameMetadata(0));
        const json expectedFirstFrameMetadata({
            {"fc", 1},
            {"tsc", 215738669569}
        });
        REQUIRE(actualFirstFrameMetadata == expectedFirstFrameMetadata);

        const auto actualSecondFrameMetadata = json::parse(file.readFrameMetadata(1));
        const json expectedSecondFrameMetadata({
            {"fc", 2},
            {"tsc", 215738701589}
        });
        REQUIRE(actualSecondFrameMetadata == expectedSecondFrameMetadata);
    }

    isx::CoreShutdown();
}

TEST_CASE("NVisionMovieFile-Dropped", "[core]")
{
    const std::string testFileName = g_resources["unitTestDataPath"] + "/nVision/2022-04-18-21-48-13-camera-1_dropped.isxb";

    isx::CoreInitialize();

    isx::NVisionMovieFile file(testFileName);

    SECTION("Timing info")
    {
        const isx::Time start(2022, 4, 19, 4, 48, 13, isx::DurationInSeconds(459, 1000));
        const isx::DurationInSeconds step(37712004, 1131000000);
        const size_t numSamples = 1132;
        const std::vector<size_t> droppedFrames = {
            186, 188, 189, 190, 192, 193, 194, 196, 197, 198, 200, 201, 202, 204, 205,
            206, 208, 209, 210, 212, 213, 214, 216, 217, 218, 220, 221, 223, 224, 226,
            227, 228, 230, 231, 232, 234, 235, 237, 238, 239, 241, 242, 244, 245, 247,
            248, 249, 251, 252, 254, 255, 256, 258, 259, 260, 262, 263, 265, 266, 267,
            269, 270, 271, 273, 274, 276, 277, 278, 280, 281, 283, 284, 286, 287, 288,
            290, 291, 293, 294, 295, 297, 298, 300, 301, 303, 304, 305, 307, 308, 309,
            311, 313, 314, 315, 317, 318, 319, 321, 322, 324, 325, 326, 328, 329, 331,
            332, 334, 335, 336, 338, 339, 340, 342, 343, 345, 346, 347, 349, 350, 352,
            353, 354, 355, 357, 358, 359, 361, 362, 364, 365, 366, 368, 369, 370, 372,
            373, 375, 376, 378, 379, 380, 382, 383, 385, 386, 387, 389, 390, 392, 393,
            394, 396, 397, 398, 400, 401, 403, 404, 405, 407, 408, 410, 411, 412, 414,
            415, 417, 418, 419, 421, 422, 423, 425, 426, 428, 429, 430, 432, 433, 435,
            436, 438, 439, 440, 442, 443, 444, 446, 447, 449, 450, 451, 453, 454, 455,
            457, 458, 459, 461, 462, 464, 465, 466, 468, 470, 471, 472, 474, 475, 477,
            478, 479, 481, 482, 484, 485, 486, 488, 489, 491, 492, 494, 495, 496, 498,
            499, 500, 502, 503, 505, 506, 507, 509, 510, 512, 513, 514, 516, 517, 519,
            520, 521, 523, 524, 526, 527, 529, 530, 531, 533, 534, 535, 537, 538
        };
        const isx::TimingInfo expTimingInfo(start, step, numSamples, droppedFrames);

        REQUIRE(file.getTimingInfo() == expTimingInfo);
        REQUIRE(file.getTimingInfosForSeries() == std::vector<isx::TimingInfo>{expTimingInfo});
    }

    SECTION("Frames")
    {
        // Verify movie data by computing sum of entire movie
        // The sum should be the same whether you include dropped frames in the calculation or not
        // since a dropped frame is represented as a fully black frame (all zeroes)
        const isx::TimingInfo ti = file.getTimingInfo();
        const size_t numFrames = ti.getNumTimes();
        // Results of codec are slightly different between windows and linux/mac, but images look very similar
#if ISX_OS_WIN32
        const uint64_t expSum = 138002733830;
#else
        const uint64_t expSum = 138002696178;
#endif
        uint64_t sumWithDropped = 0;
        uint64_t sumWithoutDropped = 0;
        for (size_t i = 0; i < numFrames; i++)
        {
            const size_t sum = computeFrameSum(file.readFrame(i));
            sumWithDropped += sum;
            
            if (!ti.isDropped(i))
            {
                sumWithoutDropped += sum;
            }
        }
        REQUIRE(sumWithDropped == expSum);
        REQUIRE(sumWithoutDropped == expSum);
    }

    SECTION("Frame timestamps")
    {
        const uint64_t startTsc = file.readFrameTimestamp(0);
        const uint64_t endTsc = file.readFrameTimestamp(file.getTimingInfo().getNumTimes() - 1);

        const uint64_t expStartTsc = 38971101006;
        const uint64_t expEndTsc = 39008813010;

        REQUIRE(startTsc == expStartTsc);
        REQUIRE(endTsc == expEndTsc);

        for (const auto & i : file.getTimingInfo().getDroppedFrames())
        {
            REQUIRE(file.readFrameTimestamp(i) == 0);
        }
    }

    SECTION("Per-frame metadata")
    {
        using json = nlohmann::json;
        
        const auto actualDroppedFrameMetadata = json::parse(file.readFrameMetadata(186));
        const auto expectedDroppedFrameMetadata = json::parse("null");
        REQUIRE(actualDroppedFrameMetadata == expectedDroppedFrameMetadata);
    }
    
    isx::CoreShutdown();   
}

// Copy data from an existing isxb movie to a new one and verify the contents
// of the newly written isxb file.
TEST_CASE("NVisionMovieFile-Write", "[core]")
{
    const std::string inputFileName = g_resources["unitTestDataPath"] + "/nVision/20220412-200447-camera-100.isxb";
    const std::string outputFileName = g_resources["unitTestDataPath"] + "/test.isxb";

    isx::CoreInitialize();

    isx::NVisionMovieFile inputFile(inputFileName);

    {
        isx::NVisionMovieFile outputFile(outputFileName, inputFile.getTimingInfo(), inputFile.getSpacingInfo());
        for (size_t i = 0; i < inputFile.getTimingInfo().getNumTimes(); i++)
        {
            outputFile.writeFrame(inputFile.readFrame(i));
            outputFile.writeFrameMetadata(inputFile.readFrameMetadata(i));
        }
        outputFile.setExtraProperties(inputFile.getExtraProperties());
        outputFile.closeForWriting();
    }

    SECTION("Is valid")
    {
        isx::NVisionMovieFile outputFile(outputFileName);
        REQUIRE(outputFile.isValid());
    }
    
    SECTION("File name")
    {
        isx::NVisionMovieFile outputFile(outputFileName);
        REQUIRE(outputFile.getFileName() == outputFileName);
    }

    SECTION("Timing info")
    {
        isx::NVisionMovieFile outputFile(outputFileName);
        REQUIRE(outputFile.getTimingInfo() == inputFile.getTimingInfo());
    }

    SECTION("Spacing info")
    {
        isx::NVisionMovieFile outputFile(outputFileName);
        REQUIRE(outputFile.getSpacingInfo() == inputFile.getSpacingInfo());
    }

    SECTION("Frames")
    {
        isx::NVisionMovieFile outputFile(outputFileName);
        
        // Verify movie data by computing sum of both movies
        const size_t numFrames = inputFile.getTimingInfo().getNumTimes();
        uint64_t inputSum = 0;
        uint64_t outputSum = 0;
        for (size_t i = 0; i < numFrames; i++)
        {
            inputSum += computeFrameSum(inputFile.readFrame(i));
            outputSum += computeFrameSum(outputFile.readFrame(i));
        }

        REQUIRE(approxEqual(double(outputSum), double(inputSum), 1e-4));
    }

    SECTION("Frame timestamps")
    {
        isx::NVisionMovieFile outputFile(outputFileName);
        
        const size_t numFrames = inputFile.getTimingInfo().getNumTimes();
        for (size_t i = 0; i < numFrames; i++)
        {
            REQUIRE(outputFile.readFrameTimestamp(i) == inputFile.readFrameTimestamp(i));
        }
    }

    SECTION("Acquisition info")
    {
        using json = nlohmann::json;
        isx::NVisionMovieFile outputFile(outputFileName);
        const auto outputExtraProps = json::parse(outputFile.getExtraProperties());
        const auto inputExtraProps = json::parse(inputFile.getExtraProperties());
        REQUIRE(outputExtraProps == inputExtraProps);
    }

    isx::CoreShutdown();

    std::remove(outputFileName.c_str());
}

// Write isxb movie improperly
TEST_CASE("NVisionMovieFile-WriteInvalid", "[core]")
{
    const std::string outputFileName = g_resources["unitTestDataPath"] + "/test.isxb";

    isx::CoreInitialize();

    isx::TimingInfo timingInfo(isx::Time(), isx::DurationInSeconds::fromMilliseconds(10), 3);
    isx::SpacingInfo spacingInfo(isx::SizeInPixels_t(64, 10));
    
    SECTION("Number of per-frame metadata does not match number of samples")
    {
        using json = nlohmann::json;
        isx::NVisionMovieFile outputFile(outputFileName, timingInfo, spacingInfo);
        for (size_t i = 0; i < timingInfo.getNumTimes(); i++)
        {
            auto frame = std::make_shared<isx::VideoFrame>(
                spacingInfo,
                spacingInfo.getNumPixels().getWidth() * sizeof(uint8_t),
                1,
                isx::DataType::U8,
                timingInfo.convertIndexToStartTime(i),
                i);
            std::memset(frame->getPixels(), 0, frame->getImageSizeInBytes());
            outputFile.writeFrame(frame);

            if (i > 0)
            {
                outputFile.writeFrameMetadata(json::object({{"fc", i}, {"tsc", int(i * 1e5)}}).dump());
            }
        }
        outputFile.setExtraProperties("null");

        ISX_REQUIRE_EXCEPTION(outputFile.closeForWriting(), isx::ExceptionUserInput, "Number of frames in video does not match number of per-frame metadata.")
    }

    SECTION("Read frame in write mode")
    {
        isx::NVisionMovieFile outputFile(outputFileName, timingInfo, spacingInfo);

        ISX_REQUIRE_EXCEPTION(outputFile.readFrame(0), isx::ExceptionUserInput, "Cannot simultaneously read and write to nVision movies.")
    }

    SECTION("Write frame in read mode")
    {
        const std::string inputFileName = g_resources["unitTestDataPath"] + "/nVision/20220412-200447-camera-100.isxb";
        isx::NVisionMovieFile outputFile(inputFileName);

        auto frame = std::make_shared<isx::VideoFrame>(
            spacingInfo,
            spacingInfo.getNumPixels().getWidth() * sizeof(uint8_t),
            1,
            isx::DataType::U8,
            timingInfo.convertIndexToStartTime(0),
            0);
        std::memset(frame->getPixels(), 1, frame->getImageSizeInBytes());

        ISX_REQUIRE_EXCEPTION(outputFile.writeFrame(frame), isx::ExceptionUserInput, "Cannot simultaneously read and write to nVision movies.")
    }

    SECTION("Unsupported widths")
    {
        spacingInfo = isx::SpacingInfo(isx::SizeInPixels_t(97, 64));

        ISX_REQUIRE_EXCEPTION(isx::NVisionMovieFile(outputFileName, timingInfo, spacingInfo), isx::ExceptionUserInput, "Resolution (97 x 64) is unsupported for mjpeg encoder. Only resolutions with widths that are multiples of 64 are accepted.")
    }
    
    isx::CoreShutdown();

    std::remove(outputFileName.c_str());
}

// Write isxb movie with dropped frames and verify the contents of the file
TEST_CASE("NVisionMovieFile-WriteDropped", "[core]")
{
    const std::string outputFileName = g_resources["unitTestDataPath"] + "/test.isxb";

    isx::CoreInitialize();

    using json = nlohmann::json;
    isx::TimingInfo timingInfo(isx::Time(), isx::DurationInSeconds::fromMilliseconds(10), 5, {2, 3});
    isx::SpacingInfo spacingInfo(isx::SizeInPixels_t(64, 64));

    {
        isx::NVisionMovieFile outputFile(outputFileName, timingInfo, spacingInfo);
        for (size_t i = 0; i < timingInfo.getNumTimes(); i++)
        {
            if (timingInfo.isIndexValid(i))
            {
                auto frame = std::make_shared<isx::VideoFrame>(
                    spacingInfo,
                    spacingInfo.getNumPixels().getWidth() * sizeof(uint8_t),
                    1,
                    isx::DataType::U8,
                    timingInfo.convertIndexToStartTime(i),
                    i);
                std::memset(frame->getPixels(), 1, frame->getImageSizeInBytes());
                outputFile.writeFrame(frame);
                outputFile.writeFrameMetadata(json::object({{"fc", i}, {"tsc", int(i * 1e5)}}).dump());
            }
        }
        outputFile.setExtraProperties("null");
        outputFile.closeForWriting();
    }

    SECTION("Timing info")
    {
        isx::NVisionMovieFile outputFile(outputFileName);
        REQUIRE(outputFile.getTimingInfo() == timingInfo);
    }

    SECTION("Spacing info")
    {
        isx::NVisionMovieFile outputFile(outputFileName);
        REQUIRE(outputFile.getSpacingInfo() == spacingInfo);
    }

    SECTION("Frames")
    {
        isx::NVisionMovieFile outputFile(outputFileName);
        const size_t numFrames = timingInfo.getNumTimes();
        for (size_t i = 0; i < numFrames; i++)
        {
            const size_t sum = computeFrameSum(outputFile.readFrame(i));

            if (timingInfo.isDropped(i))
            {
                REQUIRE(sum == 0);
            }
            else
            {
                REQUIRE(sum == 64*64);
            }
        }
    }

    isx::CoreShutdown();

    std::remove(outputFileName.c_str());
}

// Write synthetic isxb movie with different resolutions to ensure
// they are supported by the mjpeg encoder.
TEST_CASE("NVisionMovieFile-WriteSupportedResolutions", "[core]")
{
    const std::string outputFileName = g_resources["unitTestDataPath"] + "/test.isxb";

    isx::CoreInitialize();

    using json = nlohmann::json;

    SECTION("Supported widths")
    {
        const size_t height = 97;
        for (size_t width = 64; width <= 4096; width+=64)
        {
            isx::TimingInfo timingInfo(isx::Time(), isx::DurationInSeconds::fromMilliseconds(10), 3);
            isx::SpacingInfo spacingInfo(isx::SizeInPixels_t(width, height));

            {
                isx::NVisionMovieFile outputFile(outputFileName, timingInfo, spacingInfo);
                for (size_t i = 0; i < timingInfo.getNumTimes(); i++)
                {
                    if (timingInfo.isIndexValid(i))
                    {
                        auto frame = std::make_shared<isx::VideoFrame>(
                            spacingInfo,
                            spacingInfo.getNumPixels().getWidth() * sizeof(uint8_t),
                            1,
                            isx::DataType::U8,
                            timingInfo.convertIndexToStartTime(i),
                            i);
                        std::memset(frame->getPixels(), 1, frame->getImageSizeInBytes());
                        outputFile.writeFrame(frame);
                        outputFile.writeFrameMetadata(json::object({{"fc", i}, {"tsc", int(i * 1e5)}}).dump());
                    }
                }
                outputFile.setExtraProperties("null");
                outputFile.closeForWriting();
            }

            isx::NVisionMovieFile outputFile(outputFileName);
            const size_t sum = computeFrameSum(outputFile.readFrame(0));
            REQUIRE(sum == width * height);
        }
    }

    isx::CoreShutdown();

    std::remove(outputFileName.c_str());
}

// Write synthetic isxb movie with no per-frame metadata
TEST_CASE("NVisionMovieFile-WriteNoFrameMetadata", "[core]")
{
    const std::string outputFileName = g_resources["unitTestDataPath"] + "/test.isxb";

    isx::CoreInitialize();

    isx::TimingInfo timingInfo(isx::Time(), isx::DurationInSeconds::fromMilliseconds(10), 3);
    isx::SpacingInfo spacingInfo(isx::SizeInPixels_t(64, 10));
    
    {
        isx::NVisionMovieFile outputFile(outputFileName, timingInfo, spacingInfo);
        for (size_t i = 0; i < timingInfo.getNumTimes(); i++)
        {
            auto frame = std::make_shared<isx::VideoFrame>(
                spacingInfo,
                spacingInfo.getNumPixels().getWidth() * sizeof(uint8_t),
                1,
                isx::DataType::U8,
                timingInfo.convertIndexToStartTime(i),
                i);
            std::memset(frame->getPixels(), 0, frame->getImageSizeInBytes());
            outputFile.writeFrame(frame);
        }
        outputFile.setExtraProperties("null");
        outputFile.closeForWriting();
    }

    SECTION("Timing info")
    {
        isx::NVisionMovieFile outputFile(outputFileName);
        REQUIRE(outputFile.getTimingInfo() == timingInfo);
    }

    SECTION("Spacing info")
    {
        isx::NVisionMovieFile outputFile(outputFileName);
        REQUIRE(outputFile.getSpacingInfo() == spacingInfo);
    }

    SECTION("Frame timestamps")
    {
        isx::NVisionMovieFile outputFile(outputFileName);
        REQUIRE(!outputFile.hasFrameTimestamps());
        for (size_t i = 0; i < timingInfo.getNumTimes(); i++)
        {
            REQUIRE(!outputFile.readFrameTimestamp(i));
        }
    }

    isx::CoreShutdown();

    std::remove(outputFileName.c_str());
}
