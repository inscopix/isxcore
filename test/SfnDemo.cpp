#include "isxProject.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"
#include "isxPathUtils.h"
#include "isxMovieFactory.h"
#include "isxCellSetFactory.h"
#include "isxCellSet.h"

#include <fstream>

TEST_CASE("SfnDemo", "[data][!hide]")
{
    const std::string dataDir = g_resources["realTestDataPath"] + "/prism_probe_P39";
    const std::string origDataDir = dataDir + "/orig";
    const std::string procDataDir = dataDir + "/proc";
    const std::string behavioralDataDir = dataDir + "/noldus";

    std::map<std::string, std::vector<std::string>> groups;
    groups["/Day 4"] =
    {
        "recording_20150802_115907",
        "recording_20150802_120607",
        "recording_20150802_121307",
        "recording_20150802_122007"
    };
    groups["/Day 5"] =
    {
        "recording_20150803_120121",
        "recording_20150803_120821",
        "recording_20150803_121521",
        "recording_20150803_122221"
    };

    std::string behavioralMovies[2] =
    {
        "Trial     9",
        "Trial    11"
    };

    const float dffMin = -0.2f;
    const float dffMax = 0.2f;

    SECTION("Project for SfN demo - motion corrected nVista movies only")
    {
        const std::string fileName = dataDir + "/project-sfn_demo_mc.isxp";
        std::remove(fileName.c_str());
        isx::Project project(fileName, "SfN 2016 Demo (MC)");

        for (const auto group : groups)
        {
            const std::string groupPath = group.first;
            project.createGroup(groupPath, isx::Group::Type::SERIES);
            for (const auto & name : group.second)
            {
                const std::string movieName = name + "-mc";
                const std::string movieFile = procDataDir + "/" + movieName + ".isxd";
                REQUIRE(isx::pathExists(movieFile));
                const std::string moviePath = groupPath + "/" + movieName;
                project.createDataSet(
                        moviePath,
                        isx::DataSet::Type::MOVIE,
                        movieFile,
                        {
                            {isx::DataSet::PROP_DATA_MIN, isx::Variant(0.f)},
                            {isx::DataSet::PROP_DATA_MAX, isx::Variant(4095.f)},
                            {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                            {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                        }
                );
            }
        }

        for (const auto & bms : behavioralMovies)
        {
            const std::string movieFile = behavioralDataDir + "/" + bms + ".mpg";
            REQUIRE(isx::pathExists(movieFile));
            const std::string moviePath = "/" + bms;
            project.createDataSet(moviePath, isx::DataSet::Type::BEHAVIOR, movieFile);
        }

        project.save();
    }

    SECTION("Project for SfN demo - DF/F nVista movies with ROI cell sets")
    {
        const std::string fileName = dataDir + "/project-sfn_demo_dff_rois.isxp";
        std::remove(fileName.c_str());
        isx::Project project(fileName, "SfN 2016 Demo");

        for (const auto group : groups)
        {
            const std::string groupPath = group.first;
            project.createGroup(groupPath, isx::Group::Type::SERIES);
            for (const auto & name : group.second)
            {
                const std::string movieName = name + "-mc_DFF";
                const std::string movieFile = procDataDir + "/" + movieName + ".isxd";
                REQUIRE(isx::pathExists(movieFile));
                const std::string moviePath = groupPath + "/" + movieName;
                const std::string cellSetFile = procDataDir + "/" + name + "-rois.isxd";
                REQUIRE(isx::pathExists(cellSetFile));
                project.createDataSet(
                        moviePath,
                        isx::DataSet::Type::MOVIE,
                        movieFile,
                        {
                            {isx::DataSet::PROP_DATA_MIN, isx::Variant(dffMin)},
                            {isx::DataSet::PROP_DATA_MAX, isx::Variant(dffMax)},
                            {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                            {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                        }
                );
                project.createDataSet(
                        moviePath + "/derived/Manual ROIs",
                        isx::DataSet::Type::CELLSET,
                        cellSetFile);

                // NOTE sweet : also check that timing/spacing info is consistent
                if (isx::pathExists(movieFile) && isx::pathExists(cellSetFile))
                {
                    isx::SpMovie_t movie = isx::readMovie(movieFile);
                    auto cellSet = isx::readCellSet(cellSetFile);
                    REQUIRE(movie->getTimingInfo() == cellSet->getTimingInfo());
                    REQUIRE(movie->getSpacingInfo() == cellSet->getSpacingInfo());
                }
            }
        }

        for (const auto & bms : behavioralMovies)
        {
            const std::string movieFile = behavioralDataDir + "/" + bms + ".mpg";
            REQUIRE(isx::pathExists(movieFile));
            const std::string moviePath = "/" + bms;
            project.createDataSet(moviePath, isx::DataSet::Type::BEHAVIOR, movieFile);
        }

        project.save();
    }

    SECTION("Project for SfN demo - DF/F nVista movies with PCA/ICA cell sets")
    {
        const std::string fileName = dataDir + "/project-sfn_demo_dff_pcaica.isxp";
        std::remove(fileName.c_str());
        isx::Project project(fileName, "SfN 2016 Demo");
        
        for (const auto group : groups)
        {
            const std::string groupPath = group.first;
            project.createGroup(groupPath, isx::Group::Type::SERIES);
            for (const auto & name : group.second)
            {
                const std::string movieName = name + "-mc_DFF";
                const std::string movieFile = procDataDir + "/" + movieName + ".isxd";
                REQUIRE(isx::pathExists(movieFile));
                const std::string moviePath = groupPath + "/" + movieName;
                const std::string cellSetFile = procDataDir + "/" + name + "-pca_ica.isxd";
                REQUIRE(isx::pathExists(cellSetFile));
                project.createDataSet(
                                      moviePath,
                                      isx::DataSet::Type::MOVIE,
                                      movieFile,
                                      {
                                          {isx::DataSet::PROP_DATA_MIN, isx::Variant(dffMin)},
                                          {isx::DataSet::PROP_DATA_MAX, isx::Variant(dffMax)},
                                          {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                                          {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                                      }
                                      );
                project.createDataSet(
                                      moviePath + "/derived/PCA-ICA",
                                      isx::DataSet::Type::CELLSET,
                                      cellSetFile);
                
                // NOTE sweet : also check that timing/spacing info is consistent
                if (isx::pathExists(movieFile) && isx::pathExists(cellSetFile))
                {
                    isx::SpMovie_t movie = isx::readMovie(movieFile);
                    auto cellSet = isx::readCellSet(cellSetFile);
                    REQUIRE(movie->getTimingInfo() == cellSet->getTimingInfo());
                    REQUIRE(movie->getSpacingInfo() == cellSet->getSpacingInfo());
                }
            }
        }
        
        for (const auto & bms : behavioralMovies)
        {
            const std::string movieFile = behavioralDataDir + "/" + bms + ".mpg";
            REQUIRE(isx::pathExists(movieFile));
            const std::string moviePath = "/" + bms;
            project.createDataSet(moviePath, isx::DataSet::Type::BEHAVIOR, movieFile);
        }
        
        project.save();
    }
    
}

TEST_CASE("SfnDemoOrig", "[data][!hide]")
{
    const std::string dataDir = g_resources["realTestDataPath"] + "/prism_probe_P39";
    const std::string origDataDir = dataDir + "/orig";
    const std::string procDataDir = dataDir + "/proc";
    const std::string behavioralDataDir = dataDir + "/noldus";

    std::map<std::string, std::vector<std::string>> groups;
    groups["/Day 4"] =
    {
        "recording_20150802_115907",
        "recording_20150802_120607",
        "recording_20150802_121307",
        "recording_20150802_122007"
    };
    groups["/Day 5"] =
    {
        "recording_20150803_120121",
        "recording_20150803_120821",
        "recording_20150803_121521",
        "recording_20150803_122221"
    };

    std::string behavioralMovies[2] =
    {
        "Trial     9",
        "Trial    11"
    };

    const float dffMin = -0.2f;
    const float dffMax = 0.2f;

    SECTION("Project for SfN demo - original nVista movies only")
    {
        const std::string fileName = dataDir + "/project-sfn_demo_orig.isxp";
        std::remove(fileName.c_str());
        isx::Project project(fileName, "SfN 2016 Demo (Original)");

        for (const auto group : groups)
        {
            const std::string groupPath = group.first;
            project.createGroup(groupPath, isx::Group::Type::SERIES);
            const auto names = group.second;
            for (const auto & name : names)
            {
                const std::string movieFile = origDataDir + "/" + name + ".xml";
                REQUIRE(isx::pathExists(movieFile));
                const std::string moviePath = groupPath + "/" + name;
                project.createDataSet(
                        moviePath,
                        isx::DataSet::Type::MOVIE,
                        movieFile,
                        {
                            {isx::DataSet::PROP_DATA_MIN, isx::Variant(0.f)},
                            {isx::DataSet::PROP_DATA_MAX, isx::Variant(4095.f)},
                            {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                            {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                        }
                );
            }
        }

        for (const auto & bms : behavioralMovies)
        {
            const std::string movieFile = behavioralDataDir + "/" + bms + ".mpg";
            const std::string moviePath = "/" + bms;
            project.createDataSet(moviePath, isx::DataSet::Type::BEHAVIOR, movieFile);
        }

        project.save();
    }

    SECTION("Project for SfN demo - everything")
    {
        const std::string fileName = dataDir + "/project-sfn_demo.isxp";
        std::remove(fileName.c_str());
        isx::Project project(fileName, "SfN 2016 Demo");

        for (const auto group : groups)
        {
            // Original movies
            {
                const std::string groupPath = group.first;
                project.createGroup(groupPath, isx::Group::Type::SERIES);
                for (const auto & name : group.second)
                {
                    const std::string movieFile = origDataDir + "/" + name + ".xml";
                    REQUIRE(isx::pathExists(movieFile));
                    const std::string moviePath = groupPath + "/" + name;
                    project.createDataSet(
                            moviePath,
                            isx::DataSet::Type::MOVIE,
                            movieFile,
                            {
                              {isx::DataSet::PROP_DATA_MIN, isx::Variant(0.f)},
                              {isx::DataSet::PROP_DATA_MAX, isx::Variant(4095.f)},
                              {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                              {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                            }
                    );
                }
            }

            // Motion corrected movies
            {
                const std::string groupPath = group.first + " MC";
                project.createGroup(groupPath, isx::Group::Type::SERIES);
                for (const auto & name : group.second)
                {
                    const std::string movieName = name + "-mc";
                    const std::string movieFile = procDataDir + "/" + movieName + ".isxd";
                    REQUIRE(isx::pathExists(movieFile));
                    const std::string moviePath = groupPath + "/" + movieName;
                    project.createDataSet(
                            moviePath,
                            isx::DataSet::Type::MOVIE,
                            movieFile,
                            {
                              {isx::DataSet::PROP_DATA_MIN, isx::Variant(0.f)},
                              {isx::DataSet::PROP_DATA_MAX, isx::Variant(4095.f)},
                              {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                              {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                            }
                    );
                }
            }

            // DFF movies with cell set
            {
                const std::string groupPath = group.first + " DFF";
                project.createGroup(groupPath, isx::Group::Type::SERIES);
                for (const auto & name : group.second)
                {
                    const std::string movieName = name + "-mc_DFF";
                    const std::string movieFile = procDataDir + "/" + movieName + ".isxd";
                    REQUIRE(isx::pathExists(movieFile));

                    const std::string moviePath = groupPath + "/" + movieName;
                    const std::string cellSetFile = procDataDir + "/" + name + "-rois.isxd";
                    REQUIRE(isx::pathExists(cellSetFile));
                    project.createDataSet(
                                          moviePath,
                                          isx::DataSet::Type::MOVIE,
                                          movieFile,
                                          {
                                              {isx::DataSet::PROP_DATA_MIN, isx::Variant(dffMin)},
                                              {isx::DataSet::PROP_DATA_MAX, isx::Variant(dffMax)},
                                              {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                                              {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                                          });
                    project.createDataSet(
                                          moviePath + "/derived/Manual ROIs",
                                          isx::DataSet::Type::CELLSET,
                                          cellSetFile);

                    // NOTE sweet : also check that timing/spacing info is consistent
                    if (isx::pathExists(movieFile) && isx::pathExists(cellSetFile))
                    {
                        isx::SpMovie_t movie = isx::readMovie(movieFile);
                        auto cellSet = isx::readCellSet(cellSetFile);
                        REQUIRE(movie->getTimingInfo() == cellSet->getTimingInfo());
                        REQUIRE(movie->getSpacingInfo() == cellSet->getSpacingInfo());
                    }
                }
            }
        }
        
        for (const auto & bms : behavioralMovies)
        {
            const std::string movieFile = behavioralDataDir + "/" + bms + ".mpg";
            REQUIRE(isx::pathExists(movieFile));
            const std::string moviePath = "/" + bms;
            project.createDataSet(moviePath, isx::DataSet::Type::BEHAVIOR, movieFile);
        }
        
        project.save();
    }

    SECTION("Project for SfN demo - everything 2")
    {
        const std::string fileName = dataDir + "/project-sfn_demo_2.isxp";
        std::remove(fileName.c_str());
        isx::Project project(fileName, "SfN 2016 Demo 2");

        // Original movies
        project.createGroup("/Original");
        for (const auto group : groups)
        {
            const std::string groupPath = "/Original/" + group.first;
            project.createGroup(groupPath, isx::Group::Type::SERIES);
            for (const auto & name : group.second)
            {
                const std::string movieFile = origDataDir + "/" + name + ".xml";
                REQUIRE(isx::pathExists(movieFile));
                const std::string moviePath = groupPath + "/" + name;
                project.createDataSet(
                                      moviePath,
                                      isx::DataSet::Type::MOVIE,
                                      movieFile,
                                      {
                                          {isx::DataSet::PROP_DATA_MIN, isx::Variant(0.f)},
                                          {isx::DataSet::PROP_DATA_MAX, isx::Variant(4095.f)},
                                          {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                                          {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                                      }
                                      );
            }
        }

        // Motion corrected movies
        project.createGroup("/Motion Corrected");
        for (const auto group : groups)
        {
            const std::string groupPath = "/Motion Corrected/" + group.first;
            project.createGroup(groupPath, isx::Group::Type::SERIES);
            for (const auto & name : group.second)
            {
                const std::string movieName = name + "-mc";
                const std::string movieFile = procDataDir + "/" + movieName + ".isxd";
                REQUIRE(isx::pathExists(movieFile));
                const std::string moviePath = groupPath + "/" + movieName;
                project.createDataSet(
                                      moviePath,
                                      isx::DataSet::Type::MOVIE,
                                      movieFile,
                                      {
                                          {isx::DataSet::PROP_DATA_MIN, isx::Variant(0.f)},
                                          {isx::DataSet::PROP_DATA_MAX, isx::Variant(4095.f)},
                                          {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                                          {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                                      }
                                      );
            }
        }

        // DFF movies with cell set
        project.createGroup("/Normalized");
        for (const auto group : groups)
        {
            const std::string groupPath = "/Normalized/" + group.first;
            project.createGroup(groupPath, isx::Group::Type::SERIES);
            for (const auto & name : group.second)
            {
                const std::string movieName = name + "-mc_DFF";
                const std::string movieFile = procDataDir + "/" + movieName + ".isxd";
                REQUIRE(isx::pathExists(movieFile));
                const std::string moviePath = groupPath + "/" + movieName;
                const std::string cellSetFile = procDataDir + "/" + name + "-rois.isxd";
                REQUIRE(isx::pathExists(cellSetFile));
                project.createDataSet(
                                      moviePath,
                                      isx::DataSet::Type::MOVIE,
                                      movieFile,
                                      {
                                          {isx::DataSet::PROP_DATA_MIN, isx::Variant(dffMin)},
                                          {isx::DataSet::PROP_DATA_MAX, isx::Variant(dffMax)},
                                          {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                                          {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                                      });
                project.createDataSet(
                                      moviePath + "/derived/Manual ROIs",
                                      isx::DataSet::Type::CELLSET,
                                      cellSetFile);

                // NOTE sweet : also check that timing/spacing info is consistent
                if (isx::pathExists(movieFile) && isx::pathExists(cellSetFile))
                {
                    isx::SpMovie_t movie = isx::readMovie(movieFile);
                    auto cellSet = isx::readCellSet(cellSetFile);
                    REQUIRE(movie->getTimingInfo() == cellSet->getTimingInfo());
                    REQUIRE(movie->getSpacingInfo() == cellSet->getSpacingInfo());
                }
            }
        }
        
        project.createGroup("/Behavioral");
        for (const auto & bms : behavioralMovies)
        {
            const std::string movieFile = behavioralDataDir + "/" + bms + ".mpg";
            REQUIRE(isx::pathExists(movieFile));
            const std::string moviePath = "/Behavioral/" + bms;
            project.createDataSet(moviePath, isx::DataSet::Type::BEHAVIOR, movieFile);
        }
        
        project.save();
    }

}

