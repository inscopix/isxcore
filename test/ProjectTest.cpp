#include "isxProject.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"
#include "isxPathUtils.h"
#include "isxMovieFactory.h"
#include "isxCellSet.h"

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

        ISX_EXPECT_EXCEPTION();
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
        std::string movieFileName = g_resources["unitTestDataPath"] + "/movie.isxp";
        isx::Project project(projectFileName, projectName);

        isx::DataSet * dataSet = project.createDataSet("/myDataSet",
                isx::DataSet::Type::MOVIE, movieFileName);

        REQUIRE(dataSet->getParent() == project.getGroup("/myDataSet"));
        REQUIRE(dataSet->getName() == "myDataSet");
        REQUIRE(dataSet->getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(dataSet->getPath() == "/myDataSet/myDataSet");
        REQUIRE(dataSet->getFileName() == isx::getAbsolutePath(movieFileName));
        REQUIRE(project.getDataSet("/myDataSet/myDataSet") == dataSet);
    }

    SECTION("Import a movie data set in a project")
    {
        std::string movieFileName = g_resources["unitTestDataPath"] + "/movie.isxp";
        isx::Project project(projectFileName, projectName);

        isx::DataSet * dataSet = project.createDataSet("/myDataSet",
                isx::DataSet::Type::MOVIE, movieFileName);

        REQUIRE(project.getDataSet("OriginalData/myDataSet/myDataSet") == dataSet);
        REQUIRE(project.getDataSet("/myDataSet/myDataSet") == dataSet);
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

        std::string movieGroupPath = "/" + movieName;
        std::string moviePath = movieGroupPath + "/" + movieName;

        std::string procMovieGroupPath = "/" + procMovieName;
        std::string procMoviePath = procMovieGroupPath + "/" + procMovieName;
        std::string procMovieDerivedPath = procMovieGroupPath + "/derived";

        std::string cellSetGroupPath = procMovieDerivedPath + "/" + cellSetName;
        std::string cellSetPath = cellSetGroupPath + "/" + cellSetName;

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
            project.importDataSet(movieGroupPath, isx::DataSet::Type::MOVIE, movieFileName);
            project.createDataSet(procMovieGroupPath, isx::DataSet::Type::MOVIE, procMovieFileName);
            project.createDataSet(cellSetGroupPath, isx::DataSet::Type::CELLSET, cellSetFileName);
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

TEST_CASE("ProjectSynth", "[data][!hide]")
{

    SECTION("Create a synthetic project with a group of movies")
    {
        const std::string fileName = g_resources["unitTestDataPath"] + "/project-synth_1.isxp";
        std::remove(fileName.c_str());

        const std::string groupPath = "/Day_1";

        isx::Project project(fileName, "Synthetic Project 1");
        project.createGroup(groupPath);

        project.importDataSet(
                groupPath + "/recording_20160426_145041",
                isx::DataSet::Type::MOVIE,
                g_resources["unitTestDataPath"] + "/recording_20160426_145041.hdf5",
                {
                    {isx::DataSet::PROP_DATA_MIN, 0.f},
                    {isx::DataSet::PROP_DATA_MAX, 4095.f},
                    {isx::DataSet::PROP_VIS_MIN, 0.f},
                    {isx::DataSet::PROP_VIS_MAX, 1.f}
                });

        project.importDataSet(
                groupPath + "/synth_movie-3cells",
                isx::DataSet::Type::MOVIE,
                g_resources["unitTestDataPath"] + "/synth_movie-3cells.isxd",
                {
                    {isx::DataSet::PROP_DATA_MIN, 0.f},
                    {isx::DataSet::PROP_DATA_MAX, 4095.f},
                    {isx::DataSet::PROP_VIS_MIN, 0.f},
                    {isx::DataSet::PROP_VIS_MAX, 1.f}
                });

        project.createDataSet(
                "/Day_1/synth_movie-3cells/derived/PCA-ICA",
                isx::DataSet::Type::CELLSET,
                g_resources["unitTestDataPath"] + "/synth_trace-3cells.isxd");

        project.save();
    }

    SECTION("Create a synthetic project for the sfn 2016 demo - mc files")
    {
        const std::string dataDir = g_resources["realTestDataPath"] + "/prism_probe_P39";
        const std::string procDataDir = dataDir + "/proc";
        const std::string behavioralDataDir = dataDir + "/noldus";
        const std::string fileName = dataDir + "/project-sfn_demo_mc.isxp";
        std::remove(fileName.c_str());

        std::map<std::string, std::vector<std::string>> groups;
        groups["/Day_4"] = {
            "recording_20150802_115907",
            "recording_20150802_120607",
            "recording_20150802_121307",
            "recording_20150802_122007"
        };
        groups["/Day_5"] = {
            "recording_20150803_120121",
            "recording_20150803_120821",
            "recording_20150803_121521",
            "recording_20150803_122221"
        };

        isx::Project project(fileName, "SfN 2016 Demo Project");

        for (auto groupIt = groups.begin(); groupIt != groups.end(); ++groupIt)
        {
            const std::string groupPath = groupIt->first;
            project.createGroup(groupPath);
            const auto names = groupIt->second;
            for (auto nameIt = names.begin(); nameIt != names.end(); ++nameIt)
            {
                const std::string movieName = *nameIt + "-mc";
                const std::string movieFile = procDataDir + "/" + movieName + ".isxd";
                const std::string moviePath = groupPath + "/" + movieName;
                const std::string cellSetFile = procDataDir + "/" + *nameIt + "-pca_ica.isxd";
                project.createDataSet(
                        moviePath,
                        isx::DataSet::Type::MOVIE,
                        movieFile,
                        {
                          {isx::DataSet::PROP_DATA_MIN, 0.f},
                          {isx::DataSet::PROP_DATA_MAX, 4095.f},
                          {isx::DataSet::PROP_VIS_MIN, 0.f},
                          {isx::DataSet::PROP_VIS_MAX, 1.f}
                        });
                project.createDataSet(
                        moviePath + "/derived/PCA-ICA",
                        isx::DataSet::Type::CELLSET,
                        cellSetFile);

                // NOTE sweet : also check that timing/spacing info is consistent
                isx::SpMovie_t movie = isx::readMovie(movieFile);
                auto cellSet = std::make_shared<isx::CellSet>(cellSetFile);
                REQUIRE(movie->getTimingInfo() == cellSet->getTimingInfo());
                REQUIRE(movie->getSpacingInfo() == cellSet->getSpacingInfo());
            }
        }

        std::string behavioralMovies[2] = {
            "Trial     9",
            "Trial    11" };
        
        for (auto bms: behavioralMovies)
        {
            const std::string movieFile = behavioralDataDir + "/" + bms + ".mpg";
            const std::string moviePath = "/" + bms;
            project.createDataSet(moviePath, isx::DataSet::Type::BEHAVIOR, movieFile);
        }

        project.save();
    }

    SECTION("Create a synthetic project for the sfn 2016 demo - dff files")
    {
        const std::string dataDir = g_resources["realTestDataPath"] + "/prism_probe_P39";
        const std::string procDataDir = dataDir + "/proc";
        const std::string behavioralDataDir = dataDir + "/noldus";
        const std::string fileName = dataDir + "/project-sfn_demo_dff.isxp";
        std::remove(fileName.c_str());
        
        std::map<std::string, std::vector<std::string>> groups;
        groups["/Day_4"] = {
            "recording_20150802_115907",
            "recording_20150802_120607",
            "recording_20150802_121307",
            "recording_20150802_122007"
        };
        groups["/Day_5"] = {
            "recording_20150803_120121",
            "recording_20150803_120821",
            "recording_20150803_121521",
            "recording_20150803_122221"
        };
        
        isx::Project project(fileName, "SfN 2016 Demo Project");
        
        for (auto groupIt = groups.begin(); groupIt != groups.end(); ++groupIt)
        {
            const std::string groupPath = groupIt->first;
            project.createGroup(groupPath);
            const auto names = groupIt->second;
            for (auto nameIt = names.begin(); nameIt != names.end(); ++nameIt)
            {
                const std::string movieName = *nameIt + "-mc_DFF";
                const std::string movieFile = procDataDir + "/" + movieName + ".isxd";
                const std::string moviePath = groupPath + "/" + movieName;
                const std::string cellSetFile = procDataDir + "/" + *nameIt + "-pca_ica.isxd";
                project.createDataSet(
                                      moviePath,
                                      isx::DataSet::Type::MOVIE,
                                      movieFile,
                                      {
                                          {isx::DataSet::PROP_DATA_MIN, -0.3f},
                                          {isx::DataSet::PROP_DATA_MAX, 0.5f},
                                          {isx::DataSet::PROP_VIS_MIN, 0.f},
                                          {isx::DataSet::PROP_VIS_MAX, 1.f}
                                      });
                project.createDataSet(
                                      moviePath + "/derived/PCA-ICA",
                                      isx::DataSet::Type::CELLSET,
                                      cellSetFile);
                
                // NOTE sweet : also check that timing/spacing info is consistent
                isx::SpMovie_t movie = isx::readMovie(movieFile);
                auto cellSet = std::make_shared<isx::CellSet>(cellSetFile);
                REQUIRE(movie->getTimingInfo() == cellSet->getTimingInfo());
                REQUIRE(movie->getSpacingInfo() == cellSet->getSpacingInfo());
            }
        }

        std::string behavioralMovies[2] = {
            "Trial     9",
            "Trial    11" };
        
        for (auto bms: behavioralMovies)
        {
            const std::string movieFile = behavioralDataDir + "/" + bms + ".mpg";
            const std::string moviePath = "/" + bms;
            project.createDataSet(moviePath, isx::DataSet::Type::BEHAVIOR, movieFile);
        }

        project.save();
    }
}
