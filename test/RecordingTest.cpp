#include "isxRecording.h"
#include "isxMovie.h"
#include "catch.hpp"
#include "isxTest.h"

#include <string>
#include <map>


TEST_CASE("RecordingTest", "[core]") {
    std::string testFile = g_resources["unitTestDataPath"] + "/recording_20160426_145041.hdf5";
    isx::CoreInitialize();
    SECTION("default constructor") {
        isx::Recording r;
        REQUIRE(!r.isValid());
    }

    SECTION("create movie from dataset in recording") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->isValid());
        isx::SpMovie_t m(r->getMovie());
        REQUIRE(m->isValid());
    }

    SECTION("toString") {
        isx::SpRecording_t r = std::make_shared<isx::Recording>(testFile);
        REQUIRE(r->toString() == testFile);
    }
    
    SECTION("Load recording XML - Split movie", "[core]")
    {
        std::string testXML = g_resources["unitTestDataPath"] + "/recording_20160706_132714.xml";
        std::string testHDF5_0 = g_resources["unitTestDataPath"] + "/recording_20160706_132714.hdf5";
        std::string testHDF5_1 = g_resources["unitTestDataPath"] + "/recording_20160706_132714-001.hdf5";
        
        isx::SpRecording_t rXml  = std::make_shared<isx::Recording>(testXML);
        isx::SpRecording_t rHdf0 = std::make_shared<isx::Recording>(testHDF5_0);
        isx::SpRecording_t rHdf1 = std::make_shared<isx::Recording>(testHDF5_1);
        
        REQUIRE(rXml->isValid());
        REQUIRE(rHdf0->isValid());
        REQUIRE(rHdf1->isValid());
        
        isx::SpMovie_t mXml  = rXml->getMovie();
        isx::SpMovie_t mHdf0 = rHdf0->getMovie();
        isx::SpMovie_t mHdf1 = rHdf1->getMovie();
        
        isx::isize_t nFramesXML = mXml->getTimingInfo().getNumTimes();
        isx::isize_t nFramesHdf0 = mHdf0->getTimingInfo().getNumTimes();
        isx::isize_t nFramesHdf1 = mHdf1->getTimingInfo().getNumTimes();
        
        REQUIRE(nFramesXML == nFramesHdf0 + nFramesHdf1);
        
        // Verify that what we read from an XML is the same data 
        // as we would read through independent HDF5 files.
        isx::isize_t frameIdxXML[] = {nFramesHdf0 - 3, nFramesHdf0 + 5};
        isx::isize_t frameIdxHDF[] = {nFramesHdf0 - 3, 5};
          
        isx::SpVideoFrame_t framesXML[2];
        framesXML[0] = mXml->getFrame(frameIdxXML[0]);
        framesXML[1] = mXml->getFrame(frameIdxXML[1]);
                
        isx::SpVideoFrame_t framesHDF5[2];
        framesHDF5[0] = mHdf0->getFrame(frameIdxHDF[0]);
        framesHDF5[1] = mHdf1->getFrame(frameIdxHDF[1]);
        
        char *p0_x = framesXML[0]->getPixels();
        char *p0_h = framesHDF5[0]->getPixels();
        char *p1_x = framesXML[1]->getPixels();
        char *p1_h = framesHDF5[1]->getPixels();
        REQUIRE(*p0_x == *p0_h);
        REQUIRE(*p1_x == *p1_h);
    }

    SECTION("Load recording XML - Read timing and spacing info from XML", "[core]")
    {
        std::string testXML = g_resources["unitTestDataPath"] + "/recording_20160706_132714.xml";
        isx::SpRecording_t rXml  = std::make_shared<isx::Recording>(testXML);
        isx::SpMovie_t mov  = rXml->getMovie();

        isx::Time start(2016, 7, 6, 13, 27, 23, isx::DurationInSeconds(999, 1000));
        isx::DurationInSeconds step(100, 1001);
        isx::isize_t numTimes = 82;
        isx::TimingInfo xmlTimingInfo(start, step, numTimes);
        isx::TimingInfo movTimingInfo = mov->getTimingInfo();
        REQUIRE(movTimingInfo == xmlTimingInfo);

        int64_t downsamplingFactor = 1;
        isx::SizeInPixels_t numPixels = isx::SizeInPixels_t(500, 500);
        isx::SizeInMicrons_t pixelSize = isx::SizeInMicrons_t(isx::DEFAULT_PIXEL_SIZE*downsamplingFactor, isx::DEFAULT_PIXEL_SIZE*downsamplingFactor);
        isx::PointInMicrons_t topLeft = isx::PointInMicrons_t(0, 0);

        isx::SpacingInfo xmlSpacingInfo(numPixels, pixelSize, topLeft);
        isx::SpacingInfo movSpacingInfo = mov->getSpacingInfo();
        REQUIRE(xmlSpacingInfo == movSpacingInfo);

    }
    isx::CoreShutdown();

}
