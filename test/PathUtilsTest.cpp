#include "isxPathUtils.h"
#include "catch.hpp"
#include "isxTest.h"

TEST_CASE("PathUtils-getBaseName", "[core]")
{

    SECTION("File name with extension")
    {
        REQUIRE(isx::getBaseName("movie.isxd") == "movie");
    }

    SECTION("File name without extension")
    {
        REQUIRE(isx::getBaseName("outputs") == "outputs");
    }

    SECTION("File name with directory name and extension.")
    {
        REQUIRE(isx::getBaseName("outputs/movie.isxd") == "movie");
    }

    SECTION("File name with directory and without extension.")
    {
        REQUIRE(isx::getBaseName("outputs/day1") == "day1");
    }

}

TEST_CASE("PathUtils-getFileName", "[core]")
{

    SECTION("File name with extension")
    {
        REQUIRE(isx::getFileName("movie.isxd") == "movie.isxd");
    }

    SECTION("File name without extension")
    {
        REQUIRE(isx::getFileName("outputs") == "outputs");
    }

    SECTION("File name with directory name and extension.")
    {
        REQUIRE(isx::getFileName("outputs/movie.isxd") == "movie.isxd");
    }

    SECTION("File name with directory and without extension.")
    {
        REQUIRE(isx::getFileName("outputs/day1") == "day1");
    }
    
}

TEST_CASE("PathUtils-getDirName", "[core]")
{

    SECTION("File name with extension")
    {
        REQUIRE(isx::getDirName("movie.isxd") == ".");
    }

    SECTION("File name without extension")
    {
        REQUIRE(isx::getDirName("outputs") == ".");
    }

    SECTION("File name with directory name and extension")
    {
        REQUIRE(isx::getDirName("outputs/movie.isxd") == "outputs");
    }

    SECTION("File name with directory and without extension")
    {
        REQUIRE(isx::getDirName("outputs/day1") == "outputs");
    }
    
}

TEST_CASE("PathUtils-getExtension", "[core]")
{

    SECTION("File name with extension")
    {
        REQUIRE(isx::getExtension("movie.isxd") == "isxd");
    }

    SECTION("File name with extension with two periods")
    {
        REQUIRE(isx::getExtension("movie.isxd.gz") == "isxd.gz");
    }

    SECTION("File name without extension")
    {
        REQUIRE(isx::getExtension("outputs") == "");
    }

    SECTION("File name with directory name and extension.")
    {
        REQUIRE(isx::getExtension("outputs/movie.isxd") == "isxd");
    }

    SECTION("File name with directory and without extension.")
    {
        REQUIRE(isx::getExtension("outputs/day1") == "");
    }
    
}

TEST_CASE("PathUtils-getRelativePath", "[core]")
{

#if ISX_OS_WIN32

    SECTION("Windows file system same drive")
    {
        std::string path = "C:/inscopix/data/movie.isxd";
        std::string dirName = "C:/inscopix/projects/myProject";

        std::string relPath = isx::getRelativePath(path, dirName);

        REQUIRE(relPath == "../../data/movie.isxd");
    }

    // TODO sweet : if we always want relative paths on Windows we need
    // to deal with different drive names (as well as UNC paths for network
    // drives)
    //SECTION("Windows file system different drive")
    //{
    //    std::string path = "C:/inscopix/data/movie.isxd";
    //    std::string dirName = "D:/inscopix/projects/myProject";

    //    std::string relPath = isx::getRelativePath(path, dirName);

    //    REQUIRE(relPath == "../../../C:/data/movie.isxd");
    //}

#endif

#if ISX_OS_LINUX || ISX_OS_MACOS

    SECTION("Unix file system - both absolute")
    {
        std::string path = "/inscopix/data/movie.isxd";
        std::string dirName = "/inscopix/projects/myProject";

        std::string relPath = isx::getRelativePath(path, dirName);

        REQUIRE(relPath == "../../data/movie.isxd");
    }

#endif

}

TEST_CASE("PathUtils-appendNumberToPath")
{

    SECTION("Width = 2")
    {
        REQUIRE(isx::appendNumberToPath("/a/b", 1, 2) == "/a/b_01");
    }

    SECTION("Width = 3")
    {
        REQUIRE(isx::appendNumberToPath("/a/b", 2, 3) == "/a/b_002");
    }

}
