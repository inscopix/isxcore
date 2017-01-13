#include "isxJsonUtils.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxVariant.h"

#include <stdio.h>

TEST_CASE("JsonUtilsTest", "[core-internal]")
{

    SECTION("Convert timing info to json")
    {
        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::isize_t numTimes = 20;
        std::vector<isx::isize_t> droppedFrames{4, 7};
        isx::TimingInfo timingInfo(start, step, numTimes, droppedFrames);

        isx::json jsonObject = isx::convertTimingInfoToJson(timingInfo);

        isx::isize_t jsonNumTimes = jsonObject["numTimes"];
        isx::DurationInSeconds jsonStep = isx::convertJsonToRatio(jsonObject["period"]);
        isx::Time jsonStart = isx::convertJsonToTime(jsonObject["start"]);
        std::vector<isx::isize_t> jsonDroppedFrames = jsonObject["dropped"].get<std::vector<isx::isize_t>>();

        REQUIRE(jsonNumTimes == numTimes);
        REQUIRE(jsonStep == step);
        REQUIRE(jsonStart == start);
        REQUIRE(jsonDroppedFrames == droppedFrames);
    }

}
