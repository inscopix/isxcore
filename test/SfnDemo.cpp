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

    std::map<std::string, std::vector<std::string>> allSeries;
    allSeries["/Day 4"] =
    {
        "recording_20150802_115907",
        "recording_20150802_120607",
        "recording_20150802_121307",
        "recording_20150802_122007"
    };
    allSeries["/Day 5"] =
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

    isx::Time behavioralStartTimes[2] =
    {
        isx::Time(2015, 8, 2, 11, 59, 7, isx::DurationInSeconds(161, 1000)),
        isx::Time(2015, 8, 3, 12, 1, 21, isx::DurationInSeconds(224, 1000))
    };

    const float dffMin = -0.2f;
    const float dffMax = 0.2f;
    isx::HistoricalDetails hd("Imported", "");

    SECTION("Project for SfN demo - motion corrected nVista movies only")
    {
        const std::string fileName = dataDir + "/project-sfn_demo_mc.isxp";
        std::remove(fileName.c_str());
        isx::Project project(fileName, "SfN 2016 Demo (MC)");

        for (const auto series : allSeries)
        {
            const std::string seriesPath = series.first;
            project.createSeries(seriesPath);
            for (const auto & name : series.second)
            {
                const std::string movieName = name + "-mc";
                const std::string movieFile = procDataDir + "/" + movieName + ".isxd";
                REQUIRE(isx::pathExists(movieFile));
                const std::string moviePath = seriesPath + "/" + movieName;
                project.createDataSet(
                        moviePath,
                        isx::DataSet::Type::MOVIE,
                        movieFile,
                        hd,
                        {
                            {isx::DataSet::PROP_DATA_MIN, isx::Variant(0.f)},
                            {isx::DataSet::PROP_DATA_MAX, isx::Variant(4095.f)},
                            {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                            {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                        }
                );
            }
        }

        size_t b = 0;
        for (const auto & bms : behavioralMovies)
        {
            const std::string movieFile = behavioralDataDir + "/" + bms + ".mpg";
            REQUIRE(isx::pathExists(movieFile));
            const std::string moviePath = "/" + bms;
            project.createDataSet(moviePath, isx::DataSet::Type::BEHAVIOR, movieFile, hd,
                    {
                        {isx::DataSet::PROP_MOVIE_START_TIME, isx::Variant(behavioralStartTimes[b])}
                    }
            );
            ++b;
        }

        project.save();
    }

    SECTION("Project for SfN demo - DF/F nVista movies with ROI cell sets")
    {
        const std::string fileName = dataDir + "/project-sfn_demo_dff_rois.isxp";
        std::remove(fileName.c_str());
        isx::Project project(fileName, "SfN 2016 Demo");

        for (const auto series : allSeries)
        {
            const std::string seriesPath = series.first;
            project.createSeries(seriesPath);
            for (const auto & name : series.second)
            {
                const std::string movieName = name + "-mc_DFF";
                const std::string movieFile = procDataDir + "/" + movieName + ".isxd";
                REQUIRE(isx::pathExists(movieFile));
                const std::string moviePath = seriesPath + "/" + movieName;
                const std::string cellSetFile = procDataDir + "/" + name + "-rois.isxd";
                REQUIRE(isx::pathExists(cellSetFile));
                project.createDataSet(
                        moviePath,
                        isx::DataSet::Type::MOVIE,
                        movieFile,
                        hd,
                        {
                            {isx::DataSet::PROP_DATA_MIN, isx::Variant(dffMin)},
                            {isx::DataSet::PROP_DATA_MAX, isx::Variant(dffMax)},
                            {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                            {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                        }
                );
                project.createDataSet(
                        moviePath + "/Manual ROIs",
                        isx::DataSet::Type::CELLSET,
                        cellSetFile,
                        hd);

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

        size_t b = 0;
        for (const auto & bms : behavioralMovies)
        {
            const std::string movieFile = behavioralDataDir + "/" + bms + ".mpg";
            REQUIRE(isx::pathExists(movieFile));
            const std::string moviePath = "/" + bms;
            project.createDataSet(moviePath, isx::DataSet::Type::BEHAVIOR, movieFile, hd,
                    {
                        {isx::DataSet::PROP_MOVIE_START_TIME, isx::Variant(behavioralStartTimes[b])}
                    }
            );
            ++b;
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

    std::map<std::string, std::vector<std::string>> allSeries;
    allSeries["Day 4"] =
    {
        "recording_20150802_115907",
        "recording_20150802_120607",
        "recording_20150802_121307",
        "recording_20150802_122007"
    };
    allSeries["Day 5"] =
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

    isx::Time behavioralStartTimes[2] =
    {
        isx::Time(2015, 8, 2, 11, 59, 7, isx::DurationInSeconds(161, 1000)),
        isx::Time(2015, 8, 3, 12, 1, 21, isx::DurationInSeconds(224, 1000))
    };

    const float dffMin = -0.2f;
    const float dffMax = 0.2f;
    isx::HistoricalDetails hd("Imported", "");

    SECTION("Project for SfN demo - original nVista movies only")
    {
        const std::string fileName = dataDir + "/project-sfn_demo_orig.isxp";
        std::remove(fileName.c_str());
        isx::Project project(fileName, "SfN 2016 Demo (Original)");

        for (const auto series : allSeries)
        {
            const std::string seriesPath = series.first;
            project.createSeries(seriesPath);
            const auto names = series.second;
            for (const auto & name : names)
            {
                const std::string movieFile = origDataDir + "/" + name + ".xml";
                REQUIRE(isx::pathExists(movieFile));
                const std::string moviePath = seriesPath + "/" + name;
                project.createDataSet(
                        moviePath,
                        isx::DataSet::Type::MOVIE,
                        movieFile,
                        hd,
                        {
                            {isx::DataSet::PROP_DATA_MIN, isx::Variant(0.f)},
                            {isx::DataSet::PROP_DATA_MAX, isx::Variant(4095.f)},
                            {isx::DataSet::PROP_VIS_MIN, isx::Variant(0.f)},
                            {isx::DataSet::PROP_VIS_MAX, isx::Variant(1.f)}
                        }
                );
            }
        }

        size_t b = 0;
        for (const auto & bms : behavioralMovies)
        {
            const std::string movieFile = behavioralDataDir + "/" + bms + ".mpg";
            const std::string moviePath = "/" + bms;
            project.createDataSet(moviePath, isx::DataSet::Type::BEHAVIOR, movieFile, hd,
                    {
                        {isx::DataSet::PROP_MOVIE_START_TIME, isx::Variant(behavioralStartTimes[b])}
                    }
            );
            ++b;
        }

        project.save();
    }

}
