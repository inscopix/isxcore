#include "isxException.h"
#include "isxTest.h"
#include "catch.hpp"

TEST_CASE("ExceptionTest", "[core]") {

    SECTION("Throw a vanilla exception with no message")
    {
        try
        {
            ISX_THROW(isx::Exception);
            FAIL("Failed to throw an exception");
        }
        catch (const isx::Exception& error)
        {
            REQUIRE(std::string(error.what()) == "");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::Exception");
        }
    }

    SECTION("Throw a DataIO exception with a message")
    {
        try
        {
            ISX_THROW(isx::ExceptionDataIO, "Expected exception - There was a DataIO error");
            FAIL("Failed to throw an exception");
        }
        catch (const isx::ExceptionDataIO& error)
        {
            REQUIRE(std::string(error.what()) == "Expected exception - There was a DataIO error");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionDataIO");
        }
    }

    SECTION("Throw a DataIO exception with a multi-string message")
    {
        try
        {
            ISX_THROW(isx::ExceptionDataIO, "Expected exception - There was a DataIO error with input ", 1);
            FAIL("Failed to throw an exception");
        }
        catch (const isx::ExceptionDataIO& error)
        {
            REQUIRE(std::string(error.what()) == "Expected exception - There was a DataIO error with input 1");
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionDataIO");
        }
    }

}
