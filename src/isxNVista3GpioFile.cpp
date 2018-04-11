#include "isxNVista3GpioFile.h"
#include "isxPathUtils.h"
#include "isxJsonUtils.h"
#include "isxException.h"
#include <algorithm>
#include <cstring>

namespace isx
{

std::map<uint8_t, std::string> NVista3GpioFile::s_dataTypeMap =
{
    {0x01, "GPIO1"},
    {0x02, "GPIO2"},
    {0x03, "GPIO3"},
    {0x04, "GPIO4"},
    {0x05, "SYNC"},
    {0x06, "TRIG"},
    {0x07, "GPIO4_AI"},
    {0x08, "EX_LED"},
    {0x09, "OG_LED"},
    {0x0A, "DI_LED"},
// These are in the Python script, but I don't know if we need
// them here or should do anything with them.
// I found them while working on MOS-1365, but left them unused
// because I fixed that without them.
//    {0x36, "error"},
//    {0x37, "error"},
//    {0x54, "error"},
//    {0x76, "error"},
//    {0x85, "error"},
    {0x55, "sync_pkt"}
};

std::map<uint8_t, std::string> NVista3GpioFile::s_gpioModeMap =
{
    {0x00, "Output Manual Mode"},
    {0x01, "TTL Input Mode"},
    {0x02, "Output Pulse Train Mode"},
    {0x03, "Output Pulse Train Mode Inverted"}
};

std::map<uint8_t, std::string> NVista3GpioFile::s_ledGpioFollowMap =
{
    {0x00, "GPIO1"},
    {0x20, "GPIO2"},
    {0x40, "GPIO3"},
    {0x60, "GPIO4"}
};

std::map<uint8_t, std::string> NVista3GpioFile::s_ledStateMap =
{
    {0x00, "off"},
    {0x01, "on"},
    {0x02, "ramp_up"},
    {0x03, "ramp_down"}
};

std::map<uint8_t, std::string> NVista3GpioFile::s_ledModeMap =
{
    {0x00, "Manual Mode"},
    {0x01, "Manual Mode"},
    {0x02, "Manual Pulse Train Mode"},
    {0x03, "NA"},
    {0x04, "Analog Follow Mode"},
    {0x05, "GPIO Digital Follow Mode"},
    {0x06, "GPIO Triggered Pulse Train Mode"}

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
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open GPIO data file for reading: ", m_fileName);
    }

    // Get the size of the file in bytes
    m_file.seekg(0, m_file.end);
    m_fileSizeInBytes = m_file.tellg();
    m_file.seekg(0, m_file.beg);

    // In order to fix MOS-1090, we can guess with high probability if this an
    // nVista compressed raw file as the first 4 bytes should always be 0xfe 0xff 0xff 0xff.
    // Old versions of nVoke GPIO raw do not have an identifying signature,
    // but newer versions should always start with a sync packet (0x5d 0x55 0x55 0x55 ...)
    // so probability of false detection of nVista raw should be low.
    std::array<uint8_t, 4> buf;
    m_file.read((char *)buf.data(), 4);
    bool isNVistaRaw = buf[0] == 0xfe;
    for (size_t i = 1; isNVistaRaw && i < buf.size(); ++i)
    {
        isNVistaRaw = buf[i] == 0xff;
    }

    if (isNVistaRaw)
    {
        ISX_THROW(ExceptionFileIO, "Tried to read an nVoke GPIO raw file, but this looks like an nVista raw recording file, which is not supported.");
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

bool
NVista3GpioFile::isSyncPacket(uint8_t * data)
{
    SyncPkt * syncPkt = (SyncPkt *) data;
    return (*syncPkt == SyncPkt::syncValues());
}

AsyncTaskStatus
NVista3GpioFile::parse()
{
    float progress = 0.f;
    bool cancelled = false;

    std::vector<uint8_t> buf(MAX_PACKET_SIZE);
    std::ios::pos_type readPosition = 0;
    std::ios::pos_type lastPosition = m_fileSizeInBytes;
    bool syncPacketFound = false;

    while (!syncPacketFound && !m_file.eof() && !cancelled)
    {
        progress = (float)(readPosition) / (float)(lastPosition);
        if (m_checkInCB)
        {
            cancelled = m_checkInCB(progress);
        }

        m_file.seekg(readPosition);
        m_file.read((char *)buf.data(), SYNC_PKT_LENGTH);
        syncPacketFound = isSyncPacket(buf.data());

        if (!syncPacketFound)
        {
            readPosition += 1;
        }
    }

    if (!syncPacketFound)
    {
        ISX_THROW(isx::ExceptionFileIO,
                "Failed to find the beginning of the data stream and parse the file.");
    }

    readPosition += SYNC_PKT_LENGTH;

    std::vector<EventBasedFileV2::DataPkt> packetsToWrite;

    while (!m_file.eof() && !cancelled)
    {
        progress = (float)(readPosition) / (float)(lastPosition);
        if (m_checkInCB)
        {
            cancelled = m_checkInCB(progress);
        }

        if (lastPosition - readPosition < (int)sizeof(GenericPktHeader))
        {
            break;
        }

        m_file.seekg(readPosition);
        m_file.read((char *)buf.data(), sizeof(GenericPktHeader));
        m_file.seekg(readPosition);

        checkEventCounter(buf.data());

        switch ((SignalType) buf[0])
        {
            case SignalType::GPIO1:
            case SignalType::GPIO2:
            case SignalType::GPIO3:
            case SignalType::GPIO4:
            case SignalType::SYNC:
            case SignalType::TRIG:
            {
                if (lastPosition - readPosition >= (int)GPIO_PKT_LENGTH)
                {
                    m_file.read((char *)buf.data(), GPIO_PKT_LENGTH);
                    ISX_ASSERT(buf[1] == GPIO_PKT_LENGTH);
                    packetsToWrite.push_back(parseGpioPkt(buf));
                    readPosition = m_file.tellg();
                }
                else
                {
                    readPosition = lastPosition;
                }

                break;
            }
            case SignalType::EXLED:
            case SignalType::OGLED:
            case SignalType::DILED:
            {
                if (lastPosition - readPosition >= (int)LED_PKT_LENGTH)
                {
                    m_file.read((char *)buf.data(), LED_PKT_LENGTH);
                    ISX_ASSERT(buf[1] == LED_PKT_LENGTH);
                    packetsToWrite.push_back(parseLedPkt(buf));
                    readPosition = m_file.tellg();
                }
                else
                {
                    readPosition = lastPosition;
                }
                break;
            }
            case SignalType::GPIO4_AI:
            {
                if (lastPosition - readPosition >= (int)ANALOG_FOLLOW_PKT_LENGTH)
                {
                    m_file.read((char *)buf.data(), ANALOG_FOLLOW_PKT_LENGTH);
                    ISX_ASSERT(buf[1] == ANALOG_FOLLOW_PKT_LENGTH);
                    const auto analogFollowPackets = parseAnalogFollowPkt(buf);
                    packetsToWrite.insert(packetsToWrite.end(), analogFollowPackets.begin(), analogFollowPackets.end());
                    readPosition = m_file.tellg();
                }
                else
                {
                    readPosition = lastPosition;
                }
                break;
            }
            case SignalType::SYNCPKT:
            {
                readPosition += SYNC_PKT_LENGTH;
                break;
            }
            default:
            {
                readPosition += 1;
                break;
            }
        }

        if (readPosition > lastPosition)
        {
            readPosition = lastPosition;
        }
    }

    const DurationInSeconds step(1, 1000);
    const isize_t numTimes = isize_t(double(m_endTime - m_startTime) * 1E-6 / (step.toDouble())) + 1;
    const Time startTime(DurationInSeconds(isize_t(m_startTime), isize_t(1E6)));
    const TimingInfo timingInfo(startTime, step, numTimes);
    const Time endTime = timingInfo.getEnd();

    const size_t numChannels = m_analogSignalIds.size() + m_eventSignalIds.size();
    std::vector<std::string> channels(numChannels);
    std::vector<isx::SignalType> types(numChannels, isx::SignalType::SPARSE);
    for (auto & id : m_analogSignalIds)
    {
        channels.at(id.second) = s_dataTypeMap.at(id.first);
        types.at(id.second) = isx::SignalType::DENSE;
    }
    for (auto & id : m_eventSignalIds)
    {
        channels.at(id.second) = s_dataTypeMap.at(id.first);
        types.at(id.second) = isx::SignalType::SPARSE;
    }

    m_outputFileName = m_outputDir + "/" + isx::getBaseName(m_fileName) + "_gpio.isxd";
    const std::vector<DurationInSeconds> steps(channels.size(), step);
    m_outputFile.reset(new EventBasedFileV2(m_outputFileName, DataSet::Type::GPIO, channels, steps, types));
    for (const auto p : packetsToWrite)
    {
        m_outputFile->writeDataPkt(p);
    }

    m_outputFile->setTimingInfo(startTime, endTime);
    m_outputFile->closeFileForWriting();

    if (cancelled)
    {
        return isx::AsyncTaskStatus::CANCELLED;
    }

    return isx::AsyncTaskStatus::COMPLETE;
}

void
NVista3GpioFile::checkEventCounter(uint8_t * data)
{
    if (SignalType(data[0]) == SignalType::SYNCPKT)
    {
        return;
    }

    GenericPktHeader * hdr = (GenericPktHeader *) data;

    if (s_dataTypeMap.find(hdr->dataType) == s_dataTypeMap.end())
    {
        // Thought about logging an error, but think it might be too verbose.
        //ISX_LOG_ERROR("Unrecognized data type ", hdr->dataType, ".");
        return;
    }

    auto search = m_signalEventCounters.find(hdr->dataType);
    if (search != m_signalEventCounters.end())
    {
        if (m_signalEventCounters.at(hdr->dataType) == 255)
        {
            if(hdr->eventCounter != 0)
            {
                ISX_LOG_ERROR("Detected lost packets for ", s_dataTypeMap.at(hdr->dataType), " signal.");
            }
        }
        else
        {
            if(hdr->eventCounter != m_signalEventCounters.at(hdr->dataType) + 1)
            {
                ISX_LOG_ERROR("Detected lost packets for ", s_dataTypeMap.at(hdr->dataType), " signal.");
            }
        }
    }

    m_signalEventCounters[hdr->dataType] = hdr->eventCounter;
}

const std::string &
NVista3GpioFile::getOutputFileName() const
{
    return m_outputFileName;
}

EventBasedFileV2::DataPkt
NVista3GpioFile::parseGpioPkt(const std::vector<uint8_t> & inPkt)
{
    ISX_ASSERT(inPkt.size() >= GPIO_PKT_LENGTH);
    GpioPkt * pkt = (GpioPkt *) inPkt.data();

    if (m_eventSignalIds.find(pkt->header.dataType) == m_eventSignalIds.end())
    {
        uint64_t next = m_eventSignalIds.size() + m_analogSignalIds.size();
        m_eventSignalIds[pkt->header.dataType] = next;
    }

    float state = (pkt->stateMode & GPIO_STATE_MASK) > 0 ? 1.f : 0.f;

    uint64_t usecs = getUsecs(pkt->timeSecs, pkt->timeUSecs);

    return EventBasedFileV2::DataPkt(usecs, state, m_eventSignalIds.at(pkt->header.dataType));
}

EventBasedFileV2::DataPkt
NVista3GpioFile::parseLedPkt(const std::vector<uint8_t> & inPkt)
{
    ISX_ASSERT(inPkt.size() >= LED_PKT_LENGTH);
    LedPkt * pkt = (LedPkt *) inPkt.data();

    if (m_eventSignalIds.find(pkt->header.dataType) == m_eventSignalIds.end())
    {
        uint64_t next = m_eventSignalIds.size() + m_analogSignalIds.size();
        m_eventSignalIds[pkt->header.dataType] = next;
    }
    uint16_t power1 = (uint16_t)pkt->ledPower & 0x00FF;;
    uint16_t power2 = uint16_t(pkt->powerState & LED_POWER_0_MASK);
    uint16_t power = (power1 << 1) + (power2 >> 4);
    double dPower = (double)power / 10.0; // In units of mW/mm^2, in the range of [0, 51.1]

    uint64_t usecs = getUsecs(pkt->timeSecs, pkt->timeUSecs);

    return EventBasedFileV2::DataPkt(usecs, float(dPower), m_eventSignalIds.at(pkt->header.dataType));
}

std::vector<EventBasedFileV2::DataPkt>
NVista3GpioFile::parseAnalogFollowPkt(const std::vector<uint8_t> & inPkt)
{
    ISX_ASSERT(inPkt.size() >= ANALOG_FOLLOW_PKT_LENGTH);
    AnalogFollowPkt * pkt = (AnalogFollowPkt *) inPkt.data();

    if (m_analogSignalIds.find(pkt->header.dataType) == m_analogSignalIds.end())
    {
        uint64_t next = m_eventSignalIds.size() + m_analogSignalIds.size();
        m_analogSignalIds[pkt->header.dataType] = next;
    }
    uint8_t maxPower = (pkt->ogMaxPower[0] & AF_POWER_LAST) | (pkt->ogMaxPower[1] & AF_POWER_FIRST);

    uint64_t usecs = getUsecs(pkt->timeSecs, pkt->timeUSecs);

    std::vector<EventBasedFileV2::DataPkt> packets;
    for (isize_t i(0); i < 10; ++i)
    {
        uint16_t hSample = (uint16_t)pkt->analogSamples[2*i] & 0x00FF;
        uint16_t lSample = (uint16_t)pkt->analogSamples[2*i + 1] & 0x00FF;;
        uint16_t sample = (hSample << 8) + lSample;
        double dSample = double(sample) / 655360.0 * double(maxPower);   // In units of mW/mm^2

        uint64_t usecsForPkt = usecs + (uint64_t)(1000*i);
        m_endTime = m_startTime + usecsForPkt;

        packets.push_back(EventBasedFileV2::DataPkt(usecsForPkt, float(dSample), m_analogSignalIds.at(pkt->header.dataType)));
    }
    return packets;
}

uint64_t
NVista3GpioFile::getUsecs(uint8_t * inSecs, uint8_t * inUSecs)
{
    uint64_t outUsecs = 0;

    uint64_t unixTimeInSecs = 0;
    for (isize_t i(0); i < 4; ++i)
    {
        uint64_t bytedata = (uint64_t)inSecs[i] & 0x00000000000000FF;
        unixTimeInSecs += (bytedata << (3 - i) * 8);
    }

    outUsecs += unixTimeInSecs * 1000000;

    for(isize_t i(0); i < 3; ++i)
    {
        uint64_t bytedata = (uint64_t)inUSecs[i] & 0x00000000000000FF;

        if(i == 0)
        {
            bytedata &= 0x000000000000000F;
        }

        outUsecs += (bytedata << (2-i)*8);
    }

    // Adjust values to be offsets from start times
    if (!m_startTimeSet)
    {
        m_startTime = outUsecs;
        m_startTimeSet = true;
        outUsecs = 0;
    }
    else
    {
        m_endTime = outUsecs;
        outUsecs -= m_startTime;
    }

    return outUsecs;
}

} // namespace isx
