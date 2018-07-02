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
#include "isxMovieFactory.h"
#include "isxMovie.h"

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

void
writeNV3SyncPacket(std::ofstream & inStream, const uint32_t inSequence, const bool inSync)
{
    uint32_t syncWord = isx::NVista3GpioFile::s_syncWord;
    inStream.write(reinterpret_cast<char *>(&syncWord), sizeof(syncWord));
    isx::NVista3GpioFile::PktHeader header;
    header.type = uint32_t(isx::NVista3GpioFile::Event::BNC_SYNC);
    header.sequence = inSequence;
    header.payloadSize = 4;
    inStream.write(reinterpret_cast<char *>(&header), sizeof(isx::NVista3GpioFile::PktHeader));
    isx::NVista3GpioFile::SyncPayload payload;
    payload.count.tscHigh = 0;
    payload.count.tscLow = inSequence;
    payload.count.fc = inSequence;
    payload.bncSync = uint32_t(inSync);
    inStream.write(reinterpret_cast<char *>(&payload), sizeof(isx::NVista3GpioFile::SyncPayload));
}

void
writeNV3AllPacket(std::ofstream & inStream, const uint32_t inSequence,
        const uint32_t inDigitalGpi,
        const uint16_t inBncGpio1,
        const uint16_t inBncGpio2,
        const uint16_t inBncGpio3,
        const uint16_t inBncGpio4,
        const uint16_t inExLed,
        const uint16_t inOgLed,
        const uint16_t inDiLed,
        const uint16_t inEFocus,
        const uint32_t inTrigSync
)
{
    uint32_t syncWord = isx::NVista3GpioFile::s_syncWord;
    inStream.write(reinterpret_cast<char *>(&syncWord), sizeof(syncWord));
    isx::NVista3GpioFile::PktHeader header;
    header.type = uint32_t(isx::NVista3GpioFile::Event::CAPTURE_ALL);
    header.sequence = inSequence;
    header.payloadSize = 9;
    inStream.write(reinterpret_cast<char *>(&header), sizeof(isx::NVista3GpioFile::PktHeader));
    isx::NVista3GpioFile::AllPayload payload;
    payload.count.tscHigh = 0;
    payload.count.tscLow = inSequence;
    payload.count.fc = inSequence;
    payload.digitalGpi = inDigitalGpi;
    payload.bncGpio1 = inBncGpio1;
    payload.bncGpio2 = inBncGpio2;
    payload.bncGpio3 = inBncGpio3;
    payload.bncGpio4 = inBncGpio4;
    payload.exLed = inExLed;
    payload.ogLed = inOgLed;
    payload.diLed = inDiLed;
    payload.eFocus = inEFocus;
    payload.trigSync = inTrigSync;
    inStream.write(reinterpret_cast<char *>(&payload), sizeof(isx::NVista3GpioFile::AllPayload));
}

void
requireNV3AllPayload(const isx::SpGpio_t & inGpio, const std::string & inChannel, const std::map<int64_t, float> & inValues)
{
    const isx::SpLogicalTrace_t trace = inGpio->getLogicalData(inChannel);
    const std::map<isx::Time, float> values = trace->getValues();
    REQUIRE(values.size() == inValues.size());
    for (const auto & v : inValues)
    {
        REQUIRE(values.at(isx::Time(isx::DurationInSeconds::fromMicroseconds(v.first))) == v.second);
    }
}

TEST_CASE("NVista3GpioFile", "[core]")
{
    const std::string inputDirPath = g_resources["unitTestDataPath"] + "/nVista3Gpio";
    const std::string outputDirPath = inputDirPath + "/output";
    isx::makeDirectory(outputDirPath);

    isx::CoreInitialize();

    SECTION("Write synthetic file with one channel to check only deltas get recorded")
    {
        const std::string inputFilePath = outputDirPath + "/synthetic.gpio";
        {
            std::ofstream inputFile(inputFilePath.c_str(), std::ios::binary);
            REQUIRE(inputFile.good());
            writeNV3SyncPacket(inputFile, 0, 0);
            writeNV3SyncPacket(inputFile, 1, 0);
            writeNV3SyncPacket(inputFile, 2, 1);
            writeNV3SyncPacket(inputFile, 3, 1);
            writeNV3SyncPacket(inputFile, 4, 0);
            REQUIRE(inputFile.good());
            inputFile.flush();
        }

        std::string outputFilePath;
        {
            isx::NVista3GpioFile raw(inputFilePath, outputDirPath);
            raw.parse();
            outputFilePath = raw.getOutputFileName();
        }

        const isx::SpGpio_t gpio = isx::readGpio(outputFilePath);

        REQUIRE(gpio->numberOfChannels() == 1);

        const isx::Time startTime;
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds::fromMicroseconds(1), 5);
        REQUIRE(gpio->getTimingInfo() == expTi);

        const isx::SpLogicalTrace_t trace = gpio->getLogicalData("BNC Sync Output");
        const std::map<isx::Time, float> values = trace->getValues();
        REQUIRE(values.size() == 3);
        REQUIRE(values.at(isx::Time(isx::DurationInSeconds::fromMicroseconds(0))) == 0);
        REQUIRE(values.at(isx::Time(isx::DurationInSeconds::fromMicroseconds(2))) == 1);
        REQUIRE(values.at(isx::Time(isx::DurationInSeconds::fromMicroseconds(4))) == 0);
    }

    SECTION("Write synthetic file with three all payloads to check files are read correctly")
    {
        const std::string inputFilePath = outputDirPath + "/synthetic.gpio";
        {
            std::ofstream inputFile(inputFilePath.c_str(), std::ios::binary);
            REQUIRE(inputFile.good());
            writeNV3AllPacket(inputFile, 0,
                    0b0000000010111001,
                    14523, 34, 263, 2880,
                    4000, 6000, 9000,
                    5678,
                    0b0000000000000001);
            writeNV3AllPacket(inputFile, 1,
                    0b0000000010111001,
                    14523, 34, 263, 2880,
                    4000, 6000, 9000,
                    5678,
                    0b0000000000000001);
            writeNV3AllPacket(inputFile, 2,
                    ~0b0000000010111001,
                    14539, 50, 279, 2896,
                    4001, 6001, 9001,
                    5679,
                    ~0b0000000000000001);
            REQUIRE(inputFile.good());
            inputFile.flush();
        }

        std::string outputFilePath;
        {
            isx::NVista3GpioFile raw(inputFilePath, outputDirPath);
            raw.parse();
            outputFilePath = raw.getOutputFileName();
        }

        const isx::SpGpio_t gpio = isx::readGpio(outputFilePath);

        REQUIRE(gpio->numberOfChannels() == 18);

        const isx::Time startTime;
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds::fromMicroseconds(1), 3);
        REQUIRE(gpio->getTimingInfo() == expTi);

        requireNV3AllPayload(gpio, "IO-9", {{0, 1.f}, {2, 0.f}});
        requireNV3AllPayload(gpio, "IO-10", {{0, 0.f}, {2, 1.f}});
        requireNV3AllPayload(gpio, "IO-11", {{0, 0.f}, {2, 1.f}});
        requireNV3AllPayload(gpio, "IO-12", {{0, 1.f}, {2, 0.f}});
        requireNV3AllPayload(gpio, "IO-13", {{0, 1.f}, {2, 0.f}});
        requireNV3AllPayload(gpio, "IO-14", {{0, 1.f}, {2, 0.f}});
        requireNV3AllPayload(gpio, "IO-15", {{0, 0.f}, {2, 1.f}});
        requireNV3AllPayload(gpio, "IO-16", {{0, 1.f}, {2, 0.f}});
        requireNV3AllPayload(gpio, "GPIO-1", {{0, 14512.f}, {2, 14528.f}});
        requireNV3AllPayload(gpio, "GPIO-2", {{0, 32.f}, {2, 48.f}});
        requireNV3AllPayload(gpio, "GPIO-3", {{0, 256.f}, {2, 272.f}});
        requireNV3AllPayload(gpio, "GPIO-4", {{0, 2880.f}, {2, 2896.f}});
        requireNV3AllPayload(gpio, "EX-LED", {{0, 4000.f}, {2, 4001.f}});
        requireNV3AllPayload(gpio, "OG-LED", {{0, 6000.f}, {2, 6001.f}});
        requireNV3AllPayload(gpio, "DI-LED", {{0, 9000.f}, {2, 9001.f}});
        requireNV3AllPayload(gpio, "e-focus", {{0, 5678.f}, {2, 5679.f}});
        requireNV3AllPayload(gpio, "BNC Trigger Input", {{0, 0.f}, {2, 1.f}});
        requireNV3AllPayload(gpio, "BNC Sync Output", {{0, 1.f}, {2, 0.f}});
    }

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

        REQUIRE(gpio->numberOfChannels() == 19);

        const isx::Time startTime;
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds::fromMicroseconds(1), 10000);
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

        REQUIRE(gpio->numberOfChannels() == 18);

        const isx::Time startTime;
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds::fromMicroseconds(1), 1142588);
        REQUIRE(gpio->getTimingInfo() == expTi);
    }

    SECTION("Verify that start time matches a corresponding movie")
    {
        const std::string baseName = "2018-06-21-17-51-03_video_sched_0";
        const std::string inputFilePath = inputDirPath + "/" + baseName + ".gpio";
        std::string outputFilePath;
        {
            isx::NVista3GpioFile raw(inputFilePath, outputDirPath);
            raw.parse();
            outputFilePath = raw.getOutputFileName();
        }
        const isx::SpGpio_t gpio = isx::readGpio(outputFilePath);

        const std::string movieFilePath = inputDirPath + "/" + baseName + ".isxd";
        const isx::SpMovie_t movie = isx::readMovie(movieFilePath);

        REQUIRE(gpio->getTimingInfo().getStart() == movie->getTimingInfo().getStart());
    }

    SECTION("MOS-1552")
    {
        const std::string inputFilePath = inputDirPath + "/2018-06-26-13-21-27_video.gpio";
        std::string outputFilePath;
        {
            isx::NVista3GpioFile raw(inputFilePath, outputDirPath);
            raw.parse();
            outputFilePath = raw.getOutputFileName();
        }
        const isx::SpGpio_t gpio = isx::readGpio(outputFilePath);

        REQUIRE(gpio->numberOfChannels() == 18);

        const isx::Time startTime;
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds::fromMicroseconds(1), 26569573);
        REQUIRE(gpio->getTimingInfo() == expTi);
    }

    isx::removeDirectory(outputDirPath);

    isx::CoreShutdown();
}
