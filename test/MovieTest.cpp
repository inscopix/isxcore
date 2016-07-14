#include "isxRecording.h"
#include "isxMovieInterface.h"
#include "isxProjectFile.h"
#include "isxCore.h"
#include "catch.hpp"

#include "isxTest.h"
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

TEST_CASE("MovieTest", "[core]") {
    std::string testFile = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    isx::CoreInitialize();

    SECTION("create movie from dataset in recording") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::SpMovieInterface_t m(r->getMovie());
        REQUIRE(m->isValid());
    }

    SECTION("getTimingInfo().getNumTimes()") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::SpMovieInterface_t m(r->getMovie());
        REQUIRE(m->isValid());
        REQUIRE(m->getTimingInfo().getNumTimes() == 33);
    }

    SECTION("getSpacingInfo().getNumColumns") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::SpMovieInterface_t m(r->getMovie());
        REQUIRE(m->isValid());
        REQUIRE(m->getSpacingInfo().getNumColumns() == 500);
    }

    SECTION("getSpacingInfo().getNumRows") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::SpMovieInterface_t m(r->getMovie());
        REQUIRE(m->isValid());
        REQUIRE(m->getSpacingInfo().getNumRows() == 500);
    }

    SECTION("getFrame for time") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::SpMovieInterface_t m(r->getMovie());
        REQUIRE(m->isValid());
        auto nvf = m->getFrame(m->getTimingInfo().getStart());
        unsigned char * t = reinterpret_cast<unsigned char *>(nvf->getPixels());
        REQUIRE(t[0] == 0x43);
        REQUIRE(t[1] == 0x3);
    }

    SECTION("getFrame for frame number") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::SpMovieInterface_t m(r->getMovie());
        REQUIRE(m->isValid());
        auto nvf = m->getFrame(0);
        unsigned char * t = reinterpret_cast<unsigned char *>(nvf->getPixels());
        REQUIRE(t[0] == 0x43);
        REQUIRE(t[1] == 0x3);
    }

    SECTION("getDurationInSeconds") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::SpMovieInterface_t m(r->getMovie());
        REQUIRE(m->isValid());
        REQUIRE(m->getDurationInSeconds() == 1.1);
    }

    SECTION("toString") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::SpMovieInterface_t m(r->getMovie());
        REQUIRE(m->toString() == "/images");
    }

    SECTION("Write and read timing info", "[core]") {
        // Write
        isx::TimingInfo writtenTI, readTI;
        std::string	outputFilename = g_resources["testDataPath"] + "/movieout.hdf5";

        {
            // Inputs
            isx::SpRecording_t inputFile = std::make_shared<isx::Recording>(testFile);
            isx::SpMovieInterface_t inputMovie(inputFile->getMovie());

            // Get sizes from input
            isx::isize_t nFrames = inputMovie->getTimingInfo().getNumTimes();
            isx::isize_t nCols = inputMovie->getSpacingInfo().getNumColumns();
            isx::isize_t nRows = inputMovie->getSpacingInfo().getNumRows();
            isx::TimingInfo timingInfo = inputMovie->getTimingInfo();
            isx::Ratio timeStep = timingInfo.getStep();
            isx::Ratio frameRate = timeStep.getInverse();

            // Create the output
            std::vector<std::string> inputName(1);
            inputName[0] = testFile;
            isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(outputFilename, inputName);

            isx::SpMovieSeries_t rs = outputFile->addMovieSeries("RecSeries0");
            isx::SpMovieInterface_t outputMovie = rs->addMovie("Movie0", nFrames, nCols, nRows, frameRate);
            writtenTI = outputMovie->getTimingInfo();
        }
        
        // Read
        {
            isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(outputFilename);
            isx::SpMovieSeries_t rs = outputFile->getMovieSeries(0);
            isx::SpMovieInterface_t outputMovie = rs->getMovie(0);
            readTI = outputMovie->getTimingInfo();
        }

        REQUIRE(writtenTI.getStart() == readTI.getStart());
        REQUIRE(writtenTI.getStep() == readTI.getStep());
        REQUIRE(writtenTI.getNumTimes() == readTI.getNumTimes());
    }

    SECTION("Write frames to new movie", "[core]") {
        // Inputs
        isx::SpRecording_t inputFile = std::make_shared<isx::Recording>(testFile);
        isx::SpMovieInterface_t inputMovie(inputFile->getMovie());
        
        // Get sizes from input
        isx::isize_t nFrames = inputMovie->getTimingInfo().getNumTimes();
        isx::isize_t nCols  = inputMovie->getSpacingInfo().getNumColumns();
        isx::isize_t nRows  = inputMovie->getSpacingInfo().getNumRows();
 		isx::TimingInfo timingInfo = inputMovie->getTimingInfo();
        isx::Ratio timeStep = timingInfo.getStep();
		isx::Ratio frameRate = timeStep.getInverse();

        // Create the output
        std::string	outputFilename = g_resources["testDataPath"] + "/movieout.hdf5";
        std::vector<std::string> inputName(1);
        inputName[0] = testFile;
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(outputFilename, inputName);
        
        isx::SpMovieSeries_t rs = outputFile->addMovieSeries("RecSeries0");
        isx::SpMovieInterface_t outputMovie = rs->addMovie("Movie0", nFrames, nCols, nRows, frameRate);
        
        REQUIRE(nFrames == outputMovie->getTimingInfo().getNumTimes());
        REQUIRE(nCols == outputMovie->getSpacingInfo().getNumColumns());
        REQUIRE(nRows == outputMovie->getSpacingInfo().getNumRows());

        timingInfo = outputMovie->getTimingInfo();
        timeStep = timingInfo.getStep();
        isx::Ratio outpuFrameRate = timeStep.getInverse();
        REQUIRE(frameRate == outpuFrameRate);

        // Write a frame from the input movie to the output movie
        isx::isize_t nFrame = 15;
        isx::Time frame15Time = inputMovie->getTimingInfo().getStart();
        frame15Time += inputMovie->getTimingInfo().getStep() * nFrame;
        auto nvf = inputMovie->getFrame(frame15Time);
        isx::isize_t inputSize = nvf->getImageSizeInBytes();
        unsigned char * inputFrameBuffer = reinterpret_cast<unsigned char *>(nvf->getPixels());

        {
            isx::SpacingInfo spacingInfo(isx::SizeInPixels_t(nCols, nRows));
            isx::isize_t rowBytes = nCols * sizeof(uint16_t);
            isx::isize_t numChannels = 1;
            auto outputFrame = std::make_shared<isx::U16VideoFrame_t>(spacingInfo, rowBytes, numChannels, isx::Time(), nFrame);
            memcpy(outputFrame->getPixels(), nvf->getPixels(), inputSize);
            outputMovie->writeFrame(outputFrame);
        }
        
        // Read dataset from output
        auto outputNvf = outputMovie->getFrame(frame15Time);
        unsigned char * outputFrameBuffer = reinterpret_cast<unsigned char *>(outputNvf->getPixels());

        isx::isize_t nCol = 35;
        isx::isize_t nRow = 3;
        isx::isize_t idx = (nRows * nCol + nRow) * 2;
        REQUIRE(inputFrameBuffer[idx] == outputFrameBuffer[idx]);  
        REQUIRE(inputFrameBuffer[idx+1] == outputFrameBuffer[idx+1]);        

    }

#if 0
    SECTION("Create movie with timing and spacing info", "[core]")
    {
        std::string outFileName = g_resources["testDataPath"] + "/MovieTest-createWithTimingSpacingInfo.hdf5";
        std::vector<std::string> inputName(1);
        inputName[0] = testFile;
        isx::SpProjectFile_t outFile = std::make_shared<isx::ProjectFile>(outFileName, inputName);

        isx::Time start(2016, 6, 20, 10, 32);
        isx::Ratio step(50, 1000);
        isx::isize_t numTimes = 5;
        isx::TimingInfo timingInfo(start, step, numTimes);

        isx::SizeInPixels_t numPixels(24, 16);
        isx::SizeInMicrons_t pixelSize(isx::Ratio(22, 10), isx::Ratio(22, 10));
        isx::PointInMicrons_t topLeft(0, 0);
        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        isx::MosaicMovie movie(outFile->getHdf5FileHandle(),
                "/MosaicProject/Schedules/Schedule1/Recording1/Movie",
                timingInfo, spacingInfo);

        REQUIRE(movie.getTimingInfo() == timingInfo);
        REQUIRE(movie.getSpacingInfo() == spacingInfo);
    }
#endif
    
    isx::CoreShutdown();
}

TEST_CASE("MovieTestAsync", "[core]") {
    std::string testFile = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";

    isx::CoreInitialize();
    
    bool isDataCorrect = true;
    std::atomic_int doneCount(0);
    
    uint8_t f0_data[] = { 0x43, 0x03, 0x18, 0x03, 0x55, 0x03, 0x60, 0x03 };
    uint8_t f1_data[] = { 0x2D, 0x03, 0x0C, 0x03, 0x15, 0x03, 0x36, 0x03 };
    uint8_t f2_data[] = { 0xE4, 0x02, 0x0D, 0x03, 0x1D, 0x03, 0xEB, 0x02 };
    uint8_t f3_data[] = { 0xB8 ,0x02, 0xB8, 0x02, 0xAE, 0x02, 0xD0, 0x02 };
    uint8_t f4_data[] = { 0x92, 0x02, 0xA3, 0x02, 0xAD, 0x02, 0x91, 0x02 };
    
    std::vector<uint8_t *> expected = { f0_data, f1_data, f2_data, f3_data, f4_data };
    
    size_t numTestFrames = expected.size();
    size_t numTestBytesPerFrame = sizeof(f0_data);
    
    isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
    REQUIRE(r->isValid());
    isx::SpMovieInterface_t m(r->getMovie());
    REQUIRE(m->isValid());
    isx::MovieGetFrameCB_t cb = [&](isx::SpU16VideoFrame_t inFrame){
        size_t index = inFrame->getFrameIndex();
        unsigned char * t = reinterpret_cast<unsigned char *>(inFrame->getPixels());
        if (memcmp(t, expected[index], numTestBytesPerFrame))
        {
            isDataCorrect = false;
        }
        ++doneCount;
    };

    SECTION("get frame for time asynchronously") {
        isx::Time fetchTime = m->getTimingInfo().getStart();
        for (size_t i = 0; i < numTestFrames; ++i)
        {
            m->getFrameAsync(fetchTime, cb);
            fetchTime += m->getTimingInfo().getStep();
        }

        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numTestFrames))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == int(numTestFrames));
        REQUIRE(isDataCorrect);
    }

    SECTION("get frame for frame number asynchronously") {
        for (size_t i = 0; i < numTestFrames; ++i)
        {
            m->getFrameAsync(i, cb);
        }
        
        for (int i = 0; i < 250; ++i)
        {
            if (doneCount == int(numTestFrames))
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(doneCount == numTestFrames);
        REQUIRE(isDataCorrect);
    }

    isx::CoreShutdown();
}
