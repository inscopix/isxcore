#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxNVisionMovieFile.h"
#include "isxTest.h"
#include "isxArmaUtils.h"

#include "catch.hpp"
// #include <vector>

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
        const size_t expSum = 9250722;
#else
        const size_t expSum = 9235061;
#endif
        size_t sum = 0;
        for (size_t i = 0; i < numFrames; i++)
        {
            const auto frame = file.readFrame(i);
            isx::ColumnUInt16_t frameCol;
            isx::copyFrameToColumn(frame, frameCol);
            sum += arma::sum(frameCol);
        }
        REQUIRE(sum == expSum);   
    }

    // SECTION("Acquisition info")
    // {
    //     // TODO: expose json session metadata from IDAS
    // }

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
        const size_t expSum = 27293446;
#else
        const size_t expSum = 27386866;
#endif
        size_t sumWithDropped = 0;
        size_t sumWithoutDropped = 0;
        for (size_t i = 0; i < numFrames; i++)
        {
            const auto frame = file.readFrame(i);
            isx::ColumnUInt16_t frameCol;
            isx::copyFrameToColumn(frame, frameCol);
            const size_t sum = arma::sum(frameCol); 
            sumWithDropped += sum;
            
            if (!ti.isDropped(i))
            {
                sumWithoutDropped += sum;
            }
        }
        REQUIRE(sumWithDropped == expSum);
        REQUIRE(sumWithoutDropped == expSum);
    }
    
    isx::CoreShutdown();   
}
