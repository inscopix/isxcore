#include "isxRecording.h"
#include "isxMovie.h"
#include "catch.hpp"
#include "isxTest.h"

#include <string>
#include <map>


TEST_CASE("RecordingTest", "[core]") {
    std::string testFile = g_resources["testDataPath"] + "/recording_20160426_145041.hdf5";
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
    
    SECTION("Load recording XML", "[core]")
    {
        std::string testXML = g_resources["testDataPath"] + "/recording_20160706_132714.xml";
        std::string testHDF5_0 = g_resources["testDataPath"] + "/recording_20160706_132714.hdf5";
        std::string testHDF5_1 = g_resources["testDataPath"] + "/recording_20160706_132714-001.hdf5";
        
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
          
        isx::SpU16VideoFrame_t framesXML[2];
        framesXML[0] = mXml->getFrame(frameIdxXML[0]);
        framesXML[1] = mXml->getFrame(frameIdxXML[1]);
                
        isx::SpU16VideoFrame_t framesHDF5[2];
        framesHDF5[0] = mHdf0->getFrame(frameIdxHDF[0]);
        framesHDF5[1] = mHdf1->getFrame(frameIdxHDF[1]);
        
        uint16_t *p0_x = framesXML[0]->getPixels();
        uint16_t *p0_h = framesHDF5[0]->getPixels();
        uint16_t *p1_x = framesXML[1]->getPixels();
        uint16_t *p1_h = framesHDF5[1]->getPixels();
        REQUIRE(*p0_x == *p0_h);
        REQUIRE(*p1_x == *p1_h);
    }
    isx::CoreShutdown();

}
