#include "isxRecording.h"
#include "isxMovie.h"

#include "catch.hpp"

#include "isxTest.h"
#include <vector>

TEST_CASE("MovieTest", "[core]") {
    std::string testFile = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    SECTION("default constructor") {
        isx::Movie m;
        REQUIRE(!m.isValid());
    }

    SECTION("create movie from dataset in recording", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
    }

    SECTION("getNumFrames", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getNumFrames() == 33);
    }

    SECTION("getFrameWidth", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getFrameWidth() == 500);
    }

    SECTION("getFrameHeight", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getFrameHeight() == 500);
    }

    SECTION("getFrameSizeInBytes", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getFrameSizeInBytes() == 500000);
    }

    SECTION("getFrame", "[core]") {
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

    SECTION("getDurationInSeconds", "[core]") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::Movie m(r->getHdf5FileHandle(), "/images");
        REQUIRE(m.isValid());
        REQUIRE(m.getDurationInSeconds() == 1.1);
    }

    SECTION("Set size of dataset for a new movie in a hdf5 file", "[core]") {
        std::string	outputFilename = g_resources["testDataPath"] + "/movieout.hdf5";
        isx::SpRecording_t outputFile = std::make_shared<isx::Recording>(outputFilename, isx::RECOPENMODE_TRUNC);
        isx::Movie m(outputFile->getHdf5FileHandle(), "/Movie");
        REQUIRE(m.isValid() == false);                      // Dataset doesn't exist yet
        REQUIRE(m.setMovieSize(10, 1440, 1080) == true);    // Dataset was successfully created in the file
    }

     SECTION("Write frames to new movie", "[core]") {
        // Inputs
        isx::SpRecording_t inputFile = std::make_shared<isx::Recording>(testFile);
        isx::Movie inputMovie(inputFile->getHdf5FileHandle(), "/images");

        // Outputs
        std::string	outputFilename = g_resources["testDataPath"] + "/movieout.hdf5";
        isx::SpRecording_t outputFile = std::make_shared<isx::Recording>(outputFilename, isx::RECOPENMODE_TRUNC);
        isx::Movie outputMovie(outputFile->getHdf5FileHandle(), "/Movie");

        // Get sizes from input
        int nFrames, nCols, nRows;
        nFrames = inputMovie.getNumFrames();
        nCols   = inputMovie.getFrameWidth();
        nRows   = inputMovie.getFrameHeight();

        // Set sizes in output
        bool result = outputMovie.setMovieSize(nFrames, nCols, nRows);
        REQUIRE(result);
        REQUIRE(nFrames == outputMovie.getNumFrames());
        REQUIRE(nCols == outputMovie.getFrameWidth());
        REQUIRE(nRows == outputMovie.getFrameHeight());

        // Write a frame from the input movie to the output movie
        int nFrame = 15;
        size_t inputSize = inputMovie.getFrameSizeInBytes();
        std::vector<unsigned char> inputFrameBuffer(inputSize);
        inputMovie.getFrame(nFrame, &inputFrameBuffer[0], inputSize);
        result = outputMovie.writeFrame(nFrame, &inputFrameBuffer[0], inputSize);
        REQUIRE(result);
        
        // Read dataset from output
        size_t outputSize = outputMovie.getFrameSizeInBytes();
        std::vector<unsigned char> outputFrameBuffer(outputSize);        
        outputMovie.getFrame(nFrame, &outputFrameBuffer[0], outputSize);

        int nCol = 35;
        int nRow = 3;
        int idx = (nRows * nCol + nRow) * 2;
        REQUIRE(inputFrameBuffer[idx] == outputFrameBuffer[idx]);  
        REQUIRE(inputFrameBuffer[idx+1] == outputFrameBuffer[idx+1]);        

    } 

}
