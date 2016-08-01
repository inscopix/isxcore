#include "isxProject.h"
#include "catch.hpp"
#include "isxTest.h"
#include <stdio.h>

TEST_CASE("ProjectTest", "[core]")
{
    std::string projectFileName = g_resources["testDataPath"] + "/projectFile.isxp";
    std::remove(projectFileName.c_str());

    isx::CoreInitialize();

    SECTION("Empty constructor")
    {
        auto project = std::make_shared<isx::Project>();
        REQUIRE(!project->isValid());
    }

    SECTION("Create a new empty project.")
    {
        auto project = std::make_shared<isx::Project>(projectFileName);
        REQUIRE(project->isValid());
        isx::ProjectFile::DataCollection dc = project->getDataCollection(0);
        REQUIRE(project->getNumDataCollections() == 0);
    }

    SECTION("Open an existing project.")
    {
        {
            auto project = std::make_shared<isx::Project>(projectFileName);
        }
        auto project = std::make_shared<isx::Project>(projectFileName);
        REQUIRE(project->isValid());
        REQUIRE(project->getNumDataCollections() == 0);
    }

    SECTION("Create a mosaic movie in a project.")
    {
        std::string movieFileName = g_resources["testDataPath"] + "/movieFileName.isxd";
        std::remove(movieFileName.c_str());

        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::isize_t numFrames = 5;
        isx::TimingInfo timingInfo(start, step, numFrames);

        isx::SizeInPixels_t numPixels(4, 3);
        isx::SizeInMicrons_t pixelSize(isx::Ratio(22, 10), isx::Ratio(22, 10));
        isx::PointInMicrons_t topLeft(0, 0);
        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        auto project = std::make_shared<isx::Project>(projectFileName);
        REQUIRE(project->isValid());

        isx::ProjectFile::DataCollection dc;
        dc.name = "Movies";
        project->addDataCollection(dc);

        isx::SpWritableMovie_t movie = project->createMosaicMovie(0, movieFileName, timingInfo, spacingInfo);
        REQUIRE(movie->isValid());
    }

    isx::CoreShutdown();
}
