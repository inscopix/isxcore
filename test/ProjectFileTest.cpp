#include "isxProjectFile.h"
#include "catch.hpp"
#include "isxTest.h"

TEST_CASE("ProjectFileTest", "[core]") {
    std::string testFile = g_resources["testDataPath"] + "/projectfile.hdf5";

    SECTION("default constructor") {
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>();
        REQUIRE(!outputFile->isValid());
    }

    SECTION("constructor - create new file") {
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(testFile, "dummyTest");
        REQUIRE(outputFile->isValid());
    }

    SECTION("constructor - open existing file") {
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(testFile);
        REQUIRE(outputFile->isValid());
    }

    SECTION("get number of movie series") {
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(testFile);
        int nMovieSeries = outputFile->getNumMovieSeries();
        REQUIRE(nMovieSeries == 0);
    }

    SECTION("add movie series") {
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(testFile);
        isx::SpMovieSeries_t ms = outputFile->addMovieSeries("dummyMovieSeries");
        REQUIRE(ms);
        int nMovieSeries = outputFile->getNumMovieSeries();
        REQUIRE(nMovieSeries == 1);
    }

    SECTION("get movie series") {
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(testFile);
        isx::SpMovieSeries_t ms = outputFile->getMovieSeries(0);
        REQUIRE(ms);
        REQUIRE(ms->getName() == "dummyMovieSeries");
    }
}
