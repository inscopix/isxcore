#include "isxProject.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"

#include <fstream>

TEST_CASE("ProjectTest", "[core]")
{
    std::string projectFileName = g_resources["testDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());

    std::string projectName = "myProject";

    SECTION("Empty constructor")
    {
        isx::Project project;
        REQUIRE(!project.isValid());
    }

    SECTION("Create a new project")
    {
        isx::Project project(projectFileName, projectName);

        REQUIRE(project.isValid());
        REQUIRE(project.getName() == projectName);
        REQUIRE(project.getFileName() == projectFileName);
        REQUIRE(project.getGroup("/")->getGroups().size() == 2);
        REQUIRE_NOTHROW(project.getRootGroup());
        REQUIRE_NOTHROW(project.getOriginalGroup());
        REQUIRE_NOTHROW(project.getOutputGroup());
    }

    SECTION("Create a new project in a file that already exists")
    {
        {
            std::ofstream outStream(projectFileName);
            outStream << "testing";
        }

        try
        {
            isx::Project project(projectFileName, projectName);
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
        isx::Project project(projectFileName, projectName);

        isx::Group * group = project.createGroup("/myGroup");

        REQUIRE(group->getParent() == project.getRootGroup());
        REQUIRE(group->getName() == "myGroup");
        REQUIRE(group->getPath() == "/myGroup");
        REQUIRE(project.getGroup("/myGroup") == group);
    }

    SECTION("Create a movie data set in a project")
    {
        std::string movieFileName = g_resources["testDataPath"] + "/movie.isxp";
        isx::Project project(projectFileName, projectName);

        isx::DataSet * dataSet = project.createDataSet("/myDataSet",
                isx::DataSet::Type::MOVIE, movieFileName);

        REQUIRE(dataSet->getParent() == project.getRootGroup());
        REQUIRE(dataSet->getName() == "myDataSet");
        REQUIRE(dataSet->getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(dataSet->getPath() == "/myDataSet");
        REQUIRE(dataSet->getFileName() == movieFileName);
        REQUIRE(project.getDataSet("/myDataSet") == dataSet);
    }

    SECTION("Open an existing project after adding a group.")
    {
        {
            isx::Project project(projectFileName, projectName);
            project.createGroup("/myGroup");
        }
        isx::Project project(projectFileName);
        REQUIRE(project.isValid());
        isx::Group * group = project.getGroup("/myGroup");
        REQUIRE(group->getName() == "myGroup");
        REQUIRE(group->getPath() == "/myGroup");
        REQUIRE(*(group->getParent()) == isx::Group("/"));
    }

    SECTION("Open an existing project after adding some typical groups and data sets")
    {
        std::string baseName = "recording-20160808-133943";
        std::string origMovieName = baseName;
        std::string outMovieName = baseName + "-pp";
        std::string origMoviePath = "/Original/" + origMovieName;
        std::string outMoviePath = "/Output/" + outMovieName;

        std::string origMovieFileName = "/inscopix/data/" + origMovieName + ".isxp";
        std::string outMovieFileName = g_resources["testDataPath"] + "/" + outMovieName + ".isxp";

        {
            isx::Project project(projectFileName, projectName);
            project.createDataSet(origMoviePath, isx::DataSet::Type::MOVIE, origMovieFileName);
            project.createDataSet(outMoviePath, isx::DataSet::Type::MOVIE, outMovieFileName);
        }

        isx::Group rootGroup = isx::Group("/");
        isx::Group * origGroup = rootGroup.createGroup("Original");
        isx::Group * outGroup = rootGroup.createGroup("Output");

        isx::DataSet * expOrigMovie = origGroup->createDataSet(origMovieName, isx::DataSet::Type::MOVIE, origMovieFileName);
        isx::DataSet * expOutMovie = outGroup->createDataSet(outMovieName, isx::DataSet::Type::MOVIE, outMovieFileName);

        isx::Project project(projectFileName);
        REQUIRE(project.isValid());
        REQUIRE(*(project.getDataSet(origMoviePath)) == *expOrigMovie);
        REQUIRE(*(project.getDataSet(outMoviePath)) == *expOutMovie);
    }

}
