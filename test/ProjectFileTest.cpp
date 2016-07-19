#include "isxProjectFile.h"
#include "catch.hpp"
#include "isxTest.h"
#include <stdio.h>

TEST_CASE("ProjectFileTest", "[core]") {
    std::string testFile = g_resources["testDataPath"] + "/projectfile.isxp";

    SECTION("default constructor") {        
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>();
        REQUIRE(!outputFile->isValid());
    }

    SECTION("constructor - create new file") {
        std::remove(testFile.c_str());
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(testFile);
        REQUIRE(outputFile->isValid());
    }

    SECTION("constructor - open existing file") {
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(testFile);
        REQUIRE(outputFile->isValid());
    }

    SECTION("get number of data collections") {
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(testFile);
        isx::isize_t nDataCollections = outputFile->getNumDataCollections();
        REQUIRE(nDataCollections == 0);
    }

    SECTION("add data collection and save") {
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(testFile);        
        isx::ProjectFile::DataCollection dc;
        dc.name = "dummyCollection";        
        isx::ProjectFile::DataFileDescriptor dfd;
        dfd.filename = "dummyFile";
        dfd.type = isx::ProjectFile::PF_DATAFILETYPE_MOVIE;        
        dc.files.push_back(dfd);        
        outputFile->addDataCollection(dc);
        isx::isize_t nDataCollections = outputFile->getNumDataCollections();
        REQUIRE(nDataCollections == 1);
    }


    SECTION("get data collection") {
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(testFile);
        isx::isize_t nDataCollections = outputFile->getNumDataCollections();
        REQUIRE(nDataCollections == 1);

        isx::ProjectFile::DataCollection dc = outputFile->getDataCollection(0);
        REQUIRE(dc.name == "dummyCollection");
        REQUIRE(dc.files[0].filename == "dummyFile");
        REQUIRE(dc.files[0].type == isx::ProjectFile::PF_DATAFILETYPE_MOVIE);        
    }

    SECTION("add file to data collection") {
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(testFile);
        isx::isize_t nDataCollections = outputFile->getNumDataCollections();
        REQUIRE(nDataCollections == 1);

        isx::ProjectFile::DataFileDescriptor dfd;
        dfd.filename = "addedDummyFile";
        dfd.type = isx::ProjectFile::PF_DATAFILETYPE_MOVIE;
        outputFile->addFileToDataCollection(dfd, 0);

        isx::ProjectFile::DataCollection dc = outputFile->getDataCollection(0);
        REQUIRE(dc.name == "dummyCollection");
        REQUIRE(dc.files.size() == 2);
        REQUIRE(dc.files[1].filename == "addedDummyFile");
    }

    SECTION("remove file from data collection") {
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(testFile);
        isx::isize_t nDataCollections = outputFile->getNumDataCollections();
        REQUIRE(nDataCollections == 1);
        outputFile->removeFileFromDataCollection(1, 0);
        isx::ProjectFile::DataCollection dc = outputFile->getDataCollection(0);
        REQUIRE(dc.name == "dummyCollection");
        REQUIRE(dc.files.size() == 1);
    }

    SECTION("remove data collection") {
        isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(testFile);
        isx::isize_t nDataCollections = outputFile->getNumDataCollections();
        REQUIRE(nDataCollections == 1);
        outputFile->removeDataCollection(0);
        nDataCollections = outputFile->getNumDataCollections();
        REQUIRE(nDataCollections == 0);
    }


}
