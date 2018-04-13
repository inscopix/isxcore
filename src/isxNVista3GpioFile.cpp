#include "isxNVista3GpioFile.h"
#include "isxPathUtils.h"
#include "isxJsonUtils.h"
#include "isxException.h"
#include <algorithm>
#include <cstring>

#define ISX_DEBUG_NV3_GPIO 0
#if ISX_DEBUG_NV3_GPIO
#define ISX_LOG_DEBUG_NV3_GPIO(...) ISX_LOG_DEBUG(__VA_ARGS__)
#else
#define ISX_LOG_DEBUG_NV3_GPIO(...)
#endif

namespace
{

void
checkPayloadSize(const size_t inActual, const size_t inExpected)
{
    if (inActual != inExpected)
    {
        ISX_LOG_DEBUG_NV3_GPIO("Unexpected payload size: ", inActual, " != ", inExpected);
    }
}

} // namespace

namespace isx
{

const std::map<NVista3GpioFile::Channel, std::string> NVista3GpioFile::s_channelNames
{
    {NVista3GpioFile::Channel::FRAME_COUNTER, "Frame Count"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_0, "IO-9"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_1, "IO-10"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_2, "IO-11"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_3, "IO-12"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_4, "IO-13"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_5, "IO-14"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_6, "IO-15"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_7, "IO-16"},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_1, "GPIO-1"},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_2, "GPIO-2"},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_3, "GPIO-3"},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_4, "GPIO-4"},
    {NVista3GpioFile::Channel::EX_LED, "EX-LED"},
    {NVista3GpioFile::Channel::OG_LED, "OG-LED"},
    {NVista3GpioFile::Channel::DI_LED, "DI-LED"},
    {NVista3GpioFile::Channel::EFOCUS, "e-focus"},
    {NVista3GpioFile::Channel::TRIG, "Sensor TRIG"},
    {NVista3GpioFile::Channel::SYNC, "Sensor SYNC"},
    {NVista3GpioFile::Channel::FLASH, "Sensor FLASH"},
    {NVista3GpioFile::Channel::BNC_TRIG, "TRIG"},
    {NVista3GpioFile::Channel::BNC_SYNC, "SYNC"},
};

const std::map<NVista3GpioFile::Channel, SignalType> NVista3GpioFile::s_channelTypes
{
    {NVista3GpioFile::Channel::FRAME_COUNTER, SignalType::SPARSE},
    {NVista3GpioFile::Channel::DIGITAL_GPI_0, SignalType::SPARSE},
    {NVista3GpioFile::Channel::DIGITAL_GPI_1, SignalType::SPARSE},
    {NVista3GpioFile::Channel::DIGITAL_GPI_2, SignalType::SPARSE},
    {NVista3GpioFile::Channel::DIGITAL_GPI_3, SignalType::SPARSE},
    {NVista3GpioFile::Channel::DIGITAL_GPI_4, SignalType::SPARSE},
    {NVista3GpioFile::Channel::DIGITAL_GPI_5, SignalType::SPARSE},
    {NVista3GpioFile::Channel::DIGITAL_GPI_6, SignalType::SPARSE},
    {NVista3GpioFile::Channel::DIGITAL_GPI_7, SignalType::SPARSE},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_1, SignalType::SPARSE},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_2, SignalType::SPARSE},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_3, SignalType::SPARSE},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_4, SignalType::SPARSE},
    {NVista3GpioFile::Channel::EX_LED, SignalType::SPARSE},
    {NVista3GpioFile::Channel::OG_LED, SignalType::SPARSE},
    {NVista3GpioFile::Channel::DI_LED, SignalType::SPARSE},
    {NVista3GpioFile::Channel::TRIG, SignalType::SPARSE},
    {NVista3GpioFile::Channel::SYNC, SignalType::SPARSE},
    {NVista3GpioFile::Channel::FLASH, SignalType::SPARSE},
    {NVista3GpioFile::Channel::EFOCUS, SignalType::SPARSE},
    {NVista3GpioFile::Channel::BNC_TRIG, SignalType::SPARSE},
    {NVista3GpioFile::Channel::BNC_SYNC, SignalType::SPARSE},
};

NVista3GpioFile::NVista3GpioFile()
{
}

NVista3GpioFile::NVista3GpioFile(const std::string & inFileName, const std::string & inOutputDir)
    : m_fileName(inFileName)
    , m_outputDir(inOutputDir)
{
    m_file.open(m_fileName, std::ios::binary | std::ios_base::in);
    if (!m_file.good() || !m_file.is_open())
    {
        ISX_THROW(ExceptionFileIO, "Failed to open GPIO data file for reading: ", m_fileName);
    }
    m_valid = true;
}

NVista3GpioFile::~NVista3GpioFile()
{
    isx::closeFileStreamWithChecks(m_file, m_fileName);
}

bool
NVista3GpioFile::isValid()
{
    return m_valid;
}

const std::string &
NVista3GpioFile::getFileName()
{
    return m_fileName;
}

void
NVista3GpioFile::setCheckInCallback(AsyncCheckInCB_t inCheckInCB)
{
    m_checkInCB = inCheckInCB;
}

void
NVista3GpioFile::skipBytes(const size_t inNumBytes)
{
    m_file.ignore(inNumBytes);
}

void
NVista3GpioFile::skipWords(const size_t inNumWords)
{
    skipBytes(4 * inNumWords);
}

void
NVista3GpioFile::addPkt(const Channel inChannel, const uint64_t inTimeStamp, const float inValue)
{
    if (m_indices.find(inChannel) == m_indices.end())
    {
        m_indices[inChannel] = m_indices.size();
    }
    const EventBasedFileV2::DataPkt pkt(inTimeStamp, inValue, m_indices[inChannel]);
    m_packets.push_back(pkt);
}

void
NVista3GpioFile::addDigitalGpiPkts(const uint64_t inTsc, const uint16_t inDigitalGpi)
{
    uint64_t digitalGpi = inDigitalGpi;
    for (uint32_t i = uint32_t(Channel::DIGITAL_GPI_0); i <= uint32_t(Channel::DIGITAL_GPI_7); ++i)
    {
        addPkt(Channel(i), inTsc, float(digitalGpi & 0b1));
        digitalGpi >>= 1;
    }
}

AsyncTaskStatus
NVista3GpioFile::parse()
{
    uint32_t sync;
    PktHeader header;

    uint64_t tsc;
    uint32_t tscLow;
    uint32_t tscHigh;
    uint32_t fc;
    uint16_t digitalGpi;
    uint16_t bncGpioIn1, bncGpioIn2, bncGpioIn3, bncGpioIn4;
    uint16_t exLed, ogLed, diLed;
    uint16_t eFocus;
    uint32_t trigSyncFlash;
    uint16_t bncTrig, bncSync;

    m_packets.clear();
    m_indices.clear();

    size_t syncCount = 0;

    while (!m_file.eof())
    {
        read(sync);
        if (sync != s_syncWord)
        {
            continue;
        }
        ++syncCount;
        ISX_LOG_DEBUG_NV3_GPIO("Found sync at byte ", m_file.tellg());

        read(header);
        ISX_LOG_DEBUG_NV3_GPIO("Read packet type ", header.m_type);
        if ((header.m_type >> 8) != s_eventSignature)
        {
            ISX_LOG_DEBUG_NV3_GPIO("Found non-event header");
            continue;
        }
        ISX_LOG_DEBUG_NV3_GPIO("Read sequence ", header.m_sequence);
        ISX_LOG_DEBUG_NV3_GPIO("Read payloadSize ", header.m_payloadSize);

        if (Event(header.m_type) != Event::WAVEFORM)
        {
            read(tscHigh);
            read(tscLow);
            ISX_LOG_DEBUG_NV3_GPIO("Read TSC high ", tscHigh);
            ISX_LOG_DEBUG_NV3_GPIO("Read TSC low ", tscLow);
            tsc = (uint64_t(tscHigh) << 32) | uint64_t(tscLow);
            ISX_LOG_DEBUG_NV3_GPIO("Got TSC ", tsc);

            read(fc);
            ISX_LOG_DEBUG_NV3_GPIO("Read FC ", fc);
        }

        switch (Event(header.m_type))
        {
            case Event::CAPTURE_ALL:
                ISX_LOG_DEBUG_NV3_GPIO("Event::CAPTURE_ALL : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 9);
                addDigitalGpiPkts(tsc, read(digitalGpi));
                skipBytes(2);
                addPkt(Channel::BNC_GPIO_IN_1, tsc, float(read(bncGpioIn1)));
                addPkt(Channel::BNC_GPIO_IN_2, tsc, float(read(bncGpioIn2)));
                addPkt(Channel::BNC_GPIO_IN_3, tsc, float(read(bncGpioIn3)));
                addPkt(Channel::BNC_GPIO_IN_4, tsc, float(read(bncGpioIn4)));
                addPkt(Channel::EX_LED, tsc, float(read(exLed)));
                addPkt(Channel::OG_LED, tsc, float(read(ogLed)));
                addPkt(Channel::DI_LED, tsc, float(read(diLed)));
                addPkt(Channel::EFOCUS, tsc, float(read(eFocus)));
                read(trigSyncFlash);
                for (const auto channel : std::vector<Channel>({Channel::TRIG, Channel::SYNC, Channel::FLASH}))
                {
                    addPkt(channel, tsc, trigSyncFlash & 0b1);
                    trigSyncFlash >>= 1;
                }
                break;

            case Event::CAPTURE_GPIO:
                ISX_LOG_DEBUG_NV3_GPIO("Event::CAPTURE_GPIO : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 6);
                addDigitalGpiPkts(tsc, read(digitalGpi));
                addPkt(Channel::BNC_TRIG, tsc, float(read(bncTrig)));
                addPkt(Channel::BNC_GPIO_IN_1, tsc, float(read(bncGpioIn1)));
                addPkt(Channel::BNC_GPIO_IN_2, tsc, float(read(bncGpioIn2)));
                addPkt(Channel::BNC_GPIO_IN_3, tsc, float(read(bncGpioIn3)));
                addPkt(Channel::BNC_GPIO_IN_4, tsc, float(read(bncGpioIn4)));
                break;

            case Event::BNC_GPIO_IN_1:
                ISX_LOG_DEBUG_NV3_GPIO("Event::BNC_GPIO_IN_1 : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::BNC_GPIO_IN_1, tsc, float(read(bncGpioIn1)));
                skipBytes(2);
                break;

            case Event::BNC_GPIO_IN_2:
                ISX_LOG_DEBUG_NV3_GPIO("Event::BNC_GPIO_IN_2 : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::BNC_GPIO_IN_2, tsc, float(read(bncGpioIn2)));
                skipBytes(2);
                break;

            case Event::BNC_GPIO_IN_3:
                ISX_LOG_DEBUG_NV3_GPIO("Event::BNC_GPIO_IN_3 : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::BNC_GPIO_IN_3, tsc, float(read(bncGpioIn3)));
                skipBytes(2);
                break;

            case Event::BNC_GPIO_IN_4:
                ISX_LOG_DEBUG_NV3_GPIO("Event::BNC_GPIO_IN_4 : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::BNC_GPIO_IN_4, tsc, float(read(bncGpioIn4)));
                skipBytes(2);
                break;

            case Event::DIGITAL_GPI:
                ISX_LOG_DEBUG_NV3_GPIO("Event::DIGITAL_GPI : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 4);
                addDigitalGpiPkts(tsc, read(digitalGpi));
                skipBytes(2);
                break;

            case Event::EX_LED:
                ISX_LOG_DEBUG_NV3_GPIO("Event::EX_LED : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::EX_LED, tsc, float(read(exLed)));
                skipBytes(2);
                break;

            case Event::OG_LED:
                ISX_LOG_DEBUG_NV3_GPIO("Event::OG_LED : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::OG_LED, tsc, float(read(ogLed)));
                skipBytes(2);
                break;

            case Event::DI_LED:
                ISX_LOG_DEBUG_NV3_GPIO("Event::DI_LED : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::DI_LED, tsc, float(read(diLed)));
                skipBytes(2);
                break;

            case Event::FRAME_COUNT:
                ISX_LOG_DEBUG_NV3_GPIO("Event::FRAME_COUNT : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 3);
                addPkt(Channel::FRAME_COUNTER, tsc, float(fc));
                break;

            case Event::BNC_TRIG:
                ISX_LOG_DEBUG_NV3_GPIO("Event::BNC_TRIG : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::BNC_TRIG, tsc, float(read(bncTrig)));
                skipBytes(2);
                break;

            case Event::BNC_SYNC:
                ISX_LOG_DEBUG_NV3_GPIO("Event::BNC_SYNC : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::BNC_SYNC, tsc, float(read(bncSync)));
                skipBytes(2);
                break;

            case Event::WAVEFORM:
                ISX_LOG_DEBUG_NV3_GPIO("Event::WAVEFORM : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 2);
                skipWords(header.m_payloadSize);
                break;

            default:
                break;
        }
    }

    ISX_LOG_DEBUG_NV3_GPIO("syncCount = ", syncCount);

    if (syncCount == 0)
    {
        ISX_THROW(ExceptionFileIO, "Failed to find the beginning of the data stream and parse the file.");
    }

    // TODO : refactor with nVoke.
    uint64_t firstTime = 0;
    uint64_t lastTime = 0;
    if (!m_packets.empty())
    {
        firstTime = m_packets.front().offsetMicroSecs;
        lastTime = m_packets.back().offsetMicroSecs;
    }

    // TODO : Need an actual start time.
    const auto step = DurationInSeconds::fromMicroseconds(1);
    const isize_t numTimes = lastTime - firstTime + 1;
    //const isize_t numTimes = isize_t(double(lastTime - firstTime) * 1E-6 / (step.toDouble())) + 1;
    const Time startTime(DurationInSeconds::fromMicroseconds(firstTime));
    const TimingInfo timingInfo(startTime, step, numTimes);
    const Time endTime = timingInfo.getEnd();

    const size_t numChannels = m_indices.size();
    std::vector<std::string> channels(numChannels);
    std::vector<isx::SignalType> types(numChannels, isx::SignalType::SPARSE);
    for (auto & index : m_indices)
    {
        channels.at(index.second) = s_channelNames.at(index.first);
        types.at(index.second) = s_channelTypes.at(index.first);
    }

    m_outputFileName = m_outputDir + "/" + isx::getBaseName(m_fileName) + "_gpio.isxd";
    const std::vector<DurationInSeconds> steps(channels.size(), step);
    EventBasedFileV2 outputFile(m_outputFileName, DataSet::Type::GPIO, channels, steps, types);
    for (const auto p : m_packets)
    {
        outputFile.writeDataPkt(p);
    }

    outputFile.setTimingInfo(startTime, endTime);
    outputFile.closeFileForWriting();

    return isx::AsyncTaskStatus::COMPLETE;
}

const std::string &
NVista3GpioFile::getOutputFileName() const
{
    return m_outputFileName;
}

} // namespace isx
