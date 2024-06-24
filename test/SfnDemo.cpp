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
    const std::string dataDir = isx::getAbsolutePath(g_resources["realTestDataPath"]) + "/prism_probe_P39";
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
    const std::string operation = "Import";
    const std::string params = "";
    isx::HistoricalDetails hd(operation, params);

    SECTION("Project for SfN demo - motion corrected nVista movies only")
    {
        const std::string fileName = dataDir + "/project-sfn_demo_mc.isxp";
        std::remove(fileName.c_str());
        isx::Project project(fileName, "SfN 2016 Demo (MC)");
        for (const auto & series : allSeries)
        {
            const isx::DataSet::Properties prop =
            {
                {isx::DataSet::PROP_DATA_MIN, isx::Variant(0.f)},
                {isx::DataSet::PROP_DATA_MAX, isx::Variant(4095.f)}
            };
            
            auto s = project.createSeriesInRoot(series.first);
            for (const auto & name : series.second)
            {
                const std::string movieName = name + "-mc";
                const std::string movieFile = procDataDir + "/" + movieName + "_he.isxd";
                REQUIRE(isx::pathExists(movieFile));
                auto us = std::make_shared<isx::Series>(
                        movieName,
                        isx::DataSet::Type::MOVIE,
                        movieFile,
                        hd,
                        prop
                );
                s->insertUnitarySeries(us);
            }
        }

        size_t b = 0;
        for (const auto & bms : behavioralMovies)
        {
            const std::string movieFile = behavioralDataDir + "/" + bms + ".mpg";
            REQUIRE(isx::pathExists(movieFile));
            const isx::DataSet::Properties prop =
            {
                {isx::DataSet::PROP_MOVIE_START_TIME, isx::Variant(behavioralStartTimes[b])}
            };
            project.importDataSetInRoot(bms, isx::DataSet::Type::BEHAVIOR, movieFile, hd,prop);
            ++b;
        }

        project.save();
    }

    SECTION("Project for SfN demo - DF/F nVista movies with ROI cell sets")
    {
        const std::string fileName = dataDir + "/project-sfn_demo_dff_rois.isxp";
        std::remove(fileName.c_str());
        isx::Project project(fileName, "SfN 2016 Demo");

        for (const auto & series : allSeries)
        {
            auto s = project.createSeriesInRoot(series.first);
            auto css = std::make_shared<isx::Series>("Manual ROIs");
            s->addChild(css);

            for (const auto & name : series.second)
            {
                const std::string movieName = name + "-mc_DFF";
                const std::string movieFile = procDataDir + "/" + movieName + "_he.isxd";
                REQUIRE(isx::pathExists(movieFile));
                const std::string cellName = name + "-rois_he.isxd";
                const std::string cellSetFile = procDataDir + "/" + cellName;
                REQUIRE(isx::pathExists(cellSetFile));
                const isx::DataSet::Properties prop =
                {
                    {isx::DataSet::PROP_DATA_MIN, isx::Variant(dffMin)},
                    {isx::DataSet::PROP_DATA_MAX, isx::Variant(dffMax)}
                };
                const isx::DataSet::Properties propCs;
                auto m = std::make_shared<isx::Series>(
                        movieName,
                        isx::DataSet::Type::MOVIE,
                        movieFile,
                        hd,
                        prop
                );
                s->insertUnitarySeries(m);
                auto cs = std::make_shared<isx::Series>(
                        cellName,
                        isx::DataSet::Type::CELLSET,
                        cellSetFile,
                        hd,
                        propCs);
                css->insertUnitarySeries(cs);

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
            const isx::DataSet::Properties prop =
            {
                {isx::DataSet::PROP_MOVIE_START_TIME, isx::Variant(behavioralStartTimes[b])}
            };
            project.importDataSetInRoot(bms, isx::DataSet::Type::BEHAVIOR, movieFile, hd,prop);
            ++b;
        }

        project.save();
    }

}

TEST_CASE("SfnDemoOrig", "[data][!hide]")
{
    const std::string dataDir = isx::getAbsolutePath(g_resources["realTestDataPath"]) + "/prism_probe_P39";
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
    const std::string operation = "Import";
    const std::string params = "";
    isx::HistoricalDetails hd(operation, params);

    SECTION("Project for SfN demo - original nVista movies only")
    {
        const std::string fileName = dataDir + "/project-sfn_demo_orig.isxp";
        std::remove(fileName.c_str());
        isx::Project project(fileName, "SfN 2016 Demo (Original)");

        for (const auto & series : allSeries)
        {
            project.createSeriesInRoot(series.first);
            const auto names = series.second;
            for (const auto & name : names)
            {
                const std::string movieFile = origDataDir + "/" + name + ".xml";
                REQUIRE(isx::pathExists(movieFile));
                const isx::DataSet::Properties prop =
                {
                    {isx::DataSet::PROP_DATA_MIN, isx::Variant(0.f)},
                    {isx::DataSet::PROP_DATA_MAX, isx::Variant(4095.f)}
                };
                auto us = std::make_shared<isx::Series>(
                        name,
                        isx::DataSet::Type::MOVIE,
                        movieFile,
                        hd,
                        prop
                );
            }
        }

        size_t b = 0;
        for (const auto & bms : behavioralMovies)
        {
            const std::string movieFile = behavioralDataDir + "/" + bms + ".mpg";
            REQUIRE(isx::pathExists(movieFile));
            const isx::DataSet::Properties prop =
            {
                {isx::DataSet::PROP_MOVIE_START_TIME, isx::Variant(behavioralStartTimes[b])}
            };
            project.importDataSetInRoot(bms, isx::DataSet::Type::BEHAVIOR, movieFile, hd,prop);
            ++b;
        }

        project.save();
    }

}
