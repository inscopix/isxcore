#include "isxTime.h"
#include "isxCore.h"
#include "isxTest.h"
#include "catch.hpp"

TEST_CASE("TimeTest", "[core]") {

    SECTION("empty constructor")
    {
        isx::Time actual;
        isx::Time expected(1970, 1, 1, 0, 0, 0, 0);
        REQUIRE(actual == expected);
    }

    SECTION("constructor using seconds since epoch")
    {
        isx::Time actual(isx::DurationInSeconds(1952, 1000));
        isx::Time expected(1970, 1, 1, 0, 0, 1, isx::DurationInSeconds(952, 1000));
        REQUIRE(actual == expected);
    }

    SECTION("add zero seconds to a time")
    {
        isx::Time expected(1970, 1, 1, 0, 0, 0, 0);
        isx::Time actual = expected + 0;
        REQUIRE(actual == expected);
    }

    SECTION("add integral duration to a time")
    {
        isx::Time time(1970, 1, 1, 0, 0, 0, 0);
        isx::Time expected(1970, 1, 1, 0, 0, 7, 0);
        isx::Time actual = time + 7;
        REQUIRE(actual == expected);
    }

    SECTION("add rational duration to a time")
    {
        isx::Time time;
        isx::DurationInSeconds offset(531, 1000);
        isx::Time expected(1970, 1, 1, 0, 0, 0, offset);
        isx::Time actual = time + offset;
        REQUIRE(actual == expected);
    }

    SECTION("accumulate rational durations")
    {
        isx::DurationInSeconds stepTime(1, 30);
        isx::isize_t numTimes = 32;
        isx::Time time;
        isx::Time expected = time + (stepTime * numTimes);

        isx::Time actual = time;
        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            actual += stepTime;
        }

        REQUIRE(actual == expected);
    }

    SECTION("subtract rational duration from a time")
    {
        isx::Time time(1970, 1, 1, 0, 0, 1);
        isx::Time expected(1970, 1, 1, 0, 0, 0, isx::DurationInSeconds(913, 1000));

        isx::Time actual = time - isx::DurationInSeconds(87, 1000);

        REQUIRE(actual == expected);
    }

    SECTION("decrement rational durations")
    {
        isx::DurationInSeconds stepTime(1, 30);
        isx::isize_t numTimes = 32;
        isx::Time time(1970, 1, 1, 0, 0, 2);
        isx::Time expected = time - (stepTime * numTimes);

        isx::Time actual = time;
        for (isx::isize_t t = 0; t < numTimes; ++t)
        {
            actual -= stepTime;
        }

        REQUIRE(actual == expected);
    }

    SECTION("find duration between two times")
    {
        isx::Time time1(1970, 1, 1, 0, 0, 5, isx::DurationInSeconds(53, 1000));
        isx::Time time2(1970, 1, 1, 0, 0, 8, isx::DurationInSeconds(294, 1000));

        isx::DurationInSeconds expected(3241, 1000);
        isx::DurationInSeconds actual = time2 - time1;

        REQUIRE(actual == expected);
    }

    SECTION("copy constructor")
    {
        isx::Time time;
        isx::Time otherTime(time);
        REQUIRE(time == otherTime);
    }

    SECTION("copy assignment")
    {
        isx::Time time;
        isx::Time otherTime = time;
        REQUIRE(time == otherTime);
    }

    SECTION("equals operator")
    {
        isx::Time time(1970, 1, 1, 0, 0, 0, 0);
        isx::Time otherTime(1970, 1, 1, 0, 0, 0, 0);
        REQUIRE(time == otherTime);
    }

    SECTION("String conversion")
    {
        isx::Time time(1970, 1, 1, 0, 0, 0, 0);
        std::string expected = "1970/01/01-00:00:00.000";
        std::string actual = time.toString();
        REQUIRE(actual == expected);
    }

    SECTION("get seconds since epoch")
    {
        isx::Time time(1970, 1, 1, 0, 2, 10, isx::DurationInSeconds(691, 1000));
        isx::DurationInSeconds expected(130691, 1000);
        isx::DurationInSeconds actual = time.getSecsSinceEpoch();

        REQUIRE(actual == expected);
    }

    SECTION("get UTC offset")
    {
        isx::Time time(1970, 1, 1, 0, 0, 0, 0, -8);
        REQUIRE(time.getUtcOffset() == -8);
    }

    SECTION("floorToDenomOf")
    {
        isx::Time t1(isx::DurationInSeconds(3, 7), 3);
        isx::Ratio r2(20, 23);
        isx::Time tf = t1.floorToDenomOf(r2);
        isx::Time expected(isx::DurationInSeconds(9, 23), 3);
        REQUIRE(tf == expected);
        REQUIRE(tf.getUtcOffset() == expected.getUtcOffset());
    }

}

TEST_CASE("TimeTest-UtcOffset", "[core]") {

    SECTION("equality")
    {
        // two times with the same epoch but different utc offsets
        const isx::Time withUtc(2022, 4, 1, 7, 27, 6, isx::DurationInSeconds(332, 1000), 7 * 60 * 60);
        const isx::Time withoutUtc(2022, 4, 1, 7, 27, 6, isx::DurationInSeconds(332, 1000));
        REQUIRE(withUtc == withoutUtc);
    }

    SECTION("inequality")
    {
        // two times with different epoch but the same local time
        const isx::Time withUtc(2022, 4, 1, 7, 27, 6, isx::DurationInSeconds(332, 1000), 7 * 60 * 60);
        const isx::Time withoutUtc(2022, 4, 1, 14, 27, 6, isx::DurationInSeconds(332, 1000));
        REQUIRE(withUtc != withoutUtc);
    }

    SECTION("less than")
    {
        // two times with and without a utc offset
        const isx::Time withUtc(2022, 4, 1, 7, 27, 6, isx::DurationInSeconds(332, 1000), 7 * 60 * 60);
        const isx::Time withoutUtc(2022, 4, 1, 14, 24, 6, isx::DurationInSeconds(332, 1000));
        REQUIRE(withUtc < withoutUtc);
    }

    SECTION("less than or equal")
    {
        // two times with and without a utc offset
        const isx::Time withUtc(2022, 4, 1, 7, 27, 6, isx::DurationInSeconds(332, 1000), 7 * 60 * 60);
        const isx::Time withoutUtc(2022, 4, 1, 14, 24, 6, isx::DurationInSeconds(332, 1000));
        REQUIRE(withUtc <= withoutUtc);
    }

    SECTION("greater than")
    {
        // two times with and without a utc offset
        const isx::Time withUtc(2022, 4, 1, 7, 30, 6, isx::DurationInSeconds(332, 1000), 7 * 60 * 60);
        const isx::Time withoutUtc(2022, 4, 1, 14, 27, 6, isx::DurationInSeconds(332, 1000));
        REQUIRE(withoutUtc > withUtc);
    }

    SECTION("greater than or equal")
    {
        // two times with and without a utc offset
        const isx::Time withUtc(2022, 4, 1, 7, 30, 6, isx::DurationInSeconds(332, 1000), 7 * 60 * 60);
        const isx::Time withoutUtc(2022, 4, 1, 14, 27, 6, isx::DurationInSeconds(332, 1000));
        REQUIRE(withoutUtc >= withUtc);
    }

    SECTION("addition")
    {
        // add 1 hour to a time with a utc offset
        isx::Time start(2022, 4, 1, 7, 27, 6, isx::DurationInSeconds(332, 1000), 7 * 60 * 60);
        const isx::DurationInSeconds duration(60 * 60);

        const isx::Time end(2022, 4, 1, 8, 27, 6, isx::DurationInSeconds(332, 1000));
        REQUIRE((start + duration) == end);

        start += duration;
        REQUIRE(start == end);
    }

    SECTION("subtraction")
    {
        // subtract 1 hour to a time with a utc offset
        isx::Time start(2022, 4, 1, 7, 27, 6, isx::DurationInSeconds(332, 1000), 7 * 60 * 60);
        const isx::DurationInSeconds duration(60 * 60);

        const isx::Time end(2022, 4, 1, 6, 27, 6, isx::DurationInSeconds(332, 1000));
        REQUIRE((start - duration) == end);

        start -= duration;
        REQUIRE(start == end);
    }

    SECTION("subtract equal times")
    {
        // two equal times with and without a utc offset
        const isx::Time withUtc(2022, 4, 1, 7, 27, 6, isx::DurationInSeconds(332, 1000), 7 * 60 * 60);
        const isx::Time withoutUtc(2022, 4, 1, 7, 27, 6, isx::DurationInSeconds(332, 1000));
        const isx::DurationInSeconds expDiff(0);

        REQUIRE((withUtc - withoutUtc) == expDiff);
    }

    SECTION("subtract unequal times")
    {
        // two times one hour apart with and without a utc offset
        const isx::Time withUtc(2022, 4, 1, 7, 27, 6, isx::DurationInSeconds(332, 1000), 7 * 60 * 60);
        const isx::Time withoutUtc(2022, 4, 1, 6, 27, 6, isx::DurationInSeconds(332, 1000));
        const isx::DurationInSeconds expDiff(60 * 60);

        REQUIRE((withUtc - withoutUtc) == expDiff);
    }

    SECTION("subtract equal times with different utc offsets")
    {
        // two equal times with different utc offsets
        const isx::Time t1(2022, 4, 1, 7, 27, 6, isx::DurationInSeconds(332, 1000), 7 * 60 * 60); // 7 hour utc offset
        const isx::Time t2(2022, 4, 1, 7, 27, 6, isx::DurationInSeconds(332, 1000), - 3 * 60 * 60); // -3 hour utc offset
        const isx::DurationInSeconds expDiff(0);

        REQUIRE((t1 - t2) == expDiff);
    }

    SECTION("subtract unequal times with different utc offsets")
    {
        // two times one hour apart with different utc offsets
        const isx::Time t1(2022, 4, 1, 7, 27, 6, isx::DurationInSeconds(332, 1000), 7 * 60 * 60); // 7 hour utc offset
        const isx::Time t2(2022, 4, 1, 6, 27, 6, isx::DurationInSeconds(332, 1000), - 3 * 60 * 60); // -3 hour utc offset
        const isx::DurationInSeconds expDiff(60 * 60);

        REQUIRE((t1 - t2) == expDiff);
    }
}

