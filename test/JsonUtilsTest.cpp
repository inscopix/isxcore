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
        const isx::IndexRanges_t croppedFrames = {isx::IndexRange(9, 12)};
        std::vector<isx::isize_t> blankFrames{1, 17};
        isx::TimingInfo timingInfo(start, step, numTimes, droppedFrames);

        isx::json jsonObject = isx::convertTimingInfoToJson(timingInfo);
        isx::TimingInfo actualTimingInfo = isx::convertJsonToTimingInfo(jsonObject);

        REQUIRE(actualTimingInfo == timingInfo);
    }

}
