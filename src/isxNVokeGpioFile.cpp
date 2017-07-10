#include "isxNVokeGpioFile.h"
#include "isxPathUtils.h"
#include "isxJsonUtils.h"
#include "isxException.h"
#include <algorithm>
#include <cstring>

namespace isx
{

std::map<uint8_t, std::string> NVokeGpioFile::s_dataTypeMap = 
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
    {0x55, "sync_pkt"}
};

std::map<uint8_t, std::string> NVokeGpioFile::s_gpioModeMap = 
{
    {0x00, "Output Manual Mode"},
    {0x01, "TTL Input Mode"},
    {0x02, "Output Pulse Train Mode"},
    {0x03, "Output Pulse Train Mode Inverted"}
};

std::map<uint8_t, std::string> NVokeGpioFile::s_ledGpioFollowMap = 
{
    {0x00, "GPIO1"},
    {0x20, "GPIO2"},
    {0x40, "GPIO3"},
    {0x60, "GPIO4"}
};

std::map<uint8_t, std::string> NVokeGpioFile::s_ledStateMap = 
{
    {0x00, "off"},
    {0x01, "on"},
    {0x02, "ramp_up"},
    {0x03, "ramp_down"}
};

std::map<uint8_t, std::string> NVokeGpioFile::s_ledModeMap = 
{
    {0x00, "Manual Mode"},
    {0x01, "Manual Mode"},
    {0x02, "Manual Pulse Train Mode"},
    {0x03, "NA"},
    {0x04, "Analog Follow Mode"},
    {0x05, "GPIO Digital Follow Mode"},
    {0x06, "GPIO Triggered Pulse Train Mode"}

};


NVokeGpioFile::NVokeGpioFile()
{

}

NVokeGpioFile::NVokeGpioFile(const std::string & inFileName, const std::string & inOutputDir)
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

    m_valid = true;
}

NVokeGpioFile::~NVokeGpioFile()
{
    if(m_file.is_open() && m_file.good())
    {
        m_file.close();
        if (!m_file.good())
        {
            ISX_LOG_ERROR("Error closing the stream for file", m_fileName,
            " eof: ", m_file.eof(), 
            " bad: ", m_file.bad(), 
            " fail: ", m_file.fail());
        }
    }
}

bool 
NVokeGpioFile::isValid()
{
    return m_valid;
}

const std::string & 
NVokeGpioFile::getFileName()
{
    return m_fileName;
}



void 
NVokeGpioFile::setCheckInCallback(AsyncCheckInCB_t inCheckInCB)
{
    m_checkInCB = inCheckInCB;
}

bool 
NVokeGpioFile::isSyncPacket(uint8_t * data)
{
    SyncPkt * syncPkt = (SyncPkt *) data;
    return (*syncPkt == SyncPkt::syncValues());
}

AsyncTaskStatus 
NVokeGpioFile::parse()
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
        if(m_checkInCB)
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

    while (!m_file.eof() && !cancelled)
    {
        progress = (float)(readPosition) / (float)(lastPosition);
        if(m_checkInCB)
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
                    parseGpioPkt(buf);
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
                    parseLedPkt(buf);
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
                    parseAnalogFollowPkt(buf);
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

    closeFiles();

    if (cancelled)
    {
        return isx::AsyncTaskStatus::CANCELLED;
    }

    return isx::AsyncTaskStatus::COMPLETE;
       
}

void 
NVokeGpioFile::closeAnalogFile()
{
    if(m_analogFile)
    {
        m_analogFile->setTimingInfo(m_timingInfo);
        m_analogFile->closeFileForWriting();
    }
}

void 
NVokeGpioFile::closeEventsFile()
{
    if(!m_eventData.empty())
    {
        std::string filename = m_outputDir + "/" + isx::getBaseName(m_fileName) + "_events.isxd";
        
        std::unique_ptr<TimeStampedDataFile> eventsFile(new TimeStampedDataFile(filename, TimeStampedDataFile::StoredData::GPIO));

        for (auto & channel : m_eventData)
        {
            auto & info = channel.second;
            eventsFile->writeChannelHeader(
                channel.first,
                info.outputMode,
                info.triggerFollow,
                info.data.size());

            for (auto & pkt : info.data)
            {
                eventsFile->writeDataPkt(pkt);
            }            
        }

        
        eventsFile->setTimingInfo(m_timingInfo);
        eventsFile->closeFileForWriting(); 

        m_outputFileNames.push_back(filename);
    }
}

void 
NVokeGpioFile::closeFiles()
{
    updateTimingInfo();
    closeAnalogFile();
    closeEventsFile();
}

void 
NVokeGpioFile::checkEventCounter(uint8_t * data)
{
    if((SignalType)data[0] == SignalType::SYNCPKT)
    {
        return;
    }

    GenericPktHeader * hdr = (GenericPktHeader *) data;
    auto search = m_signalEventCounters.find(hdr->dataType);
    if(search != m_signalEventCounters.end())
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


void 
NVokeGpioFile::getOutputFileNames(std::vector<std::string> & outFileNames)
{
    outFileNames = m_outputFileNames; 
}


void 
NVokeGpioFile::writeAnalogPktToFile(
    const std::string & inChannel, 
    const std::string & inMode,
    const std::string & inTriggerFollow, 
    const TimeStampedDataFile::DataPkt & inData)
{    
    // If we haven't started writing the file, open it in write mode and create the json header
    if(!m_analogFile)
    {
        // Get destination file name
        std::string filename = m_outputDir + "/" + isx::getBaseName(m_fileName) + "_analog.isxd";
        m_outputFileNames.push_back(filename);

        m_analogFile.reset(new TimeStampedDataFile(filename, TimeStampedDataFile::StoredData::GPIO));
        m_analogFile->writeChannelHeader(
            inChannel,
            inMode,
            inTriggerFollow,
            0);
    }

    // Write binary data
    m_analogFile->writeDataPkt(inData);
    
}

void 
NVokeGpioFile::addEventPkt(
            const std::string & inChannel, 
            const std::string & inMode,
            const std::string & inTriggerFollow, 
            const TimeStampedDataFile::DataPkt & inData)
{
    auto search = m_eventData.find(inChannel);

    if(search == m_eventData.end())
    {       
        ChannelInfo info(inMode, inTriggerFollow, inData);
        m_eventData[inChannel] = info;
    }
    else
    {
        auto & info = m_eventData[inChannel];
        info.data.push_back(inData);
        // Check that the acquisition mode is not changed in the middle of the recording
        ISX_ASSERT(info.outputMode == inMode);
    }
}

void 
NVokeGpioFile::parseGpioPkt(const std::vector<uint8_t> & inPkt)
{
    ISX_ASSERT(inPkt.size() >= GPIO_PKT_LENGTH);
    GpioPkt * pkt = (GpioPkt *) inPkt.data();

    std::string dataType = s_dataTypeMap.at(pkt->header.dataType);
    bool state = (pkt->stateMode & GPIO_STATE_MASK) > 0 ? true : false;
    std::string mode = s_gpioModeMap.at((pkt->stateMode & GPIO_MODE_MASK));
    std::string triggerFollow; 

    uint64_t usecs = getUsecs(pkt->timeSecs, pkt->timeUSecs);    

    TimeStampedDataFile::DataPkt data(usecs, state, 0.0f);
    updatePktTimes(data);
    addEventPkt(dataType, mode, triggerFollow, data);
}

void 
NVokeGpioFile::parseLedPkt(const std::vector<uint8_t> & inPkt)
{
    ISX_ASSERT(inPkt.size() >= LED_PKT_LENGTH);
    LedPkt * pkt = (LedPkt *) inPkt.data();

    std::string dataType = s_dataTypeMap.at(pkt->header.dataType);
    uint16_t power1 = (uint16_t)pkt->ledPower & 0x00FF;;
    uint16_t power2 = uint16_t(pkt->powerState & LED_POWER_0_MASK);
    uint16_t power = (power1 << 1) + (power2 >> 4); 
    double   dPower = (double)power / 10.0; // In units of mW/mm^2, in the range of [0, 51.1]

    std::string triggerFollow = s_ledGpioFollowMap.at((pkt->followMode & LED_GPIO_FOLLOW_MASK) >> 4);
    bool state = (pkt->powerState & LED_STATE_MASK) > 0 ? true : false;
    std::string mode = s_ledModeMap.at((pkt->followMode & LED_MODE_MASK));
    
    uint64_t usecs = getUsecs(pkt->timeSecs, pkt->timeUSecs);

    TimeStampedDataFile::DataPkt data(usecs, state, float(dPower));
    updatePktTimes(data);
    addEventPkt(dataType, mode, triggerFollow, data);
}

void 
NVokeGpioFile::parseAnalogFollowPkt(const std::vector<uint8_t> & inPkt)
{
    ISX_ASSERT(inPkt.size() >= ANALOG_FOLLOW_PKT_LENGTH);
    AnalogFollowPkt * pkt = (AnalogFollowPkt *) inPkt.data();

    std::string dataType = s_dataTypeMap.at(pkt->header.dataType);
    uint8_t maxPower = (pkt->ogMaxPower[0] & AF_POWER_LAST) | (pkt->ogMaxPower[1] & AF_POWER_FIRST);
    
    uint64_t usecs = getUsecs(pkt->timeSecs, pkt->timeUSecs);

    for (isize_t i(0); i < 10; ++i)
    {
        uint16_t hSample = (uint16_t)pkt->analogSamples[2*i] & 0x00FF;
        uint16_t lSample = (uint16_t)pkt->analogSamples[2*i + 1] & 0x00FF;;
        uint16_t sample = (hSample << 8) + lSample;
        double dSample = double(sample) / 655360.0 * double(maxPower);   // In units of mW/mm^2

        uint64_t usecsForPkt = usecs + (uint64_t)(1000*i);
        TimeStampedDataFile::DataPkt data(usecsForPkt, false, float(dSample));
        updatePktTimes(data);
        writeAnalogPktToFile(dataType, std::string(), std::string(), data);
    }
}

uint64_t 
NVokeGpioFile::getUsecs(uint8_t * inSecs, uint8_t * inUSecs)
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

    return outUsecs;
}

void 
NVokeGpioFile::updatePktTimes(const TimeStampedDataFile::DataPkt & inData)
{
    if(!m_startTimeSet)
    {
        m_startTime = inData.getTime();
        m_startTimeSet = true;
    }
    else
    {
        m_endTime = inData.getTime();
    }
}

void NVokeGpioFile::updateTimingInfo()
{
    DurationInSeconds step(1, 1000);
    isize_t numTimes = (isize_t)((m_endTime.getSecsSinceEpoch().toDouble() - m_startTime.getSecsSinceEpoch().toDouble()) / (step.toDouble())) + 1;
    m_timingInfo = TimingInfo(m_startTime, step, numTimes);   
}


}
