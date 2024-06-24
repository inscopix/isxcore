#include "isxMovieFactory.h"
#include "isxImage.h"
#include "isxGpioImporter.h"
#include "isxPathUtils.h"
#include "isxTest.h"
#include "catch.hpp"
#include "isxLog.h"

namespace
{

void
requireSnapshopProperties(
        const std::string & inFilePath,
        const isx::SpacingInfo & inExpectedSi,
        const isx::TimingInfo & inExpectedTi,
        const std::map<size_t, uint16_t> & inExpectedValues
)
{
    const isx::SpVideoFrame_t frame = isx::readImage(inFilePath);
    REQUIRE(frame->getTimeStamp() == inExpectedTi.getStart());
    REQUIRE(frame->getImage().getSpacingInfo() == inExpectedSi);

    const isx::Image & image = frame->getImage();
    REQUIRE(image.getDataType() == isx::DataType::U16);
    const uint16_t * pixels = image.getPixelsAsU16();
    for (const auto & value : inExpectedValues)
    {
        REQUIRE(pixels[value.first] == value.second);
    }

    const isx::SpMovie_t movie = isx::readMovie(inFilePath);
    REQUIRE(movie->getSpacingInfo() == image.getSpacingInfo());
    REQUIRE(movie->getTimingInfo() == inExpectedTi);
}

} // namespace

TEST_CASE("nVista_PrismC1AAV1", "[core][snapshot]")
{
    const std::string basePath = g_resources["unitTestDataPath"] + "/Snapshots/nVista_PrismC1AAV1_tif/snapshot_20160613_102103";

    const isx::SpacingInfo expectedSi = isx::SpacingInfo(
            isx::SizeInPixels_t(1440, 1080),
            isx::SizeInMicrons_t(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE),
            isx::PointInMicrons_t(0, 0)
    );

    const std::map<size_t, uint16_t> expectedValues =
    {
        {0, 270},
        {1, 272},
        {1440, 273},
        {1440 * 1080 - 1, 237},
    };

    isx::CoreInitialize();

    SECTION("XML")
    {
        requireSnapshopProperties(
                basePath + ".xml",
                expectedSi,
                isx::TimingInfo(isx::Time(2016, 6, 13, 10, 21, 3, isx::DurationInSeconds(962, 1000)), isx::DurationInSeconds(50, 1000), 1),
                expectedValues
        );
    }

    SECTION("TIFF")
    {
        requireSnapshopProperties(
                basePath + ".tif",
                expectedSi,
                isx::TimingInfo(isx::Time(), isx::TimingInfo::s_defaultStep, 1),
                expectedValues
        );
    }

    isx::CoreShutdown();
}


TEST_CASE("nVista_ratPFC2", "[core][snapshot]")
{
    const std::string basePath = g_resources["unitTestDataPath"] + "/Snapshots/nVista_ratPFC2_tif/snapshot_20160705_094404";

    const isx::SpacingInfo expectedSi = isx::SpacingInfo(
            isx::SizeInPixels_t(1440, 1080),
            isx::SizeInMicrons_t(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE),
            isx::PointInMicrons_t(0, 0)
    );

    const std::map<size_t, uint16_t> expectedValues =
    {
        {0, 255},
        {1, 258},
        {1440, 267},
        {1440 * 1080 - 1, 708},
    };

    isx::CoreInitialize();

    SECTION("XML")
    {
        requireSnapshopProperties(
                basePath + ".xml",
                expectedSi,
                isx::TimingInfo(isx::Time(2016, 7, 5, 9, 44, 4, isx::DurationInSeconds(169, 1000)), isx::DurationInSeconds(50, 1000), 1),
                expectedValues
        );
    }

    SECTION("TIFF")
    {
        requireSnapshopProperties(
                basePath + ".tif",
                expectedSi,
                isx::TimingInfo(isx::Time(), isx::TimingInfo::s_defaultStep, 1),
                expectedValues
        );
    }

    isx::CoreShutdown();
}


TEST_CASE("nVista_ratPFC3_raw2hdf5_converted", "[core][snapshot]")
{
    const std::string basePath = g_resources["unitTestDataPath"] + "/Snapshots/nVista_ratPFC3_raw2hdf5_converted/snapshot_20160622_154726";

    const isx::SpacingInfo expectedSi = isx::SpacingInfo(
            isx::SizeInPixels_t(1440, 1080),
            isx::SizeInMicrons_t(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE),
            isx::PointInMicrons_t(0, 0)
    );

    const std::map<size_t, uint16_t> expectedValues =
    {
        {0, 374},
        {1, 389},
        {1440, 412},
        {1440 * 1080 - 1, 549},
    };

    isx::CoreInitialize();

    SECTION("XML")
    {
        requireSnapshopProperties(
                basePath + ".xml",
                expectedSi,
                isx::TimingInfo(isx::Time(2016, 6, 22, 15, 47, 26, isx::DurationInSeconds(516, 1000)), isx::DurationInSeconds(50, 1000), 1),
                expectedValues
        );
    }

    SECTION("HDF5")
    {
        // Yes, I think the start time in the HDF5 file is really different from the one in the XML,
        // and it doesn't seem to be timezone difference.
        requireSnapshopProperties(
                basePath + ".hdf5",
                expectedSi,
                isx::TimingInfo(isx::Time(2016, 6, 24, 17, 58, 46), isx::TimingInfo::s_defaultStep, 1),
                expectedValues
        );
    }

    isx::CoreShutdown();
}


TEST_CASE("nVoke", "[core][snapshot]")
{
    const std::string basePath = g_resources["unitTestDataPath"] + "/Snapshots/nVoke/snapshot_20170619_160245";

    const isx::SpacingInfo expectedSi = isx::SpacingInfo(
            isx::SizeInPixels_t(1440, 1080),
            isx::SizeInMicrons_t(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE),
            isx::PointInMicrons_t(0, 0)
    );

    const std::map<size_t, uint16_t> expectedValues =
    {
        {0, 327},
        {1, 343},
        {1440, 347},
        {1440 * 1080 - 1, 205},
    };

    isx::CoreInitialize();

    SECTION("XML")
    {
        requireSnapshopProperties(
                basePath + ".xml",
                expectedSi,
                isx::TimingInfo(isx::Time(2017, 6, 19, 16, 2, 45, isx::DurationInSeconds(316, 1000)), isx::DurationInSeconds(100, 2001), 1),
                expectedValues
        );
    }

    SECTION("TIFF")
    {
        requireSnapshopProperties(
                basePath + ".tif",
                expectedSi,
                isx::TimingInfo(isx::Time(), isx::TimingInfo::s_defaultStep, 1),
                expectedValues
        );
    }

    isx::CoreShutdown();
}


TEST_CASE("mos-1319", "[core][snapshot]")
{

    const std::string dataDirPath = g_resources["unitTestDataPath"] + "/Snapshots/mos-1319";
    const std::string filePath = dataDirPath + "/Snapshot 2.hdf5";

    isx::CoreInitialize();

    SECTION("HDF5")
    {
        const isx::SpacingInfo expectedSi = isx::SpacingInfo(
                isx::SizeInPixels_t(1440, 1080),
                isx::SizeInMicrons_t(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE),
                isx::PointInMicrons_t(0, 0)
        );

        const std::map<size_t, uint16_t> expectedValues =
        {
            {0, 0},
            {1, 0},
            {1440, 0},
            {1440 * 1080 - 1, 0},
        };

        requireSnapshopProperties(
                filePath,
                expectedSi,
                isx::TimingInfo(isx::Time(), isx::TimingInfo::s_defaultStep, 1),
                expectedValues
        );
    }

    const std::string outputDirPath = dataDirPath + "/output";
    isx::makeDirectory(outputDirPath);

    SECTION("HDF5 as GPIO")
    {
        isx::GpioDataParams inputParams(outputDirPath, filePath);

        auto outputParams = std::make_shared<isx::GpioDataOutputParams>();

        ISX_REQUIRE_EXCEPTION(
                isx::runGpioDataImporter(inputParams, outputParams, [](float){return false;}),
                isx::ExceptionDataIO,
                ""
        );
    }

    isx::removeDirectory(outputDirPath);

    isx::CoreShutdown();
}

TEST_CASE("snapshot-nVista3", "[core][snapshot]")
{
    const std::string inputDir = g_resources["unitTestDataPath"] + "/Snapshots/nVista3";

    isx::CoreInitialize();

    SECTION("MOS-1549")
    {
        const std::string filePath = inputDir + "/2018-06-19-11-34-37_snap.tiff";

        const isx::SpacingInfo expectedSi = isx::SpacingInfo(
                isx::SizeInPixels_t(1280, 800),
                isx::SizeInMicrons_t(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE),
                isx::PointInMicrons_t(0, 0)
        );

        const std::map<size_t, uint16_t> expectedValues =
        {
            {0, 357},
            {1, 367},
            {1280, 368},
            {1280 * 800 - 1, 636},
        };

        const isx::SpVideoFrame_t snapshot = isx::readImage(filePath);

        requireSnapshopProperties(
                filePath,
                expectedSi,
                isx::TimingInfo(isx::Time(), isx::TimingInfo::s_defaultStep, 1),
                expectedValues
        );
    }

    isx::CoreShutdown();
}
