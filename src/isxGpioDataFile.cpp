#include "isxGpioDataFile.h"
#include "isxPathUtils.h"
#include "isxJsonUtils.h"
#include "isxException.h"
#include <algorithm>
#include <cstring>

namespace isx
{

std::map<uint8_t, std::string> GpioDataFile::s_dataTypeMap = 
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

std::map<uint8_t, std::string> GpioDataFile::s_gpioModeMap = 
{
    {0x00, "Output Manual Mode"},
    {0x01, "TTL Input Mode"},
    {0x02, "Output Pulse Train Mode"},
    {0x03, "Output Pulse Train Mode Inverted"}
};

std::map<uint8_t, std::string> GpioDataFile::s_ledGpioFollowMap = 
{
    {0x00, "GPIO1"},
    {0x20, "GPIO2"},
    {0x40, "GPIO3"},
    {0x60, "GPIO4"}
};

std::map<uint8_t, std::string> GpioDataFile::s_ledStateMap = 
{
    {0x00, "off"},
    {0x01, "on"},
    {0x02, "ramp_up"},
    {0x03, "ramp_down"}
};

std::map<uint8_t, std::string> GpioDataFile::s_ledModeMap = 
{
    {0x00, "Manual Mode"},
    {0x01, "Manual Mode"},
    {0x02, "Manual Pulse Train Mode"},
    {0x03, "NA"},
    {0x04, "Analog Follow Mode"},
    {0x05, "GPIO Digital Follow Mode"},
    {0x06, "GPIO Triggered Pulse Train Mode"}

};


GpioDataFile::GpioDataFile()
{

}

GpioDataFile::GpioDataFile(const std::string & inFileName, const std::string & inOutputDir)
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

GpioDataFile::~GpioDataFile()
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
GpioDataFile::isValid()
{
    return m_valid;
}

const std::string & 
GpioDataFile::getFileName()
{
    return m_fileName;
}



void 
GpioDataFile::setCheckInCallback(AsyncCheckInCB_t inCheckInCB)
{
    m_checkInCB = inCheckInCB;
}

bool 
GpioDataFile::isSyncPacket(uint8_t * data)
{
    SyncPkt * syncPkt = (SyncPkt *) data;
    return (*syncPkt == SyncPkt::syncValues());
}

AsyncTaskStatus 
GpioDataFile::parse()
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
                    writeGpioPkt(buf);
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
                    writeLedPkt(buf);
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
                    writeAnalogFollowPkt(buf);
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
GpioDataFile::closeFiles()
{
    // Close all output files
    for (auto it = m_outputFiles.begin(); it != m_outputFiles.end(); ++it) 
    {
        it->second->close();
        delete it->second;
    }
}

void 
GpioDataFile::checkEventCounter(uint8_t * data)
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
GpioDataFile::getOutputFileNames(std::vector<std::string> & outFileNames)
{
    outFileNames.clear();
    for (auto it = m_outputFiles.begin(); it != m_outputFiles.end(); ++it) 
    {
        outFileNames.push_back(it->first);
    }
}


void 
GpioDataFile::writeToFile(
    const std::string & inFileSuffix, 
    const std::string & inMode,
    const std::string & inTriggerFollow, 
    const uint32_t inTimeStampSec, 
    const uint32_t inTimeStampUSec, 
    bool inState, 
    double inPowerLevel)
{
    // Get destination file name
    std::string filename = m_outputDir + "/" + isx::getBaseName(m_fileName) + "_" + inFileSuffix + ".isxd";

    auto search = m_outputFiles.find(filename);

    // If we haven't started writing the file, open it in write mode and write the json header
    std::ofstream * file;
    if(search == m_outputFiles.end())
    {
        file = new std::ofstream(filename, std::ios::binary | std::ios::trunc | std::ios::out);
        if (!file->good() || !file->is_open())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to open output file for write: ", filename);
        }
        writeHeader(*file, inFileSuffix, inMode, inTriggerFollow);
        m_outputFiles[filename] = file;
        m_outputModes[filename] = inMode;
    }
    else
    {
        file = m_outputFiles.at(filename);

        // Check that the acquisition mode is not changed in the middle of the recording
        ISX_ASSERT(m_outputModes.at(filename) == inMode);
        
    }

    // Write binary data
    char state = 0;
    if(inState)
    {
        state = 1;
    }

    file->write((char *) &inTimeStampSec, sizeof(inTimeStampSec));
    file->write((char *) &inTimeStampUSec, sizeof(inTimeStampUSec));
    file->write((char *) &state, sizeof(state));
    file->write((char *) &inPowerLevel, sizeof(inPowerLevel));

    if (!file->good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Error writing output data file: ", filename);
    }
    file->flush();

}

void 
GpioDataFile::writeHeader(
            std::ofstream & file, 
            const std::string & inSignal, 
            const std::string & inMode,
            const std::string & inTriggerFollow)
{
    json j;
    try
    {
        j["type"] = size_t(DataSet::Type::GPIO);
        j["signal"] = inSignal;
        j["mode"] = inMode;
        j["GPIO Trigger/Follow"] = inTriggerFollow;        
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO,
            "Unknown error while generating file header.");
    }

// TODO: MOS-584 merge fix
//    writeJsonHeader(j, file);
    file.flush();
}

void 
GpioDataFile::writeGpioPkt(const std::vector<uint8_t> & inPkt)
{
    ISX_ASSERT(inPkt.size() >= GPIO_PKT_LENGTH);
    GpioPkt * pkt = (GpioPkt *) inPkt.data();

    std::string dataType = s_dataTypeMap.at(pkt->header.dataType);
    bool state = (pkt->stateMode & GPIO_STATE_MASK) > 0 ? true : false;
    std::string mode = s_gpioModeMap.at((pkt->stateMode & GPIO_MODE_MASK));
    std::string triggerFollow; 

    uint32_t unixTimeInSecs = getUnixTimeInSecs(pkt->timeSecs);
    uint32_t usecs = getUsecs(pkt->timeUSecs);

    writeToFile(dataType, mode, triggerFollow, unixTimeInSecs, usecs, state, 0.0);

}

void 
GpioDataFile::writeLedPkt(const std::vector<uint8_t> & inPkt)
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
    
    uint32_t unixTimeInSecs = getUnixTimeInSecs(pkt->timeSecs);
    uint32_t usecs = getUsecs(pkt->timeUSecs);

    writeToFile(dataType, mode, triggerFollow, unixTimeInSecs, usecs, state, dPower);

}

void 
GpioDataFile::writeAnalogFollowPkt(const std::vector<uint8_t> & inPkt)
{
    ISX_ASSERT(inPkt.size() >= ANALOG_FOLLOW_PKT_LENGTH);
    AnalogFollowPkt * pkt = (AnalogFollowPkt *) inPkt.data();

    std::string dataType = s_dataTypeMap.at(pkt->header.dataType);
    uint8_t maxPower = (pkt->ogMaxPower[0] & AF_POWER_LAST) | (pkt->ogMaxPower[1] & AF_POWER_FIRST);
    
    uint32_t unixTimeInSecs = getUnixTimeInSecs(pkt->timeSecs);
    uint32_t usecs = getUsecs(pkt->timeUSecs);

    for (isize_t i(0); i < 10; ++i)
    {
        uint16_t hSample = (uint16_t)pkt->analogSamples[2*i] & 0x00FF;
        uint16_t lSample = (uint16_t)pkt->analogSamples[2*i + 1] & 0x00FF;;
        uint16_t sample = (hSample << 8) + lSample;
        double dSample = double(sample) / 655360.0 * double(maxPower);   // In units of mW/mm^2
        writeToFile(dataType, std::string(), std::string(), unixTimeInSecs, usecs + (uint32_t)(1000*i), false, dSample);
    }
}

uint32_t 
GpioDataFile::getUnixTimeInSecs(uint8_t * inTime)
{
    uint32_t outUnixTimeInSecs = 0;
    for(isize_t i(0); i < 4; ++i)
    {
        uint32_t bytedata = (uint32_t)inTime[i] & 0x000000FF;
        outUnixTimeInSecs += (bytedata << (3-i)*8);
    }
    return outUnixTimeInSecs;
}

uint32_t 
GpioDataFile::getUsecs(uint8_t * inTime)
{
    uint32_t outUsecs = 0;
    for(isize_t i(0); i < 3; ++i)
    {
        uint32_t bytedata = (uint32_t)inTime[i] & 0x000000FF;
        
        if(i == 0)
        {
            bytedata &= 0x0000000F;
        }

        outUsecs += (bytedata << (2-i)*8);
    }

    return outUsecs;
}


}