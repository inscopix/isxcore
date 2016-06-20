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
        nFrames = inputMovie.getNumFrames();
        nCols   = inputMovie.getFrameWidth();
        nRows   = inputMovie.getFrameHeight();

        // Outputs
        std::string	outputFilename = g_resources["testDataPath"] + "/movieout.hdf5";
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(outputFilename);
        
        isx::Movie outputMovie(outputFile->getHdf5FileHandle(), "/MosaicProject/Schedules/Schedule1/Recording1/Movie", nFrames, nCols, nRows); 
        REQUIRE(nFrames == outputMovie.getNumFrames());
        REQUIRE(nCols == outputMovie.getFrameWidth());
        REQUIRE(nRows == outputMovie.getFrameHeight());

        // Write a frame from the input movie to the output movie
        int nFrame = 15;
        size_t inputSize = inputMovie.getFrameSizeInBytes();
        std::vector<unsigned char> inputFrameBuffer(inputSize);
        inputMovie.getFrame(nFrame, &inputFrameBuffer[0], inputSize);
        outputMovie.writeFrame(nFrame, &inputFrameBuffer[0], inputSize); 
        
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

    SECTION("Create movie with timing and spacing info", "[core]") {

        std::string outFileName = g_resources["testDataPath"] + "/MovieTest-createWithTimingSpacingInfo.hdf5";
        isx::SpProjectFile_t outFile = std::make_shared<isx::ProjectFile>(outFileName);

        isx::Time start(2016, 6, 20, 10, 32);
        isx::Ratio step(50, 1000);
        size_t numTimes = 5;
        isx::TimingInfo timingInfo(start, step, numTimes);

        isx::Point<isx::Ratio> topLeft(0, 0);
        isx::Point<isx::Ratio> pixelSize(isx::Ratio(22, 10), isx::Ratio(22, 10));
        isx::Point<size_t> numPixels(24, 16);
        isx::SpacingInfo spacingInfo(topLeft, pixelSize, numPixels);

        isx::Movie movie(outFile->getHdf5FileHandle(),
                "/MosaicProject/Schedules/Schedule1/Recording1/Movie",
                timingInfo, spacingInfo);

        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
    }

}
