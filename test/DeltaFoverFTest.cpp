#include "isxRecording.h"
#include "isxDeltaFoverF.h"
#include "isxMovie.h"
#include "isxProjectFile.h"
#include "catch.hpp"

#include "isxTest.h"
#include <vector>

TEST_CASE("DeltaFoverFTest", "[!hide][core]") {
    std::string testFile = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    SECTION("applyAppDFF") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::SpMovie_t m = std::make_shared<isx::Movie>(r->getHdf5FileHandle(), "/images");

        std::string outputFilename = g_resources["testDataPath"] + "/movieout.hdf5";
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(outputFilename, testFile);
        
        isx::SpMovieSeries_t rs = outputFile->addMovieSeries("RecSeries0");
        isx::SpMovie_t outputMovie = rs->addMovie(
            "Movie0", 
            m->getNumFrames(), 
            m->getFrameWidth(), 
            m->getFrameHeight(), 
            m->getTimingInfo().getStep().invert());

        isx::DeltaFoverF algo = isx::DeltaFoverF(m);
        REQUIRE(algo.IsValid());
        algo.SetOutputMovie(outputMovie);
        algo.ApplyApp();
    }
}
