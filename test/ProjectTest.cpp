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

    SECTION("Open an existing project after creating a unitary series.")
    {
        isx::Project project(projectFileName, projectName);
        isx::HistoricalDetails hd("mainTest", "");
        const auto expected = project.importDataSetInRoot(
            "movie",
            isx::DataSet::Type::MOVIE,
            g_resources["unitTestDataPath"] + "/recording_20160426_145041-dff_he.isxd",
            hd);
        project.save();

        isx::Project readProject(projectFileName);
        auto pi = readProject.getRootGroup()->getGroupMember(0);
        auto s = static_cast<isx::Series *>(pi);
        auto readDs = s->getDataSet(0);
        auto ds = expected->getDataSet(0);
        
        REQUIRE(readDs->getName() == ds->getName());
        REQUIRE(isx::getFileName(readDs->getFileName()) == isx::getFileName(ds->getFileName()));
        REQUIRE(ds->isImported());
        REQUIRE(readDs->isImported());
    }

    SECTION("Open an existing project after creating a series.")
    {
        isx::Project project(projectFileName, projectName);
        isx::HistoricalDetails hd("mainTest", "");
        const auto expected = project.createSeriesInRoot("Series");
        project.save();

        isx::Project readProject(projectFileName);
        auto pi = readProject.getRootGroup()->getGroupMember(0);
        auto s = static_cast<isx::Series *>(pi);

        REQUIRE(*expected == *s);
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
    isx::HistoricalDetails hd;
    isx::DataSet::Properties prop;
    
    auto movie1Series = std::make_shared<isx::Series>("movie1", isx::DataSet::Type::MOVIE, movie1File, hd, prop);
    auto movie2Series = std::make_shared<isx::Series>("movie2", isx::DataSet::Type::MOVIE, movie2File, hd, prop);
    auto movie3Series = std::make_shared<isx::Series>("movie3", isx::DataSet::Type::MOVIE, movie3File, hd, prop);
    auto movie1OverlapSeries = std::make_shared<isx::Series>("movie1Overlap", isx::DataSet::Type::MOVIE, movie1OverlapFile, hd, prop);
    auto movie2Step2Series = std::make_shared<isx::Series>("movie2Step2", isx::DataSet::Type::MOVIE, movie2Step2File, hd, prop);
    auto movie2CroppedSeries = std::make_shared<isx::Series>("movie2Cropped", isx::DataSet::Type::MOVIE, movie2CroppedFile, hd, prop);
    auto movie2F32Series = std::make_shared<isx::Series>("movie2F32", isx::DataSet::Type::MOVIE, movie2F32File, hd, prop);
    
    SECTION("Flatten a series in a project with no other members")
    {
        auto s = project.createSeriesInRoot("series");
        s->insertUnitarySeries(movie1Series);
        s->insertUnitarySeries(movie2Series);
        
        project.flattenSeries(s.get());
        auto rg = project.getRootGroup();
        const auto members = rg->getGroupMembers();

        REQUIRE(members.size() == 2);
        REQUIRE(members.at(0) == static_cast<isx::ProjectItem *>(movie1Series.get()));
        REQUIRE(members.at(1) == static_cast<isx::ProjectItem *>(movie2Series.get()));
    }

    SECTION("Flatten a series in a project where it is sandwiched between two unitary series")
    {
        // Project layout:
        //
        // movie1
        // series
        //     movie2
        //     movie3
        // movie2F32
        //
        
        auto rg = project.getRootGroup();
        rg->insertGroupMember(movie1Series, 0);
        auto s = project.createSeriesInRoot("series");
        rg->insertGroupMember(movie2F32Series, rg->getNumGroupMembers());
        s->insertUnitarySeries(movie2Series);
        s->insertUnitarySeries(movie3Series);

        project.flattenSeries(s.get());

        const auto members = rg->getGroupMembers();
        REQUIRE(members.size() == 4);
        
        REQUIRE(members.at(0) == static_cast<isx::ProjectItem *>(movie1Series.get()));
        REQUIRE(members.at(1) == static_cast<isx::ProjectItem *>(movie2Series.get()));
        REQUIRE(members.at(2) == static_cast<isx::ProjectItem *>(movie3Series.get()));
        REQUIRE(members.at(3) == static_cast<isx::ProjectItem *>(movie2F32Series.get()));
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
    isx::HistoricalDetails hd("movie1", "");
    isx::DataSet::Properties prop;

    SECTION("Create a data set, move it into a group, then create it again")
    {
        isx::Project project(projectFileName, projectName);
        
        auto ps1 = std::make_shared<isx::Series>("movie", isx::DataSet::Type::MOVIE, movieFileName, hd, prop);
        auto s = project.createSeriesInRoot("series");
        s->insertUnitarySeries(ps1);

        ISX_REQUIRE_EXCEPTION(
                project.importDataSetInRoot(
                    "movie-alt",
                    isx::DataSet::Type::MOVIE,
                    movieFileName,
                    isx::HistoricalDetails()),
                isx::ExceptionFileIO,
                "There is already a data set with the file name: " + isx::getAbsolutePath(movieFileName));
        
        const auto members = project.getRootGroup()->getGroupMembers();
        REQUIRE(members.size() == 1);
        REQUIRE(members.at(0) == static_cast<isx::ProjectItem *>(s.get()));
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
        project.createSeriesInRoot("series_000");
        const std::string path = project.createUniquePath("/series");
        REQUIRE(path == "/series");
    }

    SECTION("When path is taken once")
    {
        project.createSeriesInRoot("series");
        const std::string path = project.createUniquePath("/series");
        REQUIRE(path == "/series_000");
    }

    SECTION("When path is taken twice")
    {
        project.createSeriesInRoot("series");
        project.createSeriesInRoot("series_000");
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
    const auto s = project.createSeriesInRoot("series");

    SECTION("MOS-469: time with lots of groups")
    {
        const size_t numGroups = 500;
        for (size_t i = 1; i < numGroups; ++i)
        {
            project.createSeriesInRoot(isx::appendNumberToPath("series", i, 3));
        }
        float duration = 0.f;
        {
            isx::ScopedStopWatch timer(&duration);
            project.createUniquePath("series");
        }
       ISX_LOG_INFO("Creating unique path after ", numGroups, " groups took ", duration, " ms.");
    }
}
