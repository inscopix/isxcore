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
#include "isxStopWatch.h"

#include "catch.hpp"

#include "json.hpp"
using json = nlohmann::json;

#include <fstream>
#include <algorithm>

const isx::isize_t fileVersion = 2;
const size_t numNVista3Channels = 26;

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

TEST_CASE("GpioDataTest", "[core][gpio]")
{
    isx::CoreInitialize();

    SECTION("Parse a nVoke GPIO file - ANALOG and SYNC")
    {
        std::string fileName = isx::getAbsolutePath(g_resources["unitTestDataPath"] + "/recording_20161104_093749_gpio.raw");
        std::string outputDir = isx::getAbsolutePath(g_resources["unitTestDataPath"]);

        ///////////////////////////////////////////////////////////////
        // Expected values

        isx::Time start(isx::DurationInSeconds(1478277469, 1) + isx::DurationInSeconds(294107, 1000000));
        auto step = isx::DurationInSeconds::fromMilliseconds(1);
        const isx::isize_t numTimes = 3640;
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
        auto step = isx::DurationInSeconds::fromMilliseconds(1);
        const isx::isize_t numTimes = 9153;
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
writeNV3SyncPacket(
        std::ofstream & inStream,
        const uint32_t inSequence,
        const uint32_t inTimeStamp,
        const uint32_t inFrameCount,
        const bool inSync)
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
    payload.count.tscLow = inTimeStamp;
    payload.count.fc = inFrameCount;
    payload.bncSync = uint32_t(inSync);
    inStream.write(reinterpret_cast<char *>(&payload), sizeof(isx::NVista3GpioFile::SyncPayload));
}

void
writeNV3AllPacket(std::ofstream & inStream,
        const uint32_t inSequence,
        const uint32_t inTimeStamp,
        const uint32_t inFrameCount,
        const uint32_t inDigitalGpio,
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
    payload.count.tscLow = inTimeStamp;
    payload.count.fc = inFrameCount;
    payload.digitalGpio = inDigitalGpio;
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
requireGpioChannelValues(
        const isx::SpGpio_t & inGpio,
        const std::string & inChannel,
        const std::map<int64_t, float> & inValues,
        const isx::Time & inStart = isx::Time())
{
    const isx::SpLogicalTrace_t trace = inGpio->getLogicalData(inChannel);
    const std::map<isx::Time, float> values = trace->getValues();
    for (const auto & v : inValues)
    {
        const float expValue = v.second;
        const float actValue = values.at(inStart + isx::DurationInSeconds::fromMicroseconds(v.first));
        if (std::isnan(expValue))
        {
            REQUIRE(std::isnan(actValue));
        }
        else
        {
            REQUIRE(actValue == expValue);
        }
    }
}

TEST_CASE("NVista3GpioFile", "[core][gpio][nv3_gpio]")
{
    const std::string inputDirPath = g_resources["unitTestDataPath"] + "/nVista3Gpio";
    const std::string outputDirPath = inputDirPath + "/output";
    isx::makeDirectory(outputDirPath);
    isx::CoreInitialize();

    const float nan = std::numeric_limits<float>::quiet_NaN();

    SECTION("Write synthetic file with one channel to check only deltas get recorded")
    {
        const std::string inputFilePath = outputDirPath + "/synthetic.gpio";
        {
            std::ofstream inputFile(inputFilePath.c_str(), std::ios::binary);
            REQUIRE(inputFile.good());
            writeNV3SyncPacket(inputFile, 0, 0, 0, 0);
            writeNV3SyncPacket(inputFile, 1, 1, 1, 0);
            writeNV3SyncPacket(inputFile, 2, 2, 2, 1);
            writeNV3SyncPacket(inputFile, 3, 3, 3, 1);
            writeNV3SyncPacket(inputFile, 4, 4, 4, 0);
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
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds::fromMilliseconds(1), 1);
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
            writeNV3AllPacket(inputFile, 0, 0, 0,
                    0b101101100000000010111001,
                    14523, 34, 263, 2880,
                    4000, 6000, 9000,
                    5678,
                    0b0000000000000001);
            writeNV3AllPacket(inputFile, 1, 1, 1,
                    0b101101100000000010111001,
                    14523, 34, 263, 2880,
                    4000, 6000, 9000,
                    5678,
                    0b0000000000000001);
            writeNV3AllPacket(inputFile, 2, 2, 2,
                    ~0b101101100000000010111001,
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

        REQUIRE(gpio->numberOfChannels() == numNVista3Channels);

        const isx::Time startTime;
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds::fromMilliseconds(1), 1);
        REQUIRE(gpio->getTimingInfo() == expTi);

        requireGpioChannelValues(gpio, "Digital GPI 0", {{0, 1.f}, {2, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPI 1", {{0, 0.f}, {2, 1.f}});
        requireGpioChannelValues(gpio, "Digital GPI 2", {{0, 0.f}, {2, 1.f}});
        requireGpioChannelValues(gpio, "Digital GPI 3", {{0, 1.f}, {2, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPI 4", {{0, 1.f}, {2, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPI 5", {{0, 1.f}, {2, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPI 6", {{0, 0.f}, {2, 1.f}});
        requireGpioChannelValues(gpio, "Digital GPI 7", {{0, 1.f}, {2, 0.f}});

        requireGpioChannelValues(gpio, "Digital GPO 0", {{0, 0.f}, {2, 1.f}});
        requireGpioChannelValues(gpio, "Digital GPO 1", {{0, 1.f}, {2, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPO 2", {{0, 1.f}, {2, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPO 3", {{0, 0.f}, {2, 1.f}});
        requireGpioChannelValues(gpio, "Digital GPO 4", {{0, 1.f}, {2, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPO 5", {{0, 1.f}, {2, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPO 6", {{0, 0.f}, {2, 1.f}});
        requireGpioChannelValues(gpio, "Digital GPO 7", {{0, 1.f}, {2, 0.f}});

        requireGpioChannelValues(gpio, "GPIO-1", {{0, 14512.f}, {2, 14528.f}});
        requireGpioChannelValues(gpio, "GPIO-2", {{0, 32.f}, {2, 48.f}});
        requireGpioChannelValues(gpio, "GPIO-3", {{0, 256.f}, {2, 272.f}});
        requireGpioChannelValues(gpio, "GPIO-4", {{0, 2880.f}, {2, 2896.f}});
        requireGpioChannelValues(gpio, "EX-LED", {{0, 400.f}, {2, 400.1f}});
        requireGpioChannelValues(gpio, "OG-LED", {{0, 600.f}, {2, 600.1f}});
        requireGpioChannelValues(gpio, "DI-LED", {{0, 900.f}, {2, 900.1f}});
        requireGpioChannelValues(gpio, "e-focus", {{0, 5678.f}, {2, 5679.f}});
        requireGpioChannelValues(gpio, "BNC Trigger Input", {{0, 0.f}, {2, 1.f}});
        requireGpioChannelValues(gpio, "BNC Sync Output", {{0, 1.f}, {2, 0.f}});
    }

    SECTION("Write synthetic file with some all payloads and dropped packets")
    {
        const std::string inputFilePath = outputDirPath + "/synthetic.gpio";
        {
            std::ofstream inputFile(inputFilePath.c_str(), std::ios::binary);
            REQUIRE(inputFile.good());
            // Initial values => one write.
            writeNV3AllPacket(inputFile, 0, 0, 0,
                    0b010101100000000010111001,
                    14523, 34, 263, 2880,
                    4000, 6000, 9000,
                    5678,
                    0b0000000000000001);
            // No change, no dropped => no write.
            writeNV3AllPacket(inputFile, 1, 2, 0,
                    0b010101100000000010111001,
                    14523, 34, 263, 2880,
                    4000, 6000, 9000,
                    5678,
                    0b0000000000000001);
            // No change, dropped => two writes (one for dropped, one for recovery).
            writeNV3AllPacket(inputFile, 3, 6, 1,
                    0b010101100000000010111001,
                    14523, 34, 263, 2880,
                    4000, 6000, 9000,
                    5678,
                    0b0000000000000001);
            // Change, dropped => two writes (one for dropped, one for change).
            writeNV3AllPacket(inputFile, 6, 12, 2,
                    ~0b010101100000000010111001,
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

        REQUIRE(gpio->numberOfChannels() == numNVista3Channels);

        const isx::Time startTime;
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds::fromMilliseconds(1), 1);
        REQUIRE(gpio->getTimingInfo() == expTi);

        requireGpioChannelValues(gpio, "Digital GPI 0", {{0, 1.f}, {3, nan}, {6, 1.f}, {7, nan}, {12, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPI 1", {{0, 0.f}, {3, nan}, {6, 0.f}, {7, nan}, {12, 1.f}});
        requireGpioChannelValues(gpio, "Digital GPI 2", {{0, 0.f}, {3, nan}, {6, 0.f}, {7, nan}, {12, 1.f}});
        requireGpioChannelValues(gpio, "Digital GPI 3", {{0, 1.f}, {3, nan}, {6, 1.f}, {7, nan}, {12, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPI 4", {{0, 1.f}, {3, nan}, {6, 1.f}, {7, nan}, {12, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPI 5", {{0, 1.f}, {3, nan}, {6, 1.f}, {7, nan}, {12, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPI 6", {{0, 0.f}, {3, nan}, {6, 0.f}, {7, nan}, {12, 1.f}});
        requireGpioChannelValues(gpio, "Digital GPI 7", {{0, 1.f}, {3, nan}, {6, 1.f}, {7, nan}, {12, 0.f}});

        requireGpioChannelValues(gpio, "Digital GPO 0", {{0, 0.f}, {3, nan}, {6, 0.f}, {7, nan}, {12, 1.f}});
        requireGpioChannelValues(gpio, "Digital GPO 1", {{0, 1.f}, {3, nan}, {6, 1.f}, {7, nan}, {12, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPO 2", {{0, 1.f}, {3, nan}, {6, 1.f}, {7, nan}, {12, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPO 3", {{0, 0.f}, {3, nan}, {6, 0.f}, {7, nan}, {12, 1.f}});
        requireGpioChannelValues(gpio, "Digital GPO 4", {{0, 1.f}, {3, nan}, {6, 1.f}, {7, nan}, {12, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPO 5", {{0, 0.f}, {3, nan}, {6, 0.f}, {7, nan}, {12, 1.f}});
        requireGpioChannelValues(gpio, "Digital GPO 6", {{0, 1.f}, {3, nan}, {6, 1.f}, {7, nan}, {12, 0.f}});
        requireGpioChannelValues(gpio, "Digital GPO 7", {{0, 0.f}, {3, nan}, {6, 0.f}, {7, nan}, {12, 1.f}});

        requireGpioChannelValues(gpio, "GPIO-1", {{0, 14512.f}, {3, nan}, {6, 14512.f}, {7, nan}, {12, 14528.f}});
        requireGpioChannelValues(gpio, "GPIO-2", {{0, 32.f}, {3, nan}, {6, 32.f}, {7, nan}, {12, 48.f}});
        requireGpioChannelValues(gpio, "GPIO-3", {{0, 256.f}, {3, nan}, {6, 256.f}, {7, nan}, {12, 272.f}});
        requireGpioChannelValues(gpio, "GPIO-4", {{0, 2880.f}, {3, nan}, {6, 2880.f}, {7, nan}, {12, 2896.f}});
        requireGpioChannelValues(gpio, "EX-LED", {{0, 400.f}, {3, nan}, {6, 400.f}, {7, nan}, {12, 400.1f}});
        requireGpioChannelValues(gpio, "OG-LED", {{0, 600.f}, {3, nan}, {6, 600.f}, {7, nan}, {12, 600.1f}});
        requireGpioChannelValues(gpio, "DI-LED", {{0, 900.f}, {3, nan}, {6, 900.f}, {7, nan}, {12, 900.1f}});
        requireGpioChannelValues(gpio, "e-focus", {{0, 5678.f}, {3, nan}, {6, 5678.f}, {7, nan}, {12, 5679.f}});
        requireGpioChannelValues(gpio, "BNC Trigger Input", {{0, 0.f}, {3, nan}, {6, 0.f}, {7, nan}, {12, 1.f}});
        requireGpioChannelValues(gpio, "BNC Sync Output", {{0, 1.f}, {3, nan}, {6, 1.f}, {7, nan}, {12, 0.f}});
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

        // This has one more than usual because of the frame count.
        REQUIRE(gpio->numberOfChannels() == numNVista3Channels + 1);

        const isx::Time startTime;
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds::fromMilliseconds(1), 10);
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

        REQUIRE(gpio->numberOfChannels() == numNVista3Channels);

        const isx::Time startTime;
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds::fromMilliseconds(1), 3058);
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

        REQUIRE(gpio->numberOfChannels() == numNVista3Channels);

        const isx::Time startTime;
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds::fromMilliseconds(1), 26570);
        REQUIRE(gpio->getTimingInfo() == expTi);
    }

    SECTION("MOS-1559: No trigger")
    {
        const std::string inputFilePath = inputDirPath + "/2018-06-29-23-02-38_video.gpio";
        std::string outputFilePath;
        {
            isx::NVista3GpioFile raw(inputFilePath, outputDirPath);
            raw.parse();
            outputFilePath = raw.getOutputFileName();
        }
        const isx::SpGpio_t gpio = isx::readGpio(outputFilePath);

        REQUIRE(gpio->numberOfChannels() == numNVista3Channels);

        const isx::Time startTime(2018, 6, 29, 23, 2, 38, isx::DurationInSeconds::fromMilliseconds(865));
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds::fromMilliseconds(1), 38273);
        REQUIRE(gpio->getTimingInfo() == expTi);

        requireGpioChannelValues(gpio, "BNC Trigger Input", {
                {0, 0.f},
                {6931, nan},
                {8910, 0.f},
                // Skip most of the packets ...
                {37609288, nan},
                {37611267, 0.f},
                {38272660, 0.f},
        }, startTime);
        requireGpioChannelValues(gpio, "BNC Sync Output", {
                {0, 0.f},
                {6931, nan},
                {8910, 1.f},
                {18811, 0.f},
                {58414, 1.f},
                // Skip most of the packets ...
                {38224143, 1.f},
                {38234046, 0.f},
                {38272660, 0.f},
        }, startTime);
    }

    SECTION("MOS-1559: Trigger")
    {
        const std::string inputFilePath = inputDirPath + "/2018-06-29-23-07-14_video_trig_0.gpio";
        std::string outputFilePath;
        {
            isx::NVista3GpioFile raw(inputFilePath, outputDirPath);
            raw.parse();
            outputFilePath = raw.getOutputFileName();
        }
        const isx::SpGpio_t gpio = isx::readGpio(outputFilePath);

        REQUIRE(gpio->numberOfChannels() == numNVista3Channels);

        const isx::Time startTime(2018, 6, 29, 23, 7, 21, isx::DurationInSeconds::fromMilliseconds(2));
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds::fromMilliseconds(1), 9860);
        REQUIRE(gpio->getTimingInfo() == expTi);

        requireGpioChannelValues(gpio, "BNC Trigger Input", {
                {0, 1.f},
                {13863, nan},
                // Skip most of the packets ...
                {9855543, 0.f},
                {9856535, nan},
                {9859506, 0.f},
        }, startTime);
        requireGpioChannelValues(gpio, "BNC Sync Output", {
                {0, 1.f},
                {4952, 0.f},
                {13863, nan},
                {44556, 1.f},
                {54456, 0.f},
                // Skip most of the packets ...
                {9809009, 1.f},
                {9818911, 0.f},
                {9856535, nan},
                {9859506, 0.f},
        }, startTime);
    }

    isx::removeDirectory(outputDirPath);
    isx::CoreShutdown();
}

TEST_CASE("nVista3GpioWithExtras", "[core][gpio][nv3_gpio]")
{
    const std::string inputDirPath = g_resources["unitTestDataPath"] + "/nVista3Gpio";
    const std::string outputDirPath = inputDirPath + "/output";
    isx::makeDirectory(outputDirPath);
    isx::CoreInitialize();

    SECTION("MOS-1669-oldFile")
    {
        const std::string inputFilePath = inputDirPath + "/2018-08-09-11-13-52_video.gpio";
        std::string outputFilePath;
        {
            isx::NVista3GpioFile raw(inputFilePath, outputDirPath);
            raw.parse();
            outputFilePath = raw.getOutputFileName();
        }

        const isx::SpGpio_t gpio = isx::readGpio(outputFilePath);

        REQUIRE(gpio->numberOfChannels() == numNVista3Channels);

        const isx::Time startTime(2018, 8, 9, 11, 13, 52, isx::DurationInSeconds::fromMilliseconds(747));
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds(1, 1000), 8938672);
        REQUIRE(gpio->getTimingInfo() == expTi);

        const std::string extraPropsStr = gpio->getExtraProperties();
        const json extraProps = json::parse(extraPropsStr);
        REQUIRE(extraProps == nullptr);
    }

    SECTION("MOS-1669-newFile")
    {
        const std::string inputFilePath = inputDirPath + "/2018-08-15-09-01-58_video.gpio";
        std::string outputFilePath;
        {
            isx::NVista3GpioFile raw(inputFilePath, outputDirPath);
            raw.parse();
            outputFilePath = raw.getOutputFileName();
        }

        const isx::SpGpio_t gpio = isx::readGpio(outputFilePath);

        REQUIRE(gpio->numberOfChannels() == numNVista3Channels);

        const isx::Time startTime(2018, 8, 15, 9, 1, 58, isx::DurationInSeconds::fromMilliseconds(159));
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds(1, 5000), 42779);
        REQUIRE(gpio->getTimingInfo() == expTi);

        const std::string extraPropsStr = gpio->getExtraProperties();
        const json extraProps = json::parse(extraPropsStr);
        REQUIRE(extraProps != nullptr);
        REQUIRE(extraProps.at("ad").at("clock").get<size_t>() == 5000);
    }

    isx::removeDirectory(outputDirPath);
    isx::CoreShutdown();
}

TEST_CASE("nVoke1Gpio", "[core][gpio]")
{
    isx::CoreInitialize();

    const std::string inputDir = g_resources["unitTestDataPath"] + "/nVokeGpio";
    const std::string outputDir = inputDir + "/output";

    isx::removeDirectory(outputDir);
    isx::makeDirectory(outputDir);

    SECTION("MOS-1406")
    {
        const std::string inputFile = inputDir + "/gpio_3.raw";
        isx::NVokeGpioFile raw(inputFile, outputDir);
        raw.parse();

        const isx::Time start(2018, 3, 8, 0, 13, 25, isx::DurationInSeconds::fromMicroseconds(911315));

        const isx::SpGpio_t gpio = isx::readGpio(raw.getOutputFileName());
        REQUIRE(gpio->getTimingInfo().getStart() == start);
        const isx::SpLogicalTrace_t ogLed = gpio->getLogicalData("OG_LED");

        requireGpioChannelValues(gpio, "OG_LED",
            {
                {0, 0.f},
                {7174850, 0.f},
                {7174901, 5.f},
                {19402739, 5.f},
                {21403082, 0.f},
                {31630919, 0.f},
                {31630970, 5.f},
                {43858809, 5.f},
                {45858964, 0.f},
                {56086799, 0.f},
                {56086850, 5.f},
                {68314689, 5.f},
                {70314846, 0.f},
            },
            start
        );
    }

    SECTION("MOS-1701")
    {
        const std::string inputFile = inputDir + "/recording_20180928_103522_gpio.raw";
        isx::NVokeGpioFile raw(inputFile, outputDir);
        raw.parse();

        const isx::Time start(2018, 9, 28, 17, 35, 22, isx::DurationInSeconds::fromMicroseconds(22474));

        const isx::SpGpio_t gpio = isx::readGpio(raw.getOutputFileName());
        REQUIRE(gpio->getTimingInfo().getStart() == start);
        const isx::SpLogicalTrace_t ogLed = gpio->getLogicalData("OG_LED");

        requireGpioChannelValues(gpio, "OG_LED",
            {
                {0, 0.f},
                {3660328, 1.f},
                {9602068, 0.f},
                {10703405, 1.f},
                {12300614, 0.f},
                {13109846, 1.f},
                {14202742, 0.f},
                {15004025, 1.f},
                {16298505, 0.f},
            },
            start
        );
    }

    isx::removeDirectory(outputDir);
    isx::CoreShutdown();
}

TEST_CASE("nVista3Gpio-digitalGPO", "[core][gpio][nv3_gpio]")
{
    const std::string inputDirPath = g_resources["unitTestDataPath"] + "/nVista3Gpio";
    const std::string outputDirPath = inputDirPath + "/output";
    isx::removeDirectory(outputDirPath);
    isx::makeDirectory(outputDirPath);
    isx::CoreInitialize();

    SECTION("File with digital GPO 1, 3, 5 enabled")
    {
        const std::string inputFilePath = inputDirPath + "/2018-09-26-18-01-57_video.gpio";
        std::string outputFilePath;
        {
            isx::NVista3GpioFile raw(inputFilePath, outputDirPath);
            raw.parse();
            outputFilePath = raw.getOutputFileName();
        }

        const isx::SpGpio_t gpio = isx::readGpio(outputFilePath);

        REQUIRE(gpio->numberOfChannels() == numNVista3Channels);

        const isx::Time startTime(2018, 9, 26, 18, 1, 57, isx::DurationInSeconds::fromMilliseconds(118));
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds(1, 1000), 4598);
        REQUIRE(gpio->getTimingInfo() == expTi);

        for (size_t i = 0; i < 8; ++i)
        {
            const isx::SpLogicalTrace_t gpo = gpio->getLogicalData("Digital GPO " + std::to_string(i));
            // Even when there is no change in signal, the converter still outputs
            // 0 for the first and last timestamp, so we define channels with more than
            // 2 values to be "non-empty".
            REQUIRE((gpo->getValues().size() > 2) == (((i % 2) == 1) && (i <= 5)));
        }
    }

    isx::removeDirectory(outputDirPath);
    isx::CoreShutdown();
}

TEST_CASE("nVoke2-LEDPowerConversion", "[core][nv3_gpio]")
{
    isx::CoreInitialize();
    const std::string inputDir = g_resources["unitTestDataPath"] + "/nVista3Gpio";
    const std::string outputDir = inputDir + "/output";
    isx::removeDirectory(outputDir);
    isx::makeDirectory(outputDir);

    const std::string inputFile = inputDir + "/2018-10-26-16-08-31_video.gpio";
    std::string outputFile;
    {
        isx::NVista3GpioFile raw(inputFile, outputDir);
        raw.parse();
        outputFile = raw.getOutputFileName();
    }

    const isx::SpGpio_t gpio = isx::readGpio(outputFile);

    REQUIRE(gpio->numberOfChannels() == numNVista3Channels);

    const isx::Time startTime(2018, 10, 26, 16, 8, 31, isx::DurationInSeconds::fromMilliseconds(358));
    const isx::TimingInfo expTi(startTime, isx::DurationInSeconds(1, 1000), 16568);
    REQUIRE(gpio->getTimingInfo() == expTi);

    const isx::SpLogicalTrace_t exLed = gpio->getLogicalData("EX-LED");
    const std::map<isx::Time, float> & exLedValues = exLed->getValues();
    REQUIRE(exLedValues.at(startTime) == 2.f);
    REQUIRE(exLedValues.at(startTime + isx::DurationInSeconds::fromMicroseconds(16567000)) == 2.f);

    const isx::SpLogicalTrace_t ogLed = gpio->getLogicalData("OG-LED");
    const std::map<isx::Time, float> & ogLedValues = ogLed->getValues();
    REQUIRE(ogLedValues.at(startTime) == 10.f);
    REQUIRE(ogLedValues.at(startTime + isx::DurationInSeconds::fromMicroseconds(16567000)) == 10.f);

    isx::removeDirectory(outputDir);
    isx::CoreShutdown();
}

TEST_CASE("nVista3Gpio-benchmark", "[!hide]")
{
    const std::string inputDir = g_resources["realTestDataPath"] + "/nvista3_movie_gpio_sync";
    const std::string outputDir = inputDir + "/output";
    isx::removeDirectory(outputDir);
    isx::makeDirectory(outputDir);
    isx::CoreInitialize();

    SECTION("60 MB file")
    {
        const std::string inputFile = inputDir + "/2018-09-27-08-39-00_video_trig_0.gpio";
        std::string outputFile;
        {
            isx::NVista3GpioFile raw(inputFile, outputDir);
            isx::StopWatch sw;
            sw.start();
            raw.parse();
            sw.stop();
            ISX_LOG_INFO("Parsing took ", sw.getElapsedMs(), " ms.");
        }
    }

    isx::removeDirectory(outputDir);
    isx::CoreShutdown();
}

TEST_CASE("nVoke2-newClockKey", "[core][nv3_gpio]")
{
    isx::CoreInitialize();
    const std::string inputDir = g_resources["unitTestDataPath"] + "/nVista3Gpio";
    const std::string outputDir = inputDir + "/output";
    isx::removeDirectory(outputDir);
    isx::makeDirectory(outputDir);

    SECTION("manual mode")
    {
        const std::string inputFile = inputDir + "/2018-10-26-16-08-31_video.gpio";
        std::string outputFile;
        isx::NVista3GpioFile raw(inputFile, outputDir);
        raw.parse();
        outputFile = raw.getOutputFileName();

        const isx::SpGpio_t gpio = isx::readGpio(outputFile);
        const isx::Time startTime(2018, 10, 26, 16, 8, 31, isx::DurationInSeconds::fromMilliseconds(358));
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds(1, 1000), 16568);
        REQUIRE(gpio->getTimingInfo() == expTi);
    }

    SECTION("auto mode from davep")
    {
        const std::string inputFile = inputDir + "/2018-10-26-09-42-53_video_trig_0.gpio";
        isx::NVista3GpioFile raw(inputFile, outputDir);
        raw.parse();
        const std::string outputFile = raw.getOutputFileName();

        const isx::SpGpio_t gpio = isx::readGpio(outputFile);
        const isx::Time startTime(2018, 10, 26, 9, 42, 53, isx::DurationInSeconds::fromMilliseconds(138));
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds(1, 4800), 29468);
        REQUIRE(gpio->getTimingInfo() == expTi);
    }

    SECTION("auto mode from camille")
    {
        const std::string inputFile = inputDir + "/2018-10-30-11-11-53_video_trig_0.gpio";
        isx::NVista3GpioFile raw(inputFile, outputDir);
        raw.parse();
        const std::string outputFile = raw.getOutputFileName();

        const isx::SpGpio_t gpio = isx::readGpio(outputFile);
        const isx::Time startTime(2018, 10, 30, 11, 11, 53, isx::DurationInSeconds::fromMilliseconds(500));
        const isx::TimingInfo expTi(startTime, isx::DurationInSeconds(1, 4800), 130166);
        REQUIRE(gpio->getTimingInfo() == expTi);
    }

    isx::removeDirectory(outputDir);
    isx::CoreShutdown();
}


TEST_CASE("nVista3GpioClosedLoop", "[core][gpio][nv3_gpio]")
{
    const std::string inputDirPath = g_resources["unitTestDataPath"] + "/nVista3Gpio/closed_loop";
    const std::string outputDirPath = inputDirPath + "/output";
    isx::makeDirectory(outputDirPath);
    isx::CoreInitialize();

    SECTION("Real data")
    {
        const std::string inputFilePath = inputDirPath + "/test_closed_loop_2024-07-03-11-54-19_video.gpio";
        std::string outputFilePath;
        {
            isx::NVista3GpioFile raw(inputFilePath, outputDirPath);
            raw.parse();
            outputFilePath = raw.getOutputFileName();
        }

        const isx::SpGpio_t gpio = isx::readGpio(outputFilePath);

        const auto channelList = gpio->getChannelList();

        // Check GPI channels are renamed correctly
        REQUIRE(channelList[4] == "SoftTrig-1");
        REQUIRE(channelList[5] == "SoftTrig-2");
        REQUIRE(channelList[6] == "SoftTrig-3");
        REQUIRE(channelList[7] == "SoftTrig-4");

        // Check GPO channels don't exist in parsed event file
        const std::vector<std::string> ignoredChannels = {
            "Digital GPO 4",
            "Digital GPO 5",
            "Digital GPO 6",
            "Digital GPO 7"
        };
        
        for (const auto & ignoredChannel : ignoredChannels)
        {
            bool channelExists = false;
            for (const auto & channel : channelList)
            {
                if (channel ==  ignoredChannel)
                {
                    channelExists = true;
                }
            }

            REQUIRE(!channelExists);
            REQUIRE(gpio->getLogicalData(ignoredChannel) == nullptr);
        }
    }

    isx::removeDirectory(outputDirPath);
    isx::CoreShutdown();
}