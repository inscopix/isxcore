#include "isxProject.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"
#include "isxPathUtils.h"
#include "isxMovieFactory.h"
#include "isxCellSet.h"
#include "isxStopWatch.h"

#include <fstream>

TEST_CASE("Project-Project", "[core]")
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

    SECTION("Open an existing project after creating a data set.")
    {
        isx::Project project(projectFileName, projectName);
        const isx::DataSet * expected = project.createDataSet("/movie", isx::DataSet::Type::MOVIE, "movie.isxd");
        project.save();

        isx::Project readProject(projectFileName);

        REQUIRE(*readProject.getItem("/movie") == *expected);
    }

    SECTION("Open an existing project after creating a series.")
    {
        isx::Project project(projectFileName, projectName);
        const isx::Series * expected = project.createSeries("/series");
        project.save();

        isx::Project readProject(projectFileName);

        REQUIRE(*readProject.getItem("/series") == *expected);
    }

}

TEST_CASE("Project-createDataSet", "[core]")
{
    std::string projectFileName = g_resources["unitTestDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());
    std::string projectName = "myProject";
    isx::Project project(projectFileName, projectName);

    std::string movie1File, movie2File, movie3File,
            movie1OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File;
    createSeriesTestData(movie1File, movie2File, movie3File,
            movie1OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File);

    SECTION("Create a movie data set in a project")
    {
        isx::DataSet * dataSet = project.createDataSet("/myDataSet", isx::DataSet::Type::MOVIE, movie1File);

        REQUIRE(dataSet->getParent() == project.getRootGroup());
        REQUIRE(dataSet->getName() == "myDataSet");
        REQUIRE(dataSet->getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(dataSet->getPath() == "/myDataSet");
        REQUIRE(dataSet->getFileName() == isx::getAbsolutePath(movie1File));
        REQUIRE(project.getItem("/myDataSet") == dataSet);
    }

    SECTION("Create a movie in a series")
    {
        isx::Series * series = project.createSeries("/series");

        const isx::DataSet * movie1 = project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);

        REQUIRE(movie1->getParent() == series);
        REQUIRE(movie1->getName() == "movie1");
        REQUIRE(movie1->getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(movie1->getFileName() == isx::getAbsolutePath(movie1File));
        REQUIRE(movie1->getPath() == "/series/movie1");
    }

    SECTION("Create two consistent movies to the series in order")
    {
        isx::Series * series = project.createSeries("/series");

        const isx::DataSet * movie1 = project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);
        const isx::DataSet * movie2 = project.createDataSet("/series/movie2", isx::DataSet::Type::MOVIE, movie2File);

        REQUIRE(movie1->getParent() == series);
        REQUIRE(movie1->getName() == "movie1");
        REQUIRE(movie1->getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(movie1->getFileName() == isx::getAbsolutePath(movie1File));
        REQUIRE(movie1->getPath() == "/series/movie1");

        REQUIRE(movie2->getParent() == series);
        REQUIRE(movie2->getName() == "movie2");
        REQUIRE(movie2->getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(movie2->getFileName() == isx::getAbsolutePath(movie2File));
        REQUIRE(movie2->getPath() == "/series/movie2");
    }

    SECTION("Add two consistent movies to the series out of order")
    {
        isx::Series * series = project.createSeries("/series");

        const isx::DataSet * movie2 = project.createDataSet("/series/movie2", isx::DataSet::Type::MOVIE, movie2File);
        const isx::DataSet * movie1 = project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);

        const std::vector<isx::ProjectItem *> children = series->getChildren();
        REQUIRE(*static_cast<isx::DataSet *>(children.at(0)) == *movie1);
        REQUIRE(*static_cast<isx::DataSet *>(children.at(1)) == *movie2);
    }

    SECTION("Try to create two movies that have different spacing info")
    {
        isx::Series * series = project.createSeries("/series");

        project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);

        ISX_REQUIRE_EXCEPTION(
                project.createDataSet("/series/movie2Cropped", isx::DataSet::Type::MOVIE, movie2CroppedFile),
                isx::ExceptionSeries,
                "The spacing info is different than that of the reference.");
    }

    SECTION("Try to create two movies that have different frame rates")
    {
        isx::Series * series = project.createSeries("/series");

        project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);

        ISX_REQUIRE_EXCEPTION(
                project.createDataSet("/series/movie2Step2", isx::DataSet::Type::MOVIE, movie2Step2File),
                isx::ExceptionSeries,
                "The timing info has a different frame rate than the reference.");
    }

    SECTION("Try to create two movies with temporal overlap")
    {
        isx::Series * series = project.createSeries("/series");

        project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);

        ISX_REQUIRE_EXCEPTION(
                project.createDataSet("/series/movie1Overlap", isx::DataSet::Type::MOVIE, movie1OverlapFile),
                isx::ExceptionSeries,
                "The timing info temporally overlaps with the reference.");
    }

    SECTION("Try to add a behavioral movie to a series")
    {
        isx::Series * series = project.createSeries("/series");

        ISX_REQUIRE_EXCEPTION(
                project.createDataSet("/series/behavior", isx::DataSet::Type::BEHAVIOR, "behavior.mpg"),
                isx::ExceptionSeries,
                "A series can only contain nVista movies.");
    }

    SECTION("Try to add two movies with different pixel data types")
    {
        isx::Series * series = project.createSeries("/series");

        project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);

        ISX_REQUIRE_EXCEPTION(
                project.createDataSet("/series/movie2F32", isx::DataSet::Type::MOVIE, movie2F32File),
                isx::ExceptionSeries,
                "The data type is different than that of the reference.");
    }

}

TEST_CASE("Project-removeItem", "[core]")
{
    const std::string projectFileName = g_resources["unitTestDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());
    const std::string projectName = "myProject";

    isx::Project project(projectFileName, projectName);

    SECTION("Remove a movie data set in an otherwise empty project")
    {
        project.createDataSet("/movie", isx::DataSet::Type::MOVIE, "movie.isxd");

        project.removeItem("/movie");

        ISX_REQUIRE_EXCEPTION(
                project.getItem("/movie"),
                isx::ExceptionDataIO,
                "Could not find child with the name: movie")
    }

    SECTION("Remove one series in an empty project")
    {
        project.createSeries("/series");

        project.removeItem("/series");

        ISX_REQUIRE_EXCEPTION(
                project.getItem("/series"),
                isx::ExceptionDataIO,
                "Could not find child with the name: series")
    }

}

TEST_CASE("Project-moveItem", "[core]")
{
    std::string projectFileName = g_resources["unitTestDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());
    std::string projectName = "project";

    isx::Project project(projectFileName, projectName);

    std::string movie1File, movie2File, movie3File,
        movie1OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File;
    createSeriesTestData(movie1File, movie2File, movie3File,
            movie1OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File);

    SECTION("Identity move of a series")
    {
        project.createSeries("/series");

        project.moveItem("/series", "/");

        std::vector<isx::ProjectItem *> children = project.getItem("/")->getChildren();
        REQUIRE(project.getItem("/series") == children.at(0));
    }

    SECTION("Change the order of two series in a project")
    {
        project.createSeries("/series_1");
        project.createSeries("/series_2");

        project.moveItem("/series_1", "/", 1);

        std::vector<isx::ProjectItem *> children = project.getItem("/")->getChildren();
        REQUIRE(project.getItem("/series_2") == children.at(0));
        REQUIRE(project.getItem("/series_1") == children.at(1));
    }

    SECTION("Change the order of three series in a project")
    {
        project.createSeries("/series_1");
        project.createSeries("/series_2");
        project.createSeries("/series_3");

        project.moveItem("/series_2", "/", 0);

        std::vector<isx::ProjectItem *> children = project.getItem("/")->getChildren();
        REQUIRE(project.getItem("/series_2") == children.at(0));
        REQUIRE(project.getItem("/series_1") == children.at(1));
        REQUIRE(project.getItem("/series_3") == children.at(2));
    }

    SECTION("Move a data set into an empty series")
    {
        project.createDataSet("/movie_1", isx::DataSet::Type::MOVIE, movie1File);
        project.createSeries("/series");

        project.moveItem("/movie_1", "/series");

        std::vector<isx::ProjectItem *> children = project.getItem("/series")->getChildren();
        REQUIRE(project.getItem("/series/movie_1") == children.at(0));

        ISX_REQUIRE_EXCEPTION(
                project.getItem("/movie_1"),
                isx::ExceptionDataIO,
                "Could not find child with the name: movie_1");
    }

    SECTION("Move a consistent data set into an existing series")
    {
        project.createSeries("/series");

        project.createDataSet("/series/movie_1", isx::DataSet::Type::MOVIE, movie1File);
        project.createDataSet("/movie_2", isx::DataSet::Type::MOVIE, movie2File);

        project.moveItem("/movie_2", "/series");

        std::vector<isx::ProjectItem *> children = project.getItem("/series")->getChildren();
        REQUIRE(project.getItem("/series/movie_1") == children.at(0));
        REQUIRE(project.getItem("/series/movie_2") == children.at(1));

        ISX_REQUIRE_EXCEPTION(
                project.getItem("/movie_1"),
                isx::ExceptionDataIO,
                "Could not find child with the name: movie_1");

        ISX_REQUIRE_EXCEPTION(
                project.getItem("/movie_2"),
                isx::ExceptionDataIO,
                "Could not find child with the name: movie_2");
    }

    SECTION("Move a consistent data set into an existing series out of order")
    {
        project.createSeries("/series");

        project.createDataSet("/movie_1", isx::DataSet::Type::MOVIE, movie1File);
        project.createDataSet("/series/movie_2", isx::DataSet::Type::MOVIE, movie2File);

        project.moveItem("/movie_1", "/series");

        std::vector<isx::ProjectItem *> children = project.getItem("/series")->getChildren();
        REQUIRE(project.getItem("/series/movie_1") == children.at(0));
        REQUIRE(project.getItem("/series/movie_2") == children.at(1));

        ISX_REQUIRE_EXCEPTION(
                project.getItem("/movie_1"),
                isx::ExceptionDataIO,
                "Could not find child with the name: movie_1");

        ISX_REQUIRE_EXCEPTION(
                project.getItem("/movie_2"),
                isx::ExceptionDataIO,
                "Could not find child with the name: movie_2");
    }

    SECTION("Move a data set into a series with which it is not consistent")
    {
        project.createSeries("/series");

        const isx::DataSet * movie1 = project.createDataSet("/movie_1", isx::DataSet::Type::MOVIE, movie1File);
        project.createDataSet("/series/movie_2", isx::DataSet::Type::MOVIE, movie2Step2File);

        ISX_REQUIRE_EXCEPTION(
                project.moveItem("/movie_1", "/series"),
                isx::ExceptionSeries,
                "The timing info has a different frame rate than the reference.");

        REQUIRE(project.getItem("/movie_1") == movie1);
    }

}

TEST_CASE("Project-createSeries", "[core]")
{
    const std::string projectFileName = g_resources["unitTestDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());
    const std::string projectName = "myProject";

    isx::Project project(projectFileName, projectName);

    SECTION("Create one series in an empty project")
    {
        const isx::Series * series = project.createSeries("/series");
        REQUIRE(project.getItem("/series") == series);
    }

}

TEST_CASE("Project-flattenSeries", "[core]")
{
    std::string projectFileName = g_resources["unitTestDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());
    std::string projectName = "myProject";
    isx::Project project(projectFileName, projectName);

    std::string movie1File, movie2File, movie3File,
            movie1OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File;
    createSeriesTestData(movie1File, movie2File, movie3File,
            movie1OverlapFile, movie2Step2File, movie2CroppedFile, movie2F32File);

    SECTION("Flatten a project with only one series")
    {
        project.createSeries("/series");
        project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);
        project.createDataSet("/series/movie2", isx::DataSet::Type::MOVIE, movie2File);

        project.flattenSeries("/series");

        const std::vector<isx::ProjectItem *> children = project.getRootGroup()->getChildren();
        REQUIRE(project.getItem("/movie1") == children.at(0));
        REQUIRE(project.getItem("/movie2") == children.at(1));

        ISX_REQUIRE_EXCEPTION(
            project.getItem("/series"),
            isx::ExceptionDataIO,
            "Could not find child with the name: series");
    }

    SECTION("Flatten a project with one series in a data set sandwich")
    {
        isx::DataSet * movie0 = project.createDataSet("/movie0", isx::DataSet::Type::MOVIE, "movie0.isxd");
        project.createSeries("/series");
        project.createDataSet("/series/movie1", isx::DataSet::Type::MOVIE, movie1File);
        project.createDataSet("/series/movie2", isx::DataSet::Type::MOVIE, movie2File);
        isx::DataSet * behavior = project.createDataSet("/behavior", isx::DataSet::Type::BEHAVIOR, "behavior.mpg");

        project.flattenSeries("/series");

        const std::vector<isx::ProjectItem *> children = project.getRootGroup()->getChildren();
        REQUIRE(project.getItem("/movie0") == children.at(0));
        REQUIRE(project.getItem("/movie1") == children.at(1));
        REQUIRE(project.getItem("/movie2") == children.at(2));
        REQUIRE(project.getItem("/behavior") == children.at(3));

        ISX_REQUIRE_EXCEPTION(
                project.getItem("/series"),
                isx::ExceptionDataIO,
                "Could not find child with the name: series");
    }

}

TEST_CASE("Project-isModified", "[core]")
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
        project.createSeries("/series");
        project.moveItem("/movie", "/series");

        ISX_REQUIRE_EXCEPTION(
                project.createDataSet("/movie", isx::DataSet::Type::MOVIE, movieFileName),
                isx::ExceptionFileIO,
                "There is already a data set with the file name: " + isx::getAbsolutePath(movieFileName));

        ISX_REQUIRE_EXCEPTION(
                project.getItem("/movie"),
                isx::ExceptionDataIO,
                "Could not find child with the name: movie");
    }
}

TEST_CASE("Project-createUniquePath", "[core][!hide]")
{
    std::string projectFileName = g_resources["unitTestDataPath"] + "/project.isxp";
    std::remove(projectFileName.c_str());
    std::string projectName = "myProject";

    isx::Project project(projectFileName, projectName);

    SECTION("When path is not taken")
    {
        project.createSeries("series_000");
        const std::string path = project.createUniquePath("/series");
        REQUIRE(path == "/series");
    }

    SECTION("When path is taken once")
    {
        project.createSeries("series");
        const std::string path = project.createUniquePath("/series");
        REQUIRE(path == "/series_000");
    }

    SECTION("When path is taken twice")
    {
        project.createSeries("series");
        project.createSeries("series_000");
        const std::string path = project.createUniquePath("/series");
        REQUIRE(path == "/series_001");
    }
}

TEST_CASE("Project-createUniquePath_bench", "[core][!hide]")
{
    std::string projectFileName = g_resources["unitTestDataPath"] + "/projectWithLotsOfGroups.isxp";
    std::remove(projectFileName.c_str());
    std::string projectName = "myProject";

    isx::Project project(projectFileName, projectName);
    const isx::Series * group = project.createSeries("series");

    SECTION("MOS-469: time with lots of groups")
    {
        const size_t numGroups = 500;
        for (size_t i = 1; i < numGroups; ++i)
        {
            project.createSeries(isx::appendNumberToPath("series", i, 3));
        }
        float duration = 0.f;
        {
            isx::ScopedStopWatch timer(&duration);
            project.createUniquePath("/group");
        }
       ISX_LOG_INFO("Creating unique path after ", numGroups, " groups took ", duration, " ms.");
    }
}
