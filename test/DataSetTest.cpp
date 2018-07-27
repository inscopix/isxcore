#include "isxDataSet.h"
#include "isxTest.h"
#include "isxException.h"
#include "isxGroup.h"
#include "isxProject.h"
#include "isxMovieFactory.h"
#include "isxCellSetFactory.h"
#include "isxVariant.h"
#include "isxPathUtils.h"

#include "catch.hpp"

TEST_CASE("DataSet-DataSet", "[core]")
{
    isx::HistoricalDetails hd("mainTest", "");
    isx::HistoricalDetails hdd("derivedTest", "");

    SECTION("Empty constructor")
    {
        isx::DataSet dataSet;

        REQUIRE(!dataSet.isValid());
    }

    SECTION("Construct a movie data set with no properties")
    {
        isx::DataSet dataSet("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd", hd);

        REQUIRE(dataSet.isValid());
        REQUIRE(dataSet.getType() == isx::DataSet::Type::MOVIE);
        REQUIRE(dataSet.getName() == "myMovie");
        REQUIRE(dataSet.getHistoricalDetails() == hd);
        REQUIRE(dataSet.getProperties() == isx::DataSet::Properties());
    }

    SECTION("Construct a movie data set with some properties")
    {
        isx::DataSet::Properties properties;
        properties["test"] = isx::Variant(1.0f);
        isx::DataSet dataSet("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd", hd, properties);
        isx::Variant value;
        REQUIRE(dataSet.getPropertyValue("test", value));
        REQUIRE(value.value<float>() == 1.0f);
    }

    SECTION("Construct a cell set data set")
    {
        isx::DataSet dataSet("myCellSet", isx::DataSet::Type::CELLSET, "myCellSet.isxd", hdd);

        REQUIRE(dataSet.isValid());
        REQUIRE(dataSet.getType() == isx::DataSet::Type::CELLSET);
        REQUIRE(dataSet.getName() == "myCellSet");
        REQUIRE(dataSet.getHistoricalDetails() == hdd);
    }

}

TEST_CASE("DataSet-setName", "[core]")
{
    isx::HistoricalDetails hd("mainTest", "");
    isx::DataSet dataSet("myDataSet", isx::DataSet::Type::MOVIE, "movie.isxd", hd);

    SECTION("Set the name of a data set")
    {
        dataSet.setName("myMovie");

        REQUIRE(dataSet.getName() == "myMovie");
    }
}

TEST_CASE("readDataSetType", "[core]")
{

    std::string fileName = g_resources["unitTestDataPath"] + "/myDataSet.isxd";
    std::remove(fileName.c_str());

    isx::Time start(2016, 8, 26, 10, 31, 26, isx::DurationInSeconds(117, 1000));
    isx::DurationInSeconds step(50, 1000);
    isx::isize_t numTimes = 5;
    isx::TimingInfo timingInfo(start, step, numTimes);

    isx::SizeInPixels_t numPixels(3, 4);
    isx::SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
    isx::PointInMicrons_t topLeft(0, 0);
    isx::SpacingInfo spacingInfo(numPixels, pixelSize, topLeft);

    SECTION("Movie")
    {
        auto m(isx::writeMosaicMovie(fileName, timingInfo, spacingInfo, isx::DataType::U16));
        m->closeForWriting();

        isx::DataSet::Type type = isx::readDataSetType(fileName, {});

        REQUIRE(type == isx::DataSet::Type::MOVIE);
    }

}

TEST_CASE("DataSet-toFromJsonString", "[core]")
{
    isx::HistoricalDetails hd("mainTest", "");
    isx::HistoricalDetails hdd("derivedTest", "");

    SECTION("One data set with some properties")
    {
        isx::DataSet::Properties properties;
        properties["test"] = isx::Variant(1.0f);
        isx::DataSet expected("myMovie", isx::DataSet::Type::MOVIE, "myMovie.isxd", hd, properties);

        const std::string jsonString = expected.toJsonString();
        std::shared_ptr<isx::DataSet> actual = isx::DataSet::fromJsonString(jsonString);

        REQUIRE(*actual == expected);
    }
}

// Utility functions for the following tests.
namespace
{

isx::SpDataSet_t
makeMovieDataSet(
        const std::string & inName,
        const std::string & inFilePath,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        const bool inImported = false)
{
    std::remove(inFilePath.c_str());
    const isx::SpWritableMovie_t movie = isx::writeMosaicMovie(inFilePath, inTimingInfo, inSpacingInfo, isx::DataType::U16);
    movie->closeForWriting();
    const isx::HistoricalDetails history;
    const isx::DataSet::Properties properties;
    return std::make_shared<isx::DataSet>(inName, isx::DataSet::Type::MOVIE, inFilePath, history, properties, inImported);
}

std::string
makeDeleteFilesFilePath(const std::string & inName)
{
    return g_resources["unitTestDataPath"] + "/DataSet-deleteFiles-" + inName + ".isxd";
}

} // namespace

TEST_CASE("DataSet-deleteFile", "[core]")
{
    const isx::TimingInfo ti(isx::Time(), isx::DurationInSeconds(50, 1000), 4);
    const isx::SpacingInfo si(isx::SizeInPixels_t(2, 3));
    const std::string filePath = makeDeleteFilesFilePath("movie");

    SECTION("Delete an imported dataset file")
    {
        isx::SpDataSet_t ds = makeMovieDataSet("movie", filePath, ti, si, true);
        ds->deleteFile();
        REQUIRE(isx::pathExists(ds->getFileName()));
    }

    SECTION("Delete an non-imported dataset file")
    {
        isx::SpDataSet_t ds = makeMovieDataSet("movie", filePath, ti, si, false);
        ds->deleteFile();
        REQUIRE(!isx::pathExists(ds->getFileName()));
    }

    std::remove(filePath.c_str());
}

namespace
{

std::map<std::string, std::string>
convertMetadataToMap(const isx::DataSet::Metadata & inMetadata)
{
    std::map<std::string, std::string> map;
    for (const auto & p : inMetadata)
    {
        map[p.first] = p.second;
    }
    return map;
}

} // namespace

TEST_CASE("DataSet-getMetadata", "[core]")
{
    isx::CoreInitialize();

    SECTION("nVista 2 movie")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/recording_20161104_145443.xml";
        isx::DataSet ds("movie", isx::DataSet::Type::MOVIE, filePath, isx::HistoricalDetails());

        const std::map<std::string, std::string> metaData = convertMetadataToMap(ds.getMetadata());

        REQUIRE(metaData.at("Start Time") == "2016/11/04-14:54:43.662");
        REQUIRE(metaData.at("End Time") == "2016/11/04-14:54:46.328");
        REQUIRE(metaData.at("Duration (s)") == "2.667");
        REQUIRE(metaData.at("Sample Rate (Hz)") == "15.000");
        REQUIRE(metaData.at("Number of Time Samples") == "40");
        REQUIRE(metaData.at("Number of Dropped Samples") == "1");
        REQUIRE(metaData.at("Dropped Samples") == "10 ");
        REQUIRE(metaData.at("Number of Cropped Samples") == "0");
        REQUIRE(metaData.at("Number of Pixels") == "1440 x 1080");

    }

    SECTION("nVoke 1 movie")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/nVoke/recording_20170130_165221.xml";
        isx::DataSet ds("movie", isx::DataSet::Type::MOVIE, filePath, isx::HistoricalDetails());

        const std::map<std::string, std::string> metaData = convertMetadataToMap(ds.getMetadata());

        REQUIRE(metaData.at("Start Time") == "2017/01/30-16:52:21.754");
        REQUIRE(metaData.at("End Time") == "2017/01/30-16:52:21.903");
        REQUIRE(metaData.at("Duration (s)") == "0.150");
        REQUIRE(metaData.at("Sample Rate (Hz)") == "20.010");
        REQUIRE(metaData.at("Number of Time Samples") == "3");
        REQUIRE(metaData.at("Number of Dropped Samples") == "0");
        REQUIRE(metaData.at("Number of Cropped Samples") == "0");
        REQUIRE(metaData.at("Number of Pixels") == "1440 x 1080");

        REQUIRE(metaData.at("Acquisition SW Version") == "2.1.8-20161128-093054");
        REQUIRE(metaData.at("Exposure (ms)") == "49.664");
        REQUIRE(metaData.at("Gain") == "1.0");
        REQUIRE(metaData.at("LED Power") == "0.20");
        REQUIRE(metaData.at("Recording Schedule Name") == "[]");
        REQUIRE(metaData.at("Total Time LED was ON in Session") == "00:00");
    }

    SECTION("nVista 3 movie")
    {
        const std::string filePath = g_resources["unitTestDataPath"] + "/nVista3Gpio/2018-06-21-17-51-03_video_sched_0.isxd";
        isx::DataSet ds("movie", isx::DataSet::Type::MOVIE, filePath, isx::HistoricalDetails());

        const std::map<std::string, std::string> metaData = convertMetadataToMap(ds.getMetadata());

        REQUIRE(metaData.at("Start Time") == "2018/06/21-17:51:03.965");
        REQUIRE(metaData.at("End Time") == "2018/06/21-17:51:05.005");
        REQUIRE(metaData.at("Duration (s)") == "1.041");
        REQUIRE(metaData.at("Sample Rate (Hz)") == "12.490");
        REQUIRE(metaData.at("Number of Time Samples") == "13");
        REQUIRE(metaData.at("Number of Dropped Samples") == "0");
        REQUIRE(metaData.at("Number of Cropped Samples") == "0");
        REQUIRE(metaData.at("Number of Pixels") == "1280 x 800");

        REQUIRE(metaData.at("Animal Sex") == "m");
        REQUIRE(metaData.at("Animal Date of Birth") == "");
        REQUIRE(metaData.at("Animal ID") == "");
        REQUIRE(metaData.at("Animal Species") == "");
        REQUIRE(metaData.at("Animal Weight") == "0");

        REQUIRE(metaData.at("Microscope Binning Mode") == "1");
        REQUIRE(metaData.at("Microscope Focus") == "0");
        REQUIRE(metaData.at("Microscope Gain") == "7");
        REQUIRE(metaData.at("Microscope DI LED Power") == "0.200");
        REQUIRE(metaData.at("Microscope EX LED Power") == "0");
        REQUIRE(metaData.at("Microscope OG LED Power") == "0");
        REQUIRE(metaData.at("Microscope Sensor Mode") == "master");
        REQUIRE(metaData.at("Microscope Serial Number") == "unknown");
        REQUIRE(metaData.at("Microscope Type") == "nVista");

        REQUIRE(metaData.at("Session Name") == "Session 20180621-174314");

        REQUIRE(metaData.at("Experimenter Name") == "John Doe");

        REQUIRE(metaData.at("Probe Diameter (mm)") == "0");
        REQUIRE(metaData.at("Probe Flip") == "none");
        REQUIRE(metaData.at("Probe ID") == "none");
        REQUIRE(metaData.at("Probe Length (mm)") == "0");
        REQUIRE(metaData.at("Probe Name") == "None");
        REQUIRE(metaData.at("Probe Pitch") == "0");
        REQUIRE(metaData.at("Probe Rotation") == "0");
        REQUIRE(metaData.at("Probe Type") == "None");

        REQUIRE(metaData.at("Acquisition SW Version (BE)") == "1.1.0-4453328");
        REQUIRE(metaData.at("Acquisition SW Version (FE)") == "1.1.0-ae0e21a");
    }

    isx::CoreShutdown();
}
