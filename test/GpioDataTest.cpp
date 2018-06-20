#include "isxNVokeGpioFile.h"
#include "isxNVistaGpioFile.h"
#include "isxTest.h"
#include "isxPathUtils.h"
#include "isxJsonUtils.h"
#include "isxException.h"
#include "isxDataSet.h"
#include "isxGpio.h"
#include "isxFileTypes.h"
#include "isxNVista3GpioFile.h"

#include "catch.hpp"

#include <fstream>
#include <algorithm>

const isx::isize_t fileVersion = 2;

void testNVokeParsing(
    const std::string & inFileName,
    const std::string & inOutputDir,
    const isx::json inFileJsonHeader,
    const std::vector<uint64_t> & inMicroSecs,
    const std::vector<float> & inPowerLevel
    )
{
    std::string dir = isx::getDirName(inFileName);
    std::string base = isx::getBaseName(inFileName);

    std::string fullName = dir + "/" + base + "_gpio.isxd";
    std::remove(fullName.c_str());

    isx::NVokeGpioFile raw(inFileName, inOutputDir);
    try
    {
        raw.parse();
    }
    catch(const isx::ExceptionDataIO & error)
    {
        FAIL("There was a DataIO exception when parsing file: ", error.what());
    }
    catch(const isx::ExceptionFileIO & error)
    {
        FAIL("There was a FileIO exception when parsing file: ", error.what());
    }

    std::string filename = raw.getOutputFileName();
    REQUIRE(!filename.empty());
    REQUIRE(filename == fullName);

    // Compare file json header
    std::fstream file(fullName, std::ios::binary | std::ios_base::in);
    if (!file.good() || !file.is_open())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open gpio file for reading: ", fullName);
    }

    std::ios::pos_type headerPos;
    isx::json j = isx::readJsonHeaderAtEnd(file, headerPos);
    REQUIRE(j == inFileJsonHeader);

    auto channels = j["channel list"];

    for(isx::isize_t channelIdx = 0; channelIdx < channels.size(); ++channelIdx)
    {
        /// Read first data value for channel
        isx::EventBasedFileV2::DataPkt pkt;
        file.seekg(0, std::ios_base::beg);

        while (1)
        {
            file.read((char *) &pkt, sizeof(pkt));
            if (!file.good())
            {
                ISX_THROW(isx::ExceptionFileIO,
                    "Failed to read values from  gpio file: ", fullName);
            }

            if (pkt.signal == channelIdx)
            {
                REQUIRE(pkt.offsetMicroSecs == inMicroSecs.at(channelIdx));
                REQUIRE(pkt.value == inPowerLevel.at(channelIdx));
                break;
            }
        }
    }

    std::remove(filename.c_str());
}

TEST_CASE("GpioDataTest", "[core]")
{
    isx::CoreInitialize();

    SECTION("Parse a nVoke GPIO file - ANALOG and SYNC")
    {
        std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/recording_20161104_093749_gpio.raw");
        std::string outputDir = isx::getAbsolutePath(g_resources["unitTestDataPath"]);

        ///////////////////////////////////////////////////////////////
        // Expected values

        isx::Time start(isx::DurationInSeconds(1478277469, 1) + isx::DurationInSeconds(294107, 1000000));
        auto step = isx::DurationInSeconds::fromMicroseconds(1);
        const isx::isize_t numTimes = 3639037;
        isx::TimingInfo ti(start, step, numTimes);
        std::vector<uint64_t> usecsFromStart{ 0, 17693};
        std::vector<float> power{float(2.6556396484375), 0.0f};

        isx::json header;
        std::vector<std::string> channelList{"GPIO4_AI", "SYNC"};
        header["type"] = size_t(isx::DataSet::Type::GPIO);
        header["channel list"] = channelList;
        header["global times"] = {isx::convertTimeToJson(ti.getStart()), isx::convertTimeToJson(ti.getEnd())};
        header["producer"] = isx::getProducerAsJson();
        header["fileVersion"] = fileVersion;
        header["fileType"] = int(isx::FileType::V2);
        isx::json jsteps = {isx::convertRatioToJson(step), isx::convertRatioToJson(step)};

        header["signalSteps"] = jsteps;
        header["signalTypes"] = std::vector<uint8_t>({uint8_t(isx::SignalType::DENSE), uint8_t(isx::SignalType::SPARSE)});
        header["startOffsets"] = usecsFromStart;
        header["numSamples"] = std::vector<uint64_t>({3640, 145});
        header["metrics"] = isx::convertEventMetricsToJson(isx::EventMetrics_t());
        header["extraProperties"] = nullptr;

        // End of expected values ********************************************

        testNVokeParsing(
            fileName,
            outputDir,
            header,
            usecsFromStart,
            power);
    }

    SECTION("Parse a nVoke GPIO file - EX_LED, SYNC and TRIG")
    {
        std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/recording_20170126_143728_gpio.raw");
        std::string outputDir = isx::getAbsolutePath(g_resources["unitTestDataPath"]);

        ///////////////////////////////////////////////////////////////
        // Expected values *********************************************

        isx::Time start(isx::DurationInSeconds(1485470243, 1) + isx::DurationInSeconds(163233, 1000000));
        auto step = isx::DurationInSeconds::fromMicroseconds(1);
        const isx::isize_t numTimes = 9152909;
        isx::TimingInfo ti(start, step, numTimes);
        std::vector<uint64_t> usecsFromStart{0, 3077753, 3875367};
        std::vector<float> power{1.0f, 1.5f, 1.0f};

        isx::json header;
        std::vector<std::string> eventsChannelList{"TRIG", "EX_LED", "SYNC"};
        header["type"] = size_t(isx::DataSet::Type::GPIO);
        header["channel list"] = eventsChannelList;
        header["global times"] = {isx::convertTimeToJson(ti.getStart()), isx::convertTimeToJson(ti.getEnd())};
        header["producer"] = isx::getProducerAsJson();
        header["fileVersion"] = fileVersion;
        header["fileType"] = int(isx::FileType::V2);

        const isx::json jstep = isx::convertRatioToJson(step);
        isx::json jsteps = {jstep, jstep, jstep};

        header["signalSteps"] = jsteps;
        header["signalTypes"] = std::vector<uint8_t>(3, uint8_t(isx::SignalType::SPARSE));
        header["startOffsets"] = usecsFromStart;
        header["numSamples"] = std::vector<uint64_t>({4, 2, 174});
        header["metrics"] = isx::convertEventMetricsToJson(isx::EventMetrics_t());
        header["extraProperties"] = nullptr;

        // End of expected values ********************************************
        //////////////////////////////////////////////////////////////////////

        testNVokeParsing(
            fileName,
            outputDir,
            header,
            usecsFromStart,
            power);
    }

    SECTION("Parse a nVista GPIO file - SYNC, TRIG, IO1, IO2")
    {
        std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/nVistaGpio/test_nvista_gpio.hdf5");
        std::string outputDir = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/nVistaGpio");

        isx::NVistaGpioFile raw(fileName, outputDir, nullptr);
        try
        {
            raw.parse();
        }
        catch(const isx::ExceptionDataIO & error)
        {
            FAIL("There was a DataIO exception when parsing file: ", error.what());
        }
        catch(const isx::ExceptionFileIO & error)
        {
            FAIL("There was a FileIO exception when parsing file: ", error.what());
        }

        std::string filename = raw.getOutputFileName();

        REQUIRE(!filename.empty());

        /// Expected output as parsed with export_nvista_gpio.py
        std::map<std::string, std::vector<float>> expected;
        expected["IO1"] = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f,
                            1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f,
                            1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};

        expected["IO2"] = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f,
                            1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f,
                            1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};

        expected["sync"] = { 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f,
                             1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f,
                             1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f,};

        expected["trigger"] = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f,
                                1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f,
                                1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};

        /// Read the output of the parser
        isx::SpGpio_t gpio = isx::readGpio(filename);

        std::vector<std::string> channels = gpio->getChannelList();
        for (auto & c : channels)
        {
            REQUIRE(gpio->isAnalog(c));
            isx::SpFTrace_t t = gpio->getAnalogData(c);
            REQUIRE(t);
            auto & exp = expected.at(c);

            for (isx::isize_t i(0); i < exp.size(); ++i)
            {
                REQUIRE(exp.at(i) == t->getValue(i));
            }
        }
    }

    SECTION("Try to parse an nVista raw file")
    {
        const std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/nVistaRaw/recording_20170822_163550.raw");
        const std::string outputDir = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/nVistaRaw");

        ISX_REQUIRE_EXCEPTION(isx::NVokeGpioFile raw(fileName, outputDir), isx::ExceptionFileIO, "");
    }

    SECTION("MOS-1365. Make sure we don't error.")
    {
        const std::string inputDirPath = g_resources["unitTestDataPath"] + "/nVokeGpio";
        const std::string filePath = inputDirPath + "/recording_20170917_171029_gpio.raw";

        const std::string outputDirPath = inputDirPath + "/output";
        isx::makeDirectory(outputDirPath);

        isx::NVokeGpioFile raw(filePath, outputDirPath);
        raw.parse();
    }

    isx::CoreShutdown();
}

TEST_CASE("NVista3GpioFile", "[core]")
{
    const std::string inputDirPath = g_resources["unitTestDataPath"] + "/nVista3Gpio";
    const std::string outputDirPath = inputDirPath + "/output";
    isx::makeDirectory(outputDirPath);

    isx::CoreInitialize();

    SECTION("MOS-1450")
    {
        const std::string inputFilePath = inputDirPath + "/adp_events_10000.gpio";
        std::string outputFilePath;
        {
            isx::NVista3GpioFile raw(inputFilePath, outputDirPath);
            raw.parse();
            outputFilePath = raw.getOutputFileName();
        }

        const isx::SpGpio_t gpio = isx::readGpio(outputFilePath);

        REQUIRE(gpio->numberOfChannels() == 22);

        const isx::TimingInfo expTi(isx::Time(), isx::DurationInSeconds::fromMicroseconds(1), 10000);
        REQUIRE(gpio->getTimingInfo() == expTi);
    }

    SECTION("MOS-1548")
    {
        const std::string inputFilePath = inputDirPath + "/2018-06-19-15-41-43_video.gpio";
        std::string outputFilePath;
        {
            isx::NVista3GpioFile raw(inputFilePath, outputDirPath);
            raw.parse();
            outputFilePath = raw.getOutputFileName();
        }

        const isx::SpGpio_t gpio = isx::readGpio(outputFilePath);

        REQUIRE(gpio->numberOfChannels() == 19);

        const isx::TimingInfo expTi(isx::Time(), isx::DurationInSeconds::fromMicroseconds(1), 3057457);
        REQUIRE(gpio->getTimingInfo() == expTi);
    }

    isx::CoreShutdown();
}
