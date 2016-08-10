#include "isxProject.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"

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
        REQUIRE_NOTHROW(project->getOriginalGroup());
        REQUIRE_NOTHROW(project->getRootGroup());
    }

    SECTION("Open an existing project after adding a group.")
    {
        isx::SpGroup_t group;
        {
            isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName, projectName);
            group = project->getRootGroup()->createGroup("myGroup");
        }
        isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName);
        REQUIRE(project->isValid());
        REQUIRE(*(project->getGroup("/myGroup")) == *group);
    }

    SECTION("Open an existing project after adding some typical groups and data sets")
    {
        std::string baseName = "recording-20160808-133943";
        std::string origMovieName = baseName;
        std::string outMovieName = baseName + "-pp";

        isx::SpDataSet_t origMovie, outMovie;
        {
            isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName, projectName);
            origMovie = project->getOriginalGroup()->createDataSet(baseName,
                    isx::DataSet::Type::MOVIE, baseName + ".isxd");
            outMovie = project->getOutputGroup()->createDataSet(baseName + "-pp",
                    isx::DataSet::Type::MOVIE, baseName + "-pp.isxd");
        }

        isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName);
        REQUIRE(project->isValid());
        REQUIRE(*(project->getOriginalGroup()->getDataSet(baseName)) == *origMovie);
        REQUIRE(*(project->getOutputGroup()->getDataSet(baseName + "-pp")) == *outMovie);
    }

}
