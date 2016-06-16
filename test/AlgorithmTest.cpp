#include "isxRecording.h"
#include "isxAlgorithm.h"
#include "isxMovie.h"
#include "isxProjectFile.h"
#include "catch.hpp"

#include "isxTest.h"
#include <vector>

TEST_CASE("AlgorithmTest", "[!hide][core]") {
    std::string testFile = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    SECTION("applyAppDFF") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::SpMovie_t m = std::make_shared<isx::Movie>(r->getHdf5FileHandle(), "/images");
        std::string	outputFilename = g_resources["testDataPath"] + "/DFFout.hdf5";
        isx::Algorithm algo = isx::Algorithm(m);
        REQUIRE(algo.IsValid());
        algo.SetOutputFileName(outputFilename);
        algo.ApplyApp();
    }
}
