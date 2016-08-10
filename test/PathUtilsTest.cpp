#include "isxPathUtils.h"
#include "catch.hpp"
#include "isxTest.h"

TEST_CASE("PathUtils-getRelativePath", "[core-internal]")
{

#if ISX_OS_WIN32

    SECTION("Windows file system same drive")
    {
        std::string path = "C:/inscopix/data/movie.isxd";
        std::string dirName = "C:/inscopix/projects/myProject";

        std::string relPath = isx::getRelativePath(path, dirName);

        REQUIRE(relPath == "../../data/movie.isxd");
    }

    SECTION("Windows file system different drive")
    {
        std::string path = "C:/inscopix/data/movie.isxd";
        std::string dirName = "D:/inscopix/projects/myProject";

        std::string relPath = isx::getRelativePath(path, dirName);

        REQUIRE(relPath == "../../../data/movie.isxd");
    }

#endif

    SECTION("Unix file system - both absolute")
    {
        std::string path = "/inscopix/data/movie.isxd";
        std::string dirName = "/inscopix/projects/myProject";

        std::string relPath = isx::getRelativePath(path, dirName);

        REQUIRE(relPath == "../../data/movie.isxd");
    }

}
