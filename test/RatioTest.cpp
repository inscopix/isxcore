#include "isxRatio.h"
#include "catch.hpp"

TEST_CASE("RatioTest", "[core]")
{

    SECTION("Default constructor")
    {
        isx::Ratio actual;
        isx::Ratio expected(0, 1);
        REQUIRE(actual == expected);
    }

    SECTION("Conversion from integer")
    {
        isx::Ratio expected(4, 1);
        isx::Ratio actual = 4;
        REQUIRE(actual == expected);
    }

    SECTION("Conversion to double")
    {
        isx::Ratio ratio(4, 8);
        double expected = 0.5;
        double actual = ratio.toDouble();
        REQUIRE(actual == expected);
    }

    SECTION("Addition same denominator")
    {
        isx::Ratio first(3, 6);
        isx::Ratio second(1, 6);
        isx::Ratio actual = first + second;
        isx::Ratio expected(4, 6);
        REQUIRE(actual == expected);
    }

    SECTION("Addition different denominator with shared base")
    {
        isx::Ratio first(3, 9);
        isx::Ratio second(1, 6);
        isx::Ratio actual = first + second;
        isx::Ratio expected(1, 2);
        REQUIRE(actual == expected);
    }

    SECTION("Addition different denominator without shared base")
    {
        isx::Ratio first(3, 9);
        isx::Ratio second(1, 7);
        isx::Ratio actual = first + second;
        isx::Ratio expected(30, 63);
        REQUIRE(actual == expected);
    }

    SECTION("Subtraction same denominator")
    {
        isx::Ratio first(3, 6);
        isx::Ratio second(1, 6);
        isx::Ratio actual = first - second;
        isx::Ratio expected(2, 6);
        REQUIRE(actual == expected);
    }

    SECTION("Subtraction different denominator with shared base")
    {
        isx::Ratio first(3, 9);
        isx::Ratio second(1, 6);
        isx::Ratio actual = first - second;
        isx::Ratio expected(3, 18);
        REQUIRE(actual == expected);
    }

    SECTION("Subtraction different denominator without shared base")
    {
        isx::Ratio first(3, 9);
        isx::Ratio second(1, 7);
        isx::Ratio actual = first - second;
        isx::Ratio expected(12, 63);
        REQUIRE(actual == expected);
    }

    SECTION("Multiplication")
    {
        isx::Ratio first(3, 6);
        isx::Ratio second(1, 6);
        isx::Ratio actual = first * second;
        isx::Ratio expected(3, 36);
        REQUIRE(actual == expected);
    }

    SECTION("Division")
    {
        isx::Ratio first(3, 6);
        isx::Ratio second(1, 6);
        isx::Ratio actual = first / second;
        isx::Ratio expected(3);
        REQUIRE(actual == expected);
    }

    SECTION("Addition with integer")
    {
        isx::Ratio first(3, 6);
        int64_t second = 5;
        isx::Ratio actual = first + second;
        isx::Ratio expected(33, 6);
        REQUIRE(actual == expected);
    }

    SECTION("Subtraction with integer")
    {
        isx::Ratio first(3, 6);
        int64_t second = 3;
        isx::Ratio actual = first - second;
        isx::Ratio expected(-15, 6);
        REQUIRE(actual == expected);
    }

    SECTION("Multiplication with integer")
    {
        isx::Ratio first(3, 6);
        int64_t second = 4;
        isx::Ratio actual = first * second;
        isx::Ratio expected(12, 6);
        REQUIRE(actual == expected);
    }

    SECTION("Division with integer")
    {
        isx::Ratio first(3, 5);
        int64_t second = 4;
        isx::Ratio actual = first / second;
        isx::Ratio expected(3, 20);
        REQUIRE(actual == expected);
    }

    SECTION("Equality operator")
    {
        isx::Ratio first(4, 6);
        isx::Ratio second(8, 12);
        REQUIRE(first == second);
    }

    SECTION("Inequality operator")
    {
        isx::Ratio first(4, 6);
        isx::Ratio second(7, 12);
        REQUIRE(first != second);
    }

    SECTION("Less than operator")
    {
        isx::Ratio first(1, 6);
        isx::Ratio second(3, 7);
        REQUIRE(first < second);
    }

    SECTION("Less than or equal to operator")
    {
        isx::Ratio first(1, 6);
        isx::Ratio second(3, 7);
        REQUIRE(first <= second);
    }

    SECTION("Greater than operator")
    {
        isx::Ratio first(5, 6);
        isx::Ratio second(3, 7);
        REQUIRE(first > second);
    }

    SECTION("Greater than or equal to operator")
    {
        isx::Ratio first(5, 6);
        isx::Ratio second(3, 7);
        REQUIRE(first > second);
    }

    SECTION("String conversion")
    {
        isx::Ratio ratio(3, 7);
        std::string actual = ratio.toString();
        std::string expected("3 / 7");
        REQUIRE(actual == expected);
    }
    
    SECTION("floorToDenomOf")
    {
        isx::Ratio r1(3, 7);
        isx::Ratio r2(20, 23);
        isx::Ratio f = r1.floorToDenomOf(r2);
        REQUIRE(f.getDen() == r2.getDen());
        
        double r1Value = r1.toDouble();
        double fvBelow = f.toDouble();
        double fvAbove = double(f.getNum() + 1) / double(f.getDen());
        
        REQUIRE(fvBelow <= r1Value);
        REQUIRE(fvAbove > r1Value);
        REQUIRE(f.getNum() == 9);
    }

}
