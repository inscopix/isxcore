#include "isxNVista3GpioFile.h"
#include "isxPathUtils.h"
#include "isxJsonUtils.h"
#include "isxException.h"
#include <algorithm>
#include <cstring>

namespace
{

void
checkPayloadSize(const size_t inActual, const size_t inExpected)
{
    if (inActual != inExpected)
    {
        ISX_LOG_DEBUG("Unexpected payload size: ", inActual, " != ", inExpected);
    }
}

} // namespace

namespace isx
{

const std::map<NVista3GpioFile::Channel, std::string> NVista3GpioFile::s_channelNames
{
    {NVista3GpioFile::Channel::FRAME_COUNTER, "Frame Count"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_0, "Digital GPI 0"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_1, "Digital GPI 1"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_2, "Digital GPI 2"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_3, "Digital GPI 3"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_4, "Digital GPI 4"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_5, "Digital GPI 5"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_6, "Digital GPI 6"},
    {NVista3GpioFile::Channel::DIGITAL_GPI_7, "Digital GPI 7"},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_1, "BNC GPIO IN 1"},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_2, "BNC GPIO IN 2"},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_3, "BNC GPIO IN 3"},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_4, "BNC GPIO IN 4"},
    {NVista3GpioFile::Channel::EX_LED, "EX_LED"},
    {NVista3GpioFile::Channel::OG_LED, "OG_LED"},
    {NVista3GpioFile::Channel::DI_LED, "DI_LED"},
    {NVista3GpioFile::Channel::EFOCUS, "efocus"},
    {NVista3GpioFile::Channel::TRIG, "TRIG"},
    {NVista3GpioFile::Channel::SYNC, "SYNC"},
    {NVista3GpioFile::Channel::FLASH, "FLASH"},
    {NVista3GpioFile::Channel::BNC_TRIG, "BNC TRIG"},
    {NVista3GpioFile::Channel::BNC_SYNC, "BNC SYNC"},
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
    if (m_packets.empty())
    {
        m_firstTime = inTimeStamp;
    }

    if (m_indices.find(inChannel) == m_indices.end())
    {
        m_indices[inChannel] = m_indices.size();
    }
    const EventBasedFileV2::DataPkt pkt(inTimeStamp, inValue, m_indices[inChannel]);
    m_packets.push_back(pkt);
    if (inChannel == Channel::BNC_GPIO_IN_2)
    {
        ISX_LOG_DEBUG("Wrote packet : ", inTimeStamp, ", ", inValue);
    }
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
//    uint32_t trigSyncFlash;
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
        ISX_LOG_DEBUG("At byte ", m_file.tellg());

        //ISX_LOG_DEBUG("Found sync");

        read(header);
        //ISX_LOG_DEBUG("Read packet type ", header.m_type);
        if ((header.m_type >> 8) != s_eventSignature)
        {
            ISX_LOG_DEBUG("Found non-event header");
            continue;
        }
        //ISX_LOG_DEBUG("Read sequence ", header.m_sequence);
        //ISX_LOG_DEBUG("Read payloadSize ", header.m_payloadSize);

        if (Event(header.m_type) != Event::WAVEFORM)
        {
            read(tscHigh);
            read(tscLow);
            ISX_LOG_DEBUG("Read TSC high ", tscHigh);
            ISX_LOG_DEBUG("Read TSC low ", tscLow);
            tsc = (uint64_t(tscHigh) << 32) | uint64_t(tscLow);
            ISX_LOG_DEBUG("Got TSC ", tsc);

            read(fc);
            //ISX_LOG_DEBUG("Read FC ", fc);
        }

        switch (Event(header.m_type))
        {
            case Event::CAPTURE_ALL:
                ISX_LOG_DEBUG("Event::CAPTURE_ALL : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 10);
                addDigitalGpiPkts(tsc, read(digitalGpi));
                skipBytes(2);
                addPkt(Channel::BNC_GPIO_IN_1, tsc, float(read(bncGpioIn1)));
                addPkt(Channel::BNC_GPIO_IN_2, tsc, float(read(bncGpioIn2)));
                addPkt(Channel::BNC_GPIO_IN_3, tsc, float(read(bncGpioIn3)));
                addPkt(Channel::BNC_GPIO_IN_4, tsc, float(read(bncGpioIn4)));
                addPkt(Channel::EX_LED, tsc, float(read(exLed)));
                addPkt(Channel::OG_LED, tsc, float(read(ogLed)));
                addPkt(Channel::DI_LED, tsc, float(read(diLed)));
                skipBytes(2);
                addPkt(Channel::EFOCUS, tsc, float(read(eFocus)));
                skipBytes(2);
//                read(trigSyncFlash);
//                for (const auto channel : std::vector<Channel>({Channel::TRIG, Channel::SYNC, Channel::FLASH}))
//                {
//                    addPkt(channel, tsc, trigSyncFlash & 0b1);
//                    trigSyncFlash >>= 1;
//                }
                break;

            case Event::CAPTURE_GPIO:
                ISX_LOG_DEBUG("Event::CAPTURE_GPIO : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 6);
                addDigitalGpiPkts(tsc, read(digitalGpi));
                addPkt(Channel::BNC_TRIG, tsc, float(read(bncTrig)));
                addPkt(Channel::BNC_GPIO_IN_1, tsc, float(read(bncGpioIn1)));
                addPkt(Channel::BNC_GPIO_IN_2, tsc, float(read(bncGpioIn2)));
                addPkt(Channel::BNC_GPIO_IN_3, tsc, float(read(bncGpioIn3)));
                addPkt(Channel::BNC_GPIO_IN_4, tsc, float(read(bncGpioIn4)));
                break;

            case Event::BNC_GPIO_IN_1:
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::BNC_GPIO_IN_1, tsc, float(read(bncGpioIn1)));
                skipBytes(2);
                break;

            case Event::BNC_GPIO_IN_2:
                ISX_LOG_DEBUG("Event::BNC_GPIO_IN_2 : ", header.m_sequence);
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::BNC_GPIO_IN_2, tsc, float(read(bncGpioIn2)));
                skipBytes(2);
                break;

            case Event::BNC_GPIO_IN_3:
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::BNC_GPIO_IN_3, tsc, float(read(bncGpioIn3)));
                skipBytes(2);
                break;

            case Event::BNC_GPIO_IN_4:
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::BNC_GPIO_IN_4, tsc, float(read(bncGpioIn4)));
                skipBytes(2);
                break;

            case Event::DIGITAL_GPI:
                checkPayloadSize(header.m_payloadSize, 4);
                addDigitalGpiPkts(tsc, read(digitalGpi));
                skipBytes(2);
                break;

            case Event::EX_LED:
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::EX_LED, tsc, float(read(exLed)));
                skipBytes(2);
                break;

            case Event::OG_LED:
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::OG_LED, tsc, float(read(ogLed)));
                skipBytes(2);
                break;

            case Event::DI_LED:
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::DI_LED, tsc, float(read(diLed)));
                skipBytes(2);
                break;

            case Event::FRAME_COUNT:
                checkPayloadSize(header.m_payloadSize, 3);
                addPkt(Channel::FRAME_COUNTER, tsc, float(fc));
                break;

            case Event::BNC_TRIG:
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::BNC_TRIG, tsc, float(read(bncTrig)));
                skipBytes(2);
                break;

            case Event::BNC_SYNC:
                checkPayloadSize(header.m_payloadSize, 4);
                addPkt(Channel::BNC_SYNC, tsc, float(read(bncSync)));
                skipBytes(2);
                break;

            case Event::WAVEFORM:
                checkPayloadSize(header.m_payloadSize, 2);
                skipWords(header.m_payloadSize);
                break;

            default:
                break;
        }
    }

    ISX_LOG_DEBUG("syncCount = ", syncCount);

    if (syncCount == 0)
    {
        ISX_THROW(ExceptionFileIO, "Failed to find the beginning of the data stream and parse the file.");
    }

    // TODO : refactor with nVoke.
    uint64_t lastTime = m_firstTime;
    if (!m_packets.empty())
    {
        lastTime = m_packets.back().offsetMicroSecs;
    }

    const auto step = DurationInSeconds::fromMicroseconds(1);
    const isize_t numTimes = isize_t(double(lastTime - m_firstTime) * 1E-6 / (step.toDouble())) + 1;
    const Time startTime(DurationInSeconds::fromMicroseconds(m_firstTime));
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
