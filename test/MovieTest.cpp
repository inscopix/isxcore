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

    SECTION("getFrame") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
        size_t s = m.getFrameSizeInBytes();
        std::vector<unsigned char> t(s);
        m.getFrame(0, &t[0], s);
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
        int nFrames, nCols, nRows;
        isx::TimingInfo timingInfo;
        isx::Ratio timeStep, frameRate;
        nFrames = inputMovie.getNumFrames();
        nCols   = inputMovie.getFrameWidth();
        nRows   = inputMovie.getFrameHeight();
        timingInfo = inputMovie.getTimingInfo();
        timeStep = timingInfo.getStep();
        frameRate = timeStep.invert();

        // Outputs
        std::string	outputFilename = g_resources["testDataPath"] + "/movieout.hdf5";
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(outputFilename);
        
        uint16_t nSeries = outputFile->getNumMovieSeries();
        isx::SpMovieSeries_t rs;
        isx::SpMovie_t outputMovie; 
        
        if(nSeries == 0)
        {
            // Create it
            rs = outputFile->addMovieSeries("RecSeries0");
            outputMovie = rs->addMovie("Movie0", nFrames, nCols, nRows, frameRate);
        }
        else
        {
            // Read it
            rs = outputFile->getMovieSeries(0);
            uint16_t nMovies = rs->getNumMovies();
            if(nMovies == 0)
            {
                outputMovie = rs->addMovie("Movie0", nFrames, nCols, nRows, frameRate);
            }
            else
            {
                outputMovie = rs->getMovie(0);
            }            
        }        
        
        REQUIRE(nFrames == outputMovie->getNumFrames());
        REQUIRE(nCols == outputMovie->getFrameWidth());
        REQUIRE(nRows == outputMovie->getFrameHeight());
        
        timingInfo = outputMovie->getTimingInfo();
        timeStep = timingInfo.getStep();
        isx::Ratio outpuFrameRate = timeStep.invert();
        REQUIRE(frameRate == outpuFrameRate);

        // Write a frame from the input movie to the output movie
        int nFrame = 15;
        size_t inputSize = inputMovie.getFrameSizeInBytes();
        std::vector<unsigned char> inputFrameBuffer(inputSize);
        inputMovie.getFrame(nFrame, &inputFrameBuffer[0], inputSize);
        outputMovie->writeFrame(nFrame, &inputFrameBuffer[0], inputSize); 
        
        // Read dataset from output
        size_t outputSize = outputMovie->getFrameSizeInBytes();
        std::vector<unsigned char> outputFrameBuffer(outputSize);        
        outputMovie->getFrame(nFrame, &outputFrameBuffer[0], outputSize);

        int nCol = 35;
        int nRow = 3;
        int idx = (nRows * nCol + nRow) * 2;
        REQUIRE(inputFrameBuffer[idx] == outputFrameBuffer[idx]);  
        REQUIRE(inputFrameBuffer[idx+1] == outputFrameBuffer[idx+1]);        

    } 

}
