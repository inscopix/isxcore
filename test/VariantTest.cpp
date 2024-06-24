#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"
#include "isxVariant.h"

TEST_CASE("VariantTest", "[core]")
{

    SECTION("Empty constructor and setValue()")
    {
        isx::Time expectedTime(2017, 1, 10, 2, 30, 5, isx::DurationInSeconds(913, 1000));
        float expectedFloat = 1.01f;
        isx::Variant vt, vf;

        vt.setValue(expectedTime);
        vf.setValue(expectedFloat);

        REQUIRE(vt.getType() == isx::Variant::MetaType::ISXTIME);
        REQUIRE(vf.getType() == isx::Variant::MetaType::FLOAT);

        isx::Time storedTime = vt.value<isx::Time>();
        float storedFloat = vf.value<float>();

        REQUIRE(storedTime == expectedTime);
        REQUIRE(storedFloat == expectedFloat);
    }
    

    SECTION("Time Constructor")
    {
        isx::Time expected(2017, 1, 10, 2, 30, 5, isx::DurationInSeconds(913, 1000));
        isx::Variant v(expected);
        REQUIRE(v.getType() == isx::Variant::MetaType::ISXTIME);
        isx::Time stored = v.value<isx::Time>();
        REQUIRE(stored == expected);
    }

    SECTION("Type float")
    {
        float expected = 1.03f;
        isx::Variant v(expected);
        REQUIRE(v.getType() == isx::Variant::MetaType::FLOAT);
        float stored = v.value<float>();
        REQUIRE(stored == expected);
    }

    SECTION("Unsupported cases")
    {
        float fval = 1.03f;
        isx::Time tval(2017, 1, 10, 2, 30, 5, isx::DurationInSeconds(913, 1000));
        int i = 1035;
        isx::Variant vf(fval);
        isx::Variant vt(tval);

        ISX_REQUIRE_EXCEPTION(
            isx::Time t = vf.value<isx::Time>(),
            isx::ExceptionUserInput,
            "The type of the stored value cannot be converted to Time.");

        ISX_REQUIRE_EXCEPTION(
            float f = vt.value<float>(),
            isx::ExceptionUserInput,
            "The type of the stored value cannot be converted to float.");


    }



}