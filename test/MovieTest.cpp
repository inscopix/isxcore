#include "isxRecording.h"
#include "isxMovie.h"
#include "isxProjectFile.h"
#include "catch.hpp"

#include "isxTest.h"
#include <vector>

TEST_CASE("MovieTest", "[core]") {
    std::string testFile = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    SECTION("default constructor") {
        isx::Movie m;
        REQUIRE(!m.isValid());
    }

    SECTION("create movie from dataset in recording") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
    }

    SECTION("getNumFrames") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getNumFrames() == 33);
    }

    SECTION("getFrameWidth") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getFrameWidth() == 500);
    }

    SECTION("getFrameHeight") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getFrameHeight() == 500);
    }

    SECTION("getFrameSizeInBytes") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getFrameSizeInBytes() == 500000);
    }

    SECTION("getFrame for time") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
        auto nvf = m.getFrame(m.getTimingInfo().getStart());
        unsigned char * t = reinterpret_cast<unsigned char *>(nvf->getPixels());
        REQUIRE(t[0] == 0x43);
        REQUIRE(t[1] == 0x3);
    }

    SECTION("getFrame for frame number") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
        auto nvf = m.getFrame(0);
        unsigned char * t = reinterpret_cast<unsigned char *>(nvf->getPixels());
        REQUIRE(t[0] == 0x43);
        REQUIRE(t[1] == 0x3);
    }

    SECTION("getDurationInSeconds") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getDurationInSeconds() == 1.1);
    }

    SECTION("toString") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.toString() == "/images");
    }

    SECTION("Write frames to new movie", "[core]") {
        // Inputs
        isx::SpRecording_t inputFile = std::make_shared<isx::Recording>(testFile);
        isx::Movie inputMovie(inputFile->getHdf5FileHandle(), "/images");
        
        // Get sizes from input
        isx::isize_t nFrames = inputMovie.getNumFrames();
        isx::isize_t nCols  = inputMovie.getFrameWidth();
        isx::isize_t nRows  = inputMovie.getFrameHeight();
 		isx::TimingInfo timingInfo = inputMovie.getTimingInfo();
        isx::Ratio timeStep = timingInfo.getStep();
		isx::Ratio frameRate = timeStep.invert();

        // Create the output
        std::string	outputFilename = g_resources["testDataPath"] + "/movieout.hdf5";
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(outputFilename, testFile);
        
        isx::SpMovieSeries_t rs = outputFile->addMovieSeries("RecSeries0");
        isx::SpMovie_t outputMovie = rs->addMovie("Movie0", nFrames, nCols, nRows, frameRate);             
        
        REQUIRE(nFrames == outputMovie->getNumFrames());
        REQUIRE(nCols == outputMovie->getFrameWidth());
        REQUIRE(nRows == outputMovie->getFrameHeight());
        
        timingInfo = outputMovie->getTimingInfo();
        timeStep = timingInfo.getStep();
        isx::Ratio outpuFrameRate = timeStep.invert();
        REQUIRE(frameRate == outpuFrameRate);

        // Write a frame from the input movie to the output movie
        isx::isize_t nFrame = 15;
        isx::isize_t inputSize = inputMovie.getFrameSizeInBytes();
        isx::Time frame15Time = inputMovie.getTimingInfo().getStart();
        frame15Time = frame15Time.addSecs(isx::Ratio(nFrame, 1) * inputMovie.getTimingInfo().getStep());
        auto nvf = inputMovie.getFrame(frame15Time);
        unsigned char * inputFrameBuffer = reinterpret_cast<unsigned char *>(nvf->getPixels());
        outputMovie->writeFrame(nFrame, inputFrameBuffer, inputSize); 
        
        // Read dataset from output
        auto outputNvf = outputMovie->getFrame(frame15Time);
        unsigned char * outputFrameBuffer = reinterpret_cast<unsigned char *>(outputNvf->getPixels());

        isx::isize_t nCol = 35;
        isx::isize_t nRow = 3;
        isx::isize_t idx = (nRows * nCol + nRow) * 2;
        REQUIRE(inputFrameBuffer[idx] == outputFrameBuffer[idx]);  
        REQUIRE(inputFrameBuffer[idx+1] == outputFrameBuffer[idx+1]);        

    } 

}
