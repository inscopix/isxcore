#include "isxNVista3GpioFile.h"
#include "isxPathUtils.h"
#include "isxJsonUtils.h"
#include "isxException.h"
#include "isxGpioUtils.h"
#include <algorithm>
#include <cstring>

#define ISX_DEBUG_NV3_GPIO 0
#if ISX_DEBUG_NV3_GPIO
#define ISX_LOG_DEBUG_NV3_GPIO(...) ISX_LOG_DEBUG(__VA_ARGS__)
#else
#define ISX_LOG_DEBUG_NV3_GPIO(...)
#endif

namespace isx
{

BadGpioPacket::BadGpioPacket(const std::string & file, int line, const std::string & message)
    : Exception(file, line, message)
{
}

BadGpioPacket::~BadGpioPacket()
{
}

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
    {NVista3GpioFile::Channel::BNC_GPIO_1, "GPIO-1"},
    {NVista3GpioFile::Channel::BNC_GPIO_2, "GPIO-2"},
    {NVista3GpioFile::Channel::BNC_GPIO_3, "GPIO-3"},
    {NVista3GpioFile::Channel::BNC_GPIO_4, "GPIO-4"},
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
    {NVista3GpioFile::Channel::BNC_GPIO_1, SignalType::SPARSE},
    {NVista3GpioFile::Channel::BNC_GPIO_2, SignalType::SPARSE},
    {NVista3GpioFile::Channel::BNC_GPIO_3, SignalType::SPARSE},
    {NVista3GpioFile::Channel::BNC_GPIO_4, SignalType::SPARSE},
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
        // This used to be one line, but on Linux the first index was 1
        // instead of 0 and I'm not sure why. Separating the lines fixed
        // that bug.
        const size_t numChannels = m_indices.size();
        m_indices[inChannel] = numChannels;
    }
    const EventBasedFileV2::DataPkt pkt(inTimeStamp, inValue, m_indices[inChannel]);
    m_packets.push_back(pkt);
}

void
NVista3GpioFile::addDigitalGpiPkts(const uint64_t inTsc, uint16_t inDigitalGpi)
{
    for (uint32_t i = uint32_t(Channel::DIGITAL_GPI_0); i <= uint32_t(Channel::DIGITAL_GPI_7); ++i)
    {
        addPkt(Channel(i), inTsc, float(inDigitalGpi & 0b1));
        inDigitalGpi >>= 1;
    }
}

void
NVista3GpioFile::addTrigSyncFlashPkts(const uint64_t inTsc, uint16_t inTrigSyncFlash)
{
    for (const auto channel : std::vector<Channel>({Channel::TRIG, Channel::SYNC, Channel::FLASH}))
    {
        addPkt(channel, inTsc, float(inTrigSyncFlash & 0b1));
        inTrigSyncFlash >>= 1;
    }
}

void
NVista3GpioFile::readParseAddGpioPayload(const uint32_t inExpectedSize, const Channel inChannel)
{
    const auto payload = read<GpioPayload>(inExpectedSize);
    const uint64_t tsc = parseTsc(payload.count);
    addPkt(inChannel, tsc, float(payload.bncGpio));
}

void
NVista3GpioFile::readParseAddLedPayload(const uint32_t inExpectedSize, const Channel inChannel)
{
    const auto payload = read<LedPayload>(inExpectedSize);
    const uint64_t tsc = parseTsc(payload.count);
    addPkt(inChannel, tsc, float(payload.led));
}

uint64_t
NVista3GpioFile::parseTsc(const CountPayload & inCount)
{
    ISX_LOG_DEBUG_NV3_GPIO("Got TSC high ", inCount.tscHigh);
    ISX_LOG_DEBUG_NV3_GPIO("Got TSC low ", inCount.tscLow);
    const uint64_t tsc = (uint64_t(inCount.tscHigh) << 32) | uint64_t(inCount.tscLow);
    ISX_LOG_DEBUG_NV3_GPIO("Got TSC ", tsc);
    return tsc;
}

void
NVista3GpioFile::readParseAddPayload(const PktHeader & inHeader)
{
    switch (Event(inHeader.type))
    {
        case Event::CAPTURE_ALL:
        {
            ISX_LOG_DEBUG_NV3_GPIO("Event::CAPTURE_ALL");
            const auto payload = read<AllPayload>(inHeader.payloadSize);
            const uint64_t tsc = parseTsc(payload.count);
            addDigitalGpiPkts(tsc, uint16_t(payload.digitalGpi));
            addGpioPkts(tsc, payload);
            addPkt(Channel::EX_LED, tsc, float(payload.exLed));
            addPkt(Channel::OG_LED, tsc, float(payload.ogLed));
            addPkt(Channel::DI_LED, tsc, float(payload.diLed));
            addPkt(Channel::EFOCUS, tsc, float(payload.eFocus));
            addTrigSyncFlashPkts(tsc, uint16_t(payload.trigSyncFlash));
            break;
        }

        case Event::CAPTURE_GPIO:
        {
            ISX_LOG_DEBUG_NV3_GPIO("Event::CAPTURE_GPIO");
            const auto payload = read<AllGpioPayload>(inHeader.payloadSize);
            const uint64_t tsc = parseTsc(payload.count);
            addDigitalGpiPkts(tsc, payload.digitalGpi);
            addPkt(Channel::BNC_TRIG, tsc, float(payload.bncTrig));
            addGpioPkts(tsc, payload);
            break;
        }

        case Event::BNC_GPIO_1:
            ISX_LOG_DEBUG_NV3_GPIO("Event::BNC_GPIO_1");
            readParseAddGpioPayload(inHeader.payloadSize, Channel::BNC_GPIO_1);
            break;

        case Event::BNC_GPIO_2:
            ISX_LOG_DEBUG_NV3_GPIO("Event::BNC_GPIO_2");
            readParseAddGpioPayload(inHeader.payloadSize, Channel::BNC_GPIO_2);
            break;

        case Event::BNC_GPIO_3:
            ISX_LOG_DEBUG_NV3_GPIO("Event::BNC_GPIO_3");
            readParseAddGpioPayload(inHeader.payloadSize, Channel::BNC_GPIO_3);
            break;

        case Event::BNC_GPIO_4:
            ISX_LOG_DEBUG_NV3_GPIO("Event::BNC_GPIO_4");
            readParseAddGpioPayload(inHeader.payloadSize, Channel::BNC_GPIO_4);
            break;

        case Event::DIGITAL_GPI:
        {
            ISX_LOG_DEBUG_NV3_GPIO("Event::DIGITAL_GPI");
            const auto payload = read<DigitalGpiPayload>(inHeader.payloadSize);
            const uint64_t tsc = parseTsc(payload.count);
            addDigitalGpiPkts(tsc, uint16_t(payload.digitalGpi));
            break;
        }

        case Event::EX_LED:
            ISX_LOG_DEBUG_NV3_GPIO("Event::EX_LED");
            readParseAddLedPayload(inHeader.payloadSize, Channel::EX_LED);
            break;

        case Event::OG_LED:
            ISX_LOG_DEBUG_NV3_GPIO("Event::OG_LED");
            readParseAddLedPayload(inHeader.payloadSize, Channel::OG_LED);
            break;

        case Event::DI_LED:
            ISX_LOG_DEBUG_NV3_GPIO("Event::DI_LED");
            readParseAddLedPayload(inHeader.payloadSize, Channel::DI_LED);
            break;

        case Event::FRAME_COUNT:
        {
            ISX_LOG_DEBUG_NV3_GPIO("Event::FRAME_COUNT");
            const auto payload = read<CountPayload>(inHeader.payloadSize);
            const uint64_t tsc = parseTsc(payload);
            addPkt(Channel::FRAME_COUNTER, tsc, float(payload.fc));
            break;
        }

        case Event::BNC_TRIG:
        {
            ISX_LOG_DEBUG_NV3_GPIO("Event::BNC_TRIG");
            const auto payload = read<TrigPayload>(inHeader.payloadSize);
            const uint64_t tsc = parseTsc(payload.count);
            addPkt(Channel::BNC_TRIG, tsc, float(payload.bncTrig));
            break;
        }

        case Event::BNC_SYNC:
        {
            ISX_LOG_DEBUG_NV3_GPIO("Event::BNC_SYNC");
            const auto payload = read<SyncPayload>(inHeader.payloadSize);
            const uint64_t tsc = parseTsc(payload.count);
            addPkt(Channel::BNC_SYNC, tsc, float(payload.bncSync));
            break;
        }

        case Event::WAVEFORM:
            ISX_LOG_DEBUG_NV3_GPIO("Event::WAVEFORM");
            read<WaveformPayload>(inHeader.payloadSize);
            break;

        default:
            break;
    }
}

AsyncTaskStatus
NVista3GpioFile::parse()
{
    m_file.seekg(0, m_file.end);
    const float progressMultiplier = 1 / float(m_file.tellg());
    m_file.seekg(0, m_file.beg);
    std::ios::pos_type curPos = m_file.tellg();

    m_packets.clear();
    m_indices.clear();
    size_t syncCount = 0;

    while (m_file.good())
    {
        const auto sync = read<uint32_t>();
        if (!m_file.good())
        {
            break;
        }

        curPos = m_file.tellg();
        if (m_checkInCB && m_checkInCB(progressMultiplier * float(curPos)))
        {
            return AsyncTaskStatus::CANCELLED;
        }

        if (sync != s_syncWord)
        {
            continue;
        }
        ++syncCount;

        ISX_LOG_DEBUG_NV3_GPIO("Found sync at byte ", curPos);

        const auto header = read<PktHeader>();
        if (!m_file.good())
        {
            break;
        }
        ISX_LOG_DEBUG_NV3_GPIO("Read packet header: ", header.type, ", ", header.sequence, ", ", header.payloadSize);

        if ((header.type >> 8) != s_eventSignature)
        {
            ISX_LOG_DEBUG_NV3_GPIO("Found non-event header");
            continue;
        }

        try
        {
            readParseAddPayload(header);
        }
        catch (const BadGpioPacket &)
        {
            ISX_LOG_ERROR("Skipping bad GPIO packet at byte ", curPos, " with header (",
                    header.type, ", ", header.sequence, ", ", header.payloadSize, ").");
        }
    }

    ISX_LOG_DEBUG_NV3_GPIO("syncCount = ", syncCount);

    if (syncCount == 0)
    {
        ISX_THROW(ExceptionFileIO, "Failed to find the beginning of the data stream and parse the file.");
    }

    m_outputFileName = m_outputDir + "/" + isx::getBaseName(m_fileName) + "_gpio.isxd";

    const size_t numChannels = m_indices.size();
    std::vector<std::string> channels(numChannels);
    std::vector<isx::SignalType> types(numChannels, isx::SignalType::SPARSE);
    for (auto & index : m_indices)
    {
        channels.at(index.second) = s_channelNames.at(index.first);
        types.at(index.second) = s_channelTypes.at(index.first);
    }

    uint64_t firstTime = 0;
    uint64_t lastTime = 0;
    if (!m_packets.empty())
    {
        firstTime = m_packets.front().offsetMicroSecs;
        lastTime = m_packets.back().offsetMicroSecs;
    }

    // TODO : Need actual start time.
    const Time startTime = isx::Time();

    writePktsToEventBasedFile(m_outputFileName, m_packets, channels, types,
            startTime, firstTime, lastTime);

    return isx::AsyncTaskStatus::COMPLETE;
}

const std::string &
NVista3GpioFile::getOutputFileName() const
{
    return m_outputFileName;
}

} // namespace isx
