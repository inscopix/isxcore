#include "isxProject.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"

//#include <stdio.h>
#include <fstream>

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

    SECTION("Create a new project")
    {
        isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName, projectName);

        REQUIRE(project->isValid());
        REQUIRE(project->getName() == projectName);
        REQUIRE(project->getFileName() == projectFileName);
        REQUIRE(project->getGroup("/")->getGroups().size() == 2);
        REQUIRE_NOTHROW(project->getRootGroup());
        REQUIRE_NOTHROW(project->getOriginalGroup());
        REQUIRE_NOTHROW(project->getOutputGroup());
    }

    SECTION("Create a new project in a file that already exists")
    {
        {
            std::ofstream outStream(projectFileName);
            outStream << "testing";
        }

        try
        {
            isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName, projectName);
            FAIL("Failed to throw an exception.");
        }
        catch (isx::ExceptionFileIO & error)
        {
            REQUIRE(std::string(error.what()) ==
                    "The file name already exists: " + projectFileName);
        }
        catch (...)
        {
            FAIL("Failed to throw an isx::ExceptionFileIO");
        }
    }

    SECTION("Create a group in a project")
    {
        isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName, projectName);

        isx::SpGroup_t group = project->getRootGroup()->createGroup("myGroup");

        REQUIRE(group->getParent() == project->getRootGroup());
        REQUIRE(group->getName() == "myGroup");
    }

    SECTION("Create a movie data set in a project")
    {
        std::string movieFileName = g_resources["testDataPath"] + "/movie.isxp";
        isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName, projectName);

        isx::SpDataSet_t dataSet = project->createDataSet("/myDataSet",
                isx::DataSet::Type::MOVIE, movieFileName);

        REQUIRE(dataSet->getParent() == project->getRootGroup());
        REQUIRE(dataSet->getName() == "myDataSet");
        REQUIRE(dataSet->getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(dataSet->getPath() == "/myDataSet");
        REQUIRE(dataSet->getFileName() == movieFileName);
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
        std::string origMoviePath = "/Original/" + baseName;
        std::string outMoviePath = "/Output/" + baseName + "-pp";

        std::string origMovieFileName = "/inscopix/data/" + baseName + ".isxp";
        std::string outMovieFileName = g_resources["testDataPath"] + "/" + baseName + ".isxp";

        isx::SpDataSet_t origMovie, outMovie;
        {
            isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName, projectName);
            origMovie = project->createDataSet(origMoviePath,
                    isx::DataSet::Type::MOVIE, origMovieFileName);
            outMovie = project->createDataSet(outMoviePath,
                    isx::DataSet::Type::MOVIE, outMovieFileName);
        }

        isx::SpProject_t project = std::make_shared<isx::Project>(projectFileName);
        REQUIRE(project->isValid());
        REQUIRE(*(project->getOriginalGroup()->getDataSet(baseName)) == *origMovie);
        REQUIRE(*(project->getOutputGroup()->getDataSet(baseName + "-pp")) == *outMovie);
    }

}
