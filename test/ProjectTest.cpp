#include "isxProject.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"
#include "isxPathUtils.h"

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
        REQUIRE_NOTHROW(project.getProcessedGroup());
        project.save();
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
        REQUIRE(dataSet->getFileName() == isx::getAbsolutePath(movieFileName));
        REQUIRE(project.getDataSet("/myDataSet") == dataSet);
    }

    SECTION("Open an existing project after adding a group.")
    {
        {
            isx::Project project(projectFileName, projectName);
            project.createGroup("/myGroup");
            project.save();
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
        std::string cellSetName = "ics";

        std::string origMoviePath = "/Original/ImagingData/" + origMovieName;
        std::string outMoviePath = "/Processed/ImagingData/" + outMovieName;
        std::string cellSetPath = "/Processed/CellData/" + cellSetName;

        std::string origMovieFileName = "/inscopix/data/" + origMovieName + ".isxd";
        std::string outMovieFileName = g_resources["testDataPath"] + "/" + outMovieName + ".isxd";
        std::string cellSetFileName = g_resources["testDataPath"] + "/" + cellSetName + ".isxd";

        {
            isx::Project project(projectFileName, projectName);
            project.createDataSet(origMoviePath, isx::DataSet::Type::MOVIE, origMovieFileName);
            project.createDataSet(outMoviePath, isx::DataSet::Type::MOVIE, outMovieFileName);
            project.createDataSet(cellSetPath, isx::DataSet::Type::CELLSET, cellSetFileName);
            project.save();
        }

        isx::Group rootGroup = isx::Group("/");
        isx::Group * origGroup = rootGroup.createGroup("Original");
        isx::Group * origImagingGroup = origGroup->createGroup("ImagingData");
        isx::Group * outGroup = rootGroup.createGroup("Processed");
        isx::Group * outImagingGroup = outGroup->createGroup("ImagingData");
        isx::Group * outCellGroup = outGroup->createGroup("CellData");

        isx::DataSet * expOrigMovie = origImagingGroup->createDataSet(
                origMovieName, isx::DataSet::Type::MOVIE, isx::getAbsolutePath(origMovieFileName));
        isx::DataSet * expOutMovie = outImagingGroup->createDataSet(
                outMovieName, isx::DataSet::Type::MOVIE, isx::getAbsolutePath(outMovieFileName));
        isx::DataSet * expCellSet = outCellGroup->createDataSet(
                cellSetName, isx::DataSet::Type::CELLSET, isx::getAbsolutePath(cellSetFileName));

        isx::Project project(projectFileName);
        REQUIRE(project.isValid());
        REQUIRE(*(project.getDataSet(origMoviePath)) == *expOrigMovie);
        REQUIRE(*(project.getDataSet(outMoviePath)) == *expOutMovie);
        REQUIRE(*(project.getDataSet(cellSetPath)) == *expCellSet);
    }

}

TEST_CASE("ProjectSynth", "[core][!hide]")
{

    SECTION("Create a synthetic project with a group of movies")
    {
        const std::string fileName = g_resources["testDataPath"] + "/project-synth_1.isxp";
        std::remove(fileName.c_str());

        const std::string groupPath = "/Original/ImagingData/Day_1";

        isx::Project project(fileName, "Synthetic Project 1");
        project.createGroup(groupPath);

        project.createDataSet(
                groupPath + "/recording_20160426_145041",
                isx::DataSet::Type::MOVIE,
                g_resources["testDataPath"] + "/recording_20160426_145041.hdf5");

        project.createDataSet(
                groupPath + "/recording_20160706_132714",
                isx::DataSet::Type::MOVIE,
                g_resources["testDataPath"] + "/recording_20160706_132714.hdf5");

        project.save();
    }

}
