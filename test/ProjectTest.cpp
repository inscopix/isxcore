#include "isxProject.h"
#include "catch.hpp"
#include "isxTest.h"

#include <stdio.h>

TEST_CASE("ProjectTest", "[core]")
{
    std::string projectFileName = g_resources["testDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());

    std::string projectName = "myProject";

    SECTION("Empty constructor")
    {
        isx::SpProject_t project = std::make_shared<isx::Project>();
        REQUIRE(!project->isValid());
    }

    SECTION("Create a new project.")
    {
        isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName, projectName);
        REQUIRE(project->isValid());
        REQUIRE(project->getGroup("/")->getGroups().size() == 2);
        REQUIRE_NOTHROW(project->getGroup("/Original"));
        REQUIRE_NOTHROW(project->getGroup("/Output"));
    }

    SECTION("Open an existing project after adding a group.")
    {
        isx::SpGroup_t origGroup = std::make_shared<isx::Group>("myGroup");
        {
            isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName, projectName);
            isx::SpGroup_t group = project->getGroup("/");
            group->addGroup(origGroup);
        }
        isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName);
        REQUIRE(project->isValid());
        REQUIRE(*(project->getGroup("/myGroup")) == *origGroup);
    }

    SECTION("Open an existing project atfer adding some typical groups and data sets")
    {
        std::string baseName = "recording-20160808-133943";
        std::string origMovieFileName = g_resources["testDataPath"] + "/" + baseName + ".isxd";
        std::remove(origMovieFileName.c_str());

        std::string outMovieFileName = g_resources["testDataPath"] + "/" + baseName + "-pp.isxd";
        std::remove(outMovieFileName.c_str());

        isx::SpDataSet_t origMovie = std::make_shared<isx::DataSet>(
                baseName, isx::DataSet::Type::MOVIE, origMovieFileName);
        isx::SpDataSet_t outMovie = std::make_shared<isx::DataSet>(
                baseName + "-pp", isx::DataSet::Type::MOVIE, outMovieFileName);
        {
            isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName, projectName);
            isx::SpGroup_t origGroup = project->getGroup("/Original");
            origGroup->addDataSet(origMovie);

            isx::SpGroup_t outGroup = project->getGroup("/Output");
            outGroup->addDataSet(outMovie);
        }

        isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName);
        REQUIRE(project->isValid());
        REQUIRE(*(project->getDataSet("/Original/" + baseName)) == *origMovie);
        REQUIRE(*(project->getDataSet("/Output/" + baseName + "-pp")) == *outMovie);
    }
}
