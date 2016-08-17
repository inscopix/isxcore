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
