#include "isxRecording.h"
#include "isxDeltaFoverF.h"
#include "isxNVistaMovie.h"
#include "isxProjectFile.h"
#include "catch.hpp"

#include "isxTest.h"
#include <vector>

TEST_CASE("DeltaFoverFTest", "[!hide][core]") {
    std::string testFile = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    SECTION("applyAppDFF") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::SpMovie_t m = std::make_shared<isx::NVistaMovie>(r->getHdf5FileHandle(), "/images");

        std::string outputFilename = g_resources["testDataPath"] + "/movieout.hdf5";
        std::vector<std::string> inputName(1);
        inputName[0] = testFile;
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(outputFilename, inputName);
        
        isx::SpMovieSeries_t rs = outputFile->addMovieSeries("RecSeries0");
        isx::SpMovie_t outputMovie = rs->addMovie(
            "Movie0", 
            m->getNumFrames(), 
            m->getFrameWidth(), 
            m->getFrameHeight(), 
            m->getTimingInfo().getStep().invert());

        isx::DeltaFoverF algo = isx::DeltaFoverF(m);
        REQUIRE(algo.isValid());
        algo.setOutputMovie(outputMovie);
        algo.run([](float){return false;});
    }
}
