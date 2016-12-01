#include "isxProject.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"
#include "isxPathUtils.h"
#include "isxMovieFactory.h"
#include "isxCellSet.h"
#include "isxStopWatch.h"

#include <fstream>

TEST_CASE("ProjectTest", "[core]")
{
    std::string projectFileName = g_resources["unitTestDataPath"] + "/project.isxp";
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
        REQUIRE_NOTHROW(project.getRootGroup());
        REQUIRE_NOTHROW(project.getOriginalDataGroup());
        project.save();
    }

    SECTION("Create a new project in a file that already exists")
    {
        {
            std::ofstream outStream(projectFileName);
            outStream << "testing";
        }

        ISX_REQUIRE_EXCEPTION(
                isx::Project project(projectFileName, projectName),
                isx::ExceptionFileIO,
                "The file name already exists: " + projectFileName);
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
        std::string movieFileName = g_resources["unitTestDataPath"] + "/movie.isxp";
        isx::Project project(projectFileName, projectName);

        isx::DataSet * dataSet = project.createDataSet("/myDataSet",
                isx::DataSet::Type::MOVIE, movieFileName);

        REQUIRE(dataSet->getParent() == project.getGroup("/myDataSet"));
        REQUIRE(dataSet->getName() == "myDataSet");
        REQUIRE(dataSet->getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(dataSet->getPath() == "/myDataSet/myDataSet");
        REQUIRE(dataSet->getFileName() == isx::getAbsolutePath(movieFileName));
        REQUIRE(project.getDataSet("/myDataSet") == dataSet);
    }

    SECTION("Import a movie data set in a project")
    {
        std::string movieFileName = g_resources["unitTestDataPath"] + "/movie.isxp";
        isx::Project project(projectFileName, projectName);

        isx::DataSet * dataSet = project.createDataSet("/myDataSet",
                isx::DataSet::Type::MOVIE, movieFileName);

        REQUIRE(project.getDataSet("OriginalData/myDataSet") == dataSet);
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
        std::string movieName = baseName;
        std::string procMovieName = baseName + "-pp";
        std::string cellSetName = "ics";

        std::string moviePath = "/" + movieName;

        std::string procMoviePath = "/" + procMovieName;
        std::string procMovieDerivedPath = procMoviePath + "/derived";

        std::string cellSetPath = procMovieDerivedPath + "/" + cellSetName;

#if ISX_OS_WIN32
        std::string rootFileName = "C:/";
#else
        std::string rootFileName = "/";
#endif
        std::string movieFileName = rootFileName + "inscopix/data/" + movieName + ".isxd";
        std::string procMovieFileName = g_resources["unitTestDataPath"] + "/" + procMovieName + ".isxd";
        std::string cellSetFileName = g_resources["unitTestDataPath"] + "/" + cellSetName + ".isxd";

        {
            isx::Project project(projectFileName, projectName);
            project.importDataSet(moviePath, isx::DataSet::Type::MOVIE, movieFileName);
            project.createDataSet(procMoviePath, isx::DataSet::Type::MOVIE, procMovieFileName);
            project.createDataSet(cellSetPath, isx::DataSet::Type::CELLSET, cellSetFileName);
            project.save();
        }

        // Create expected groups/datasets
        isx::Group rootGroup("/");
        isx::Group origGroup("OriginalData");
        isx::Group * movieGroup = rootGroup.createGroup(movieName, isx::Group::Type::DATASET);
        isx::Group * procMovieGroup = rootGroup.createGroup(procMovieName, isx::Group::Type::DATASET);
        isx::Group * procMovieDerivedGroup = procMovieGroup->createGroup("derived", isx::Group::Type::DERIVED);
        isx::Group * procCellGroup = procMovieDerivedGroup->createGroup(cellSetName, isx::Group::Type::DATASET);

        isx::DataSet * expOrigMovie = origGroup.createDataSet(
                movieName, isx::DataSet::Type::MOVIE, isx::getAbsolutePath(movieFileName));
        isx::DataSet * expMovie = movieGroup->createDataSet(
                movieName, isx::DataSet::Type::MOVIE, isx::getAbsolutePath(movieFileName));
        isx::DataSet * expProcMovie = procMovieGroup->createDataSet(
                procMovieName, isx::DataSet::Type::MOVIE, isx::getAbsolutePath(procMovieFileName));
        isx::DataSet * expCellSet = procCellGroup->createDataSet(
                cellSetName, isx::DataSet::Type::CELLSET, isx::getAbsolutePath(cellSetFileName));

        isx::Project project(projectFileName);
        REQUIRE(project.isValid());
        REQUIRE(*(project.getDataSet(moviePath)) == *expMovie);
        REQUIRE(*(project.getOriginalDataGroup()->getDataSet(movieName)) == *expOrigMovie);
        REQUIRE(*(project.getDataSet(procMoviePath)) == *expProcMovie);
        REQUIRE(*(project.getDataSet(cellSetPath)) == *expCellSet);
    }

}

TEST_CASE("Project-isGroup", "[core]")
{
    std::string projectFileName = g_resources["unitTestDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());

    std::string projectName = "myProject";

    SECTION("Empty project")
    {
        isx::Project project(projectFileName, projectName);
        REQUIRE(project.isGroup("/"));
    }

    SECTION("Project with one group")
    {
        isx::Project project(projectFileName, projectName);
        project.createGroup("/Day_1");
        REQUIRE(project.isGroup("/Day_1"));
    }

    SECTION("Project with one sub-group")
    {
        isx::Project project(projectFileName, projectName);
        project.createGroup("/Animal_1");
        REQUIRE(project.isGroup("/Animal_1"));
        project.createGroup("/Animal_1/Day_1");
        REQUIRE(project.isGroup("/Animal_1/Day_1"));
    }
}

TEST_CASE("ProjectModificationTest", "[core]")
{
    std::string projectFileName = g_resources["unitTestDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());

    std::string projectName = "myProject";

    SECTION("Check that a project is not modified after creating it.")
    {
        isx::Project project(projectFileName, projectName);
        REQUIRE(!project.isModified());
    }

    SECTION("Check that a project is not modified after writing it")
    {
        isx::Project project(projectFileName, projectName);
        project.save();
        REQUIRE(!project.isModified());
    }

    SECTION("Check that a file is not written if there is no explicit save")
    {
        {
            isx::Project project(projectFileName, projectName);
        }
        REQUIRE(!isx::pathExists(projectFileName));
    }

    SECTION("Check that a project is not modified after importing.")
    {
        {
            isx::Project project(projectFileName, projectName);
            project.save();
        }
        isx::Project project(projectFileName);
        REQUIRE(!project.isModified());
    }

}

TEST_CASE("MOS-444", "[core]")
{
    const std::string projectFileName = g_resources["unitTestDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());
    const std::string projectName = "myProject";
    const std::string movieFileName = g_resources["unitTestDataPath"] + "/recording_20160426_145041.xml";

    SECTION("Create a data set, move it into a group, then create it again")
    {
        isx::Project project(projectFileName, projectName);
        project.createDataSet("/movie", isx::DataSet::Type::MOVIE, movieFileName);
        project.createGroup("/series", isx::Group::Type::SERIES);
        project.moveGroups({"/movie"}, "/series");

        ISX_REQUIRE_EXCEPTION(
                project.createDataSet("/movie", isx::DataSet::Type::MOVIE, movieFileName),
                isx::ExceptionFileIO,
                "There is already a data set with the file name: " + isx::getAbsolutePath(movieFileName));

        ISX_REQUIRE_EXCEPTION(
                project.getGroup("/movie"),
                isx::ExceptionDataIO,
                "Could not find group with the name: movie");
    }
}

// A series should only contain data set groups.
// In particular, we want to prevent nested series and prevent nesting of
// general groups. Note that we do allow a general group to contain a series.
TEST_CASE("Project-preventNestedSeries", "[core]")
{
    const std::string projectFileName = g_resources["unitTestDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());
    const std::string projectName = "myProject";
    const std::string movieFileName = g_resources["unitTestDataPath"] + "/recording_20160426_145041.xml";

    isx::Project project(projectFileName, projectName);

    SECTION("Create a series within a general group")
    {
        project.createGroup("/animal_1", isx::Group::Type::GENERAL);
        project.createGroup("/animal_1/series_1", isx::Group::Type::SERIES);
        project.createDataSet("/movie_1", isx::DataSet::Type::MOVIE, movieFileName);
        REQUIRE_NOTHROW(project.moveGroups({"/movie_1"}, "/animal_1/series_1"));
    }

    SECTION("Try to create a series within another series")
    {
        project.createGroup("/series_1", isx::Group::Type::SERIES);

        ISX_REQUIRE_EXCEPTION(
                project.createGroup("/series_1/subseries_2"),
                isx::ExceptionSeries,
                "A series group can only contain data set groups.");
    }
}

void
createSeriesTestData(
        std::string & outMovie1File,
        std::string & outMovie2File,
        std::string & outMovie1OverlapFile,
        std::string & outMovie2Step2File,
        std::string & outMovie2CroppedFile,
        std::string & outMovie2F32File,
        std::string & outBehavMovieFile)
{
    const isx::Time start1(2016, 11, 8, 9, 24, 55);
    const isx::Time start1Overlap(2016, 11, 8, 9, 24, 55, isx::DurationInSeconds(2, 20));
    const isx::Time start2(2016, 11, 8, 9, 34, 29);
    const isx::DurationInSeconds step1(1, 20);
    const isx::DurationInSeconds step2(1, 15);
    const isx::TimingInfo timingInfo1(start1, step1, 5);
    const isx::TimingInfo timingInfo1Overlap(start1Overlap, step1, 9);
    const isx::TimingInfo timingInfo2(start2, step1, 6);
    const isx::TimingInfo timingInfo2Step2(start2, step2, 6);

    const isx::SpacingInfo spacingInfo1(isx::SizeInPixels_t(3, 7));
    const isx::SpacingInfo spacingInfoCropped(isx::SizeInPixels_t(3, 4));

    const std::string dataPath = g_resources["unitTestDataPath"] + "/output";

    // Movies 1 and 2 form a consistent series
    outMovie1File = dataPath + "/movie1.isxd";
    isx::writeMosaicMovie(outMovie1File, timingInfo1, spacingInfo1, isx::DataType::U16);

    outMovie2File = dataPath + "/movie2.isxd";
    isx::writeMosaicMovie(outMovie2File, timingInfo2, spacingInfo1, isx::DataType::U16);

    // The same as movie2, but starts earlier so it temporally overlaps with movie1
    outMovie1OverlapFile = dataPath + "/movie1Overlap.isxd";
    isx::writeMosaicMovie(outMovie1OverlapFile, timingInfo1Overlap, spacingInfo1, isx::DataType::U16);

    // The same as movie2, but with a different frame rate
    outMovie2Step2File = dataPath + "/movie2Step2.isxd";
    isx::writeMosaicMovie(outMovie2Step2File, timingInfo2Step2, spacingInfo1, isx::DataType::U16);

    // The same as movie2, but with different spacing info
    outMovie2CroppedFile = dataPath + "/movie2Cropped.isxd";
    isx::writeMosaicMovie(outMovie2CroppedFile, timingInfo2, spacingInfoCropped, isx::DataType::U16);

    // The same as movie2, but with a different pixel data type
    outMovie2F32File = dataPath + "/movie2F32.isxd";
    isx::writeMosaicMovie(outMovie2F32File, timingInfo2, spacingInfo1, isx::DataType::F32);

    // A behavioral movie
    outBehavMovieFile = g_resources["unitTestDataPath"] + "/trial9_OneSec.mpg";
}

TEST_CASE("Project-createDataSetInSeries", "[core]")
{
    std::string projectFileName = g_resources["unitTestDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());
    std::string projectName = "myProject";

    // Define test files
    std::string movie1File, movie2File, movie1OverlapFile, movie2Step2File,
            movie2CroppedFile, movie2F32File, behavMovieFile;
    createSeriesTestData(movie1File, movie2File, movie1OverlapFile, movie2Step2File,
            movie2CroppedFile, movie2F32File, behavMovieFile);

    // Create project with a series group
    isx::Project project(projectFileName, projectName);
    isx::Group * series = project.createGroup("/series", isx::Group::Type::SERIES);

    SECTION("Add one movie to the series in order")
    {
        project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);
    }

    SECTION("Add two consistent movies to the series in order")
    {
        project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);
        project.createDataSet("/series/movie2", isx::DataSet::Type::MOVIE, movie2File);
    }

    SECTION("Add two consistent movies to the series out of order")
    {
        isx::DataSet * movie2 = project.createDataSet("/series/movie2", isx::DataSet::Type::MOVIE, movie2File);
        isx::DataSet * movie1 = project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);

        const std::vector<isx::Group *> groups = series->getGroups();
        REQUIRE(groups[0]->getDataSetFromGroup() == movie1);
        REQUIRE(groups[1]->getDataSetFromGroup() == movie2);
    }

    SECTION("Try to add two movies that have different spacing info")
    {
        project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);
        ISX_REQUIRE_EXCEPTION(
            project.createDataSet("/series/movie2Cropped", isx::DataSet::Type::MOVIE, movie2CroppedFile),
            isx::ExceptionSeries,
            "The spacing info is different than the reference.");
    }

    SECTION("Try to add two movies that have different frame rates")
    {
        project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);
        ISX_REQUIRE_EXCEPTION(
            project.createDataSet("/series/movie2Step2", isx::DataSet::Type::MOVIE, movie2Step2File),
            isx::ExceptionSeries,
            "The timing info has a different frame rate than the reference.");
    }

    SECTION("Try to add two movies with temporal overlap")
    {
        project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);
        ISX_REQUIRE_EXCEPTION(
            project.createDataSet("/series/movie1Overlap", isx::DataSet::Type::MOVIE, movie1OverlapFile),
            isx::ExceptionSeries,
            "The timing info temporally overlaps with the reference.");
    }

    SECTION("Try to add a behavioral movie to a series")
    {
        ISX_REQUIRE_EXCEPTION(
            project.createDataSet("/series/behavMovie", isx::DataSet::Type::BEHAVIOR, behavMovieFile),
            isx::ExceptionSeries,
            "A series can only contain nVista movies.");
    }

    SECTION("Try to add two movies with different pixel data types")
    {
        project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);
        ISX_REQUIRE_EXCEPTION(
            project.createDataSet("/series/movie2F32", isx::DataSet::Type::MOVIE, movie2F32File),
            isx::ExceptionSeries,
            "The data type is different than the reference.");
    }

}

TEST_CASE("Project-flattenGroup", "[core")
{
    std::string projectFileName = g_resources["unitTestDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());
    std::string projectName = "myProject";

    // Define test files
    std::string movie1File, movie2File, movie1OverlapFile, movie2Step2File,
            movie2CroppedFile, movie2F32File, behavMovieFile;
    createSeriesTestData(movie1File, movie2File, movie1OverlapFile, movie2Step2File,
            movie2CroppedFile, movie2F32File, behavMovieFile);

    // Define a project with some series
    isx::Project project(projectFileName, projectName);
    isx::Group * series1 = project.createGroup("/series1", isx::Group::Type::SERIES);
    isx::DataSet * movie1 = project.createDataSet("/series1/movie1", isx::DataSet::Type::MOVIE, movie1File);
    isx::DataSet * movie2 = project.createDataSet("/series1/movie2", isx::DataSet::Type::MOVIE, movie2File);
    isx::DataSet * behavMovie = project.createDataSet("/behavMovie", isx::DataSet::Type::BEHAVIOR, behavMovieFile);

    SECTION("Collapse one top level series")
    {
        project.flattenGroup("/series1");

        REQUIRE(project.getDataSet("/movie1") == movie1);
        REQUIRE(project.getDataSet("/movie2") == movie2);
        REQUIRE(project.getDataSet("/behavMovie") == behavMovie);

        ISX_REQUIRE_EXCEPTION(
            project.getGroup("/series1"),
            isx::ExceptionDataIO,
            "Could not find group with the name: series1");
    }

    SECTION("Try to flatten root")
    {
        ISX_REQUIRE_EXCEPTION(
            project.flattenGroup("/"),
            isx::ExceptionDataIO,
            "The root group cannot be flattened.");
    }
}

TEST_CASE("Project-createUniquePathBench", "[core][!hide]")
{
    std::string projectFileName = g_resources["unitTestDataPath"] + "/projectWithLotsOfGroups.isxp";
    std::remove(projectFileName.c_str());
    std::string projectName = "myProject";

    isx::Project project(projectFileName, projectName);
    isx::Group * group = project.createGroup("/group");

    SECTION("MOS-469: time with lots of groups")
    {
        const size_t numGroups = 500;
        for (size_t i = 1; i < numGroups; ++i)
        {
            project.createGroup(isx::appendNumberToPath("/group", i, 3));
        }
        float duration = 0.f;
        {
            isx::ScopedStopWatch timer(&duration);
            project.createUniquePath("/group");
        }
       ISX_LOG_INFO("Creating unique path after ", numGroups, " groups took ", duration, " ms.");
    }
}

TEST_CASE("ProjectSynth", "[data][!hide]")
{

    SECTION("Create a synthetic project with a group of movies")
    {
        const std::string fileName = g_resources["unitTestDataPath"] + "/project-synth_1.isxp";
        std::remove(fileName.c_str());

        isx::Project project(fileName, "Synthetic Project 1");

        project.importDataSet(
                "/recording_20160426_145041",
                isx::DataSet::Type::MOVIE,
                g_resources["unitTestDataPath"] + "/recording_20160426_145041.xml",
                {
                    {isx::DataSet::PROP_DATA_MIN, 0.f},
                    {isx::DataSet::PROP_DATA_MAX, 4095.f},
                    {isx::DataSet::PROP_VIS_MIN, 0.f},
                    {isx::DataSet::PROP_VIS_MAX, 1.f}
                });
        project.importDataSet(
                "/recording_20160706_132714",
                isx::DataSet::Type::MOVIE,
                g_resources["unitTestDataPath"] + "/recording_20160706_132714.xml",
                {
                    {isx::DataSet::PROP_DATA_MIN, 0.f},
                    {isx::DataSet::PROP_DATA_MAX, 4095.f},
                    {isx::DataSet::PROP_VIS_MIN, 0.f},
                    {isx::DataSet::PROP_VIS_MAX, 1.f}
                });
        project.createGroup("/Day 1", isx::Group::Type::SERIES);
        project.moveGroups({"/recording_20160426_145041", "/recording_20160706_132714"}, "/Day 1");

        project.importDataSet(
                "/synth_movie-3cells",
                isx::DataSet::Type::MOVIE,
                g_resources["unitTestDataPath"] + "/synth_movie-3cells.isxd",
                {
                    {isx::DataSet::PROP_DATA_MIN, 0.f},
                    {isx::DataSet::PROP_DATA_MAX, 4095.f},
                    {isx::DataSet::PROP_VIS_MIN, 0.f},
                    {isx::DataSet::PROP_VIS_MAX, 1.f}
                });

        project.createDataSet(
                "/synth_movie-3cells/derived/PCA-ICA",
                isx::DataSet::Type::CELLSET,
                g_resources["unitTestDataPath"] + "/synth_trace-3cells.isxd");

        project.save();
    }

}
