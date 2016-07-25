#include "isxProject.h"
#include "catch.hpp"
#include "isxTest.h"
#include <stdio.h>

TEST_CASE("ProjectTest", "[core]")
{
    std::string projectFileName = g_resources["testDataPath"] + "/projectFile.isxp";
    std::remove(projectFileName.c_str());


    SECTION("Empty constructor")
    {
        std::shared_ptr<isx::Project> project = std::make_shared<isx::Project>();
        REQUIRE(!project->isValid());
    }

    SECTION("Create a new empty project.")
    {
        std::shared_ptr<isx::Project> project = std::make_shared<isx::Project>(projectFileName);
        REQUIRE(project->isValid());
        isx::ProjectFile::DataCollection dc = project->getDataCollection(0);
        REQUIRE(dc.name == "root");
        REQUIRE(dc.files.empty() == true);
    }

    SECTION("Open an existing project.")
    {
        {
            std::shared_ptr<isx::Project> project = std::make_shared<isx::Project>(projectFileName);
        }
        std::shared_ptr<isx::Project> project = std::make_shared<isx::Project>(projectFileName);
        REQUIRE(project->isValid());
        isx::ProjectFile::DataCollection dc = project->getDataCollection(0);
        REQUIRE(dc.name == "root");
        REQUIRE(dc.files.empty() == true);
    }

    SECTION("Create a mosaic movie in a project.")
    {
        std::string movieFile = g_resources["testDataPath"] + "/movieFile.isxd";
        std::remove(movieFile.c_str());

        isx::Time start;
        isx::DurationInSeconds step(50, 1000);
        isx::isize_t numFrames = 5;
        isx::TimingInfo timingInfo(start, step, numFrames);

        isx::SizeInPixels_t numPixels(4, 3);
        isx::SizeInMicrons_t pixelSize(isx::Ratio(22, 10), isx::Ratio(22, 10));
        isx::PointInMicrons_t topLeft(0, 0);
        isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

        std::shared_ptr<isx::Project> project = std::make_shared<isx::Project>(projectFileName);
        REQUIRE(project->isValid());
        isx::SpWritableMovie_t movie = project->createMosaicMovie(movieFile, timingInfo, spacingInfo);
        REQUIRE(movie->isValid());
    }

}
