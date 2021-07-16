#include "isxEventBasedFileV1.h"
#include "isxJsonUtils.h"
#include <limits>

namespace isx
{

EventBasedFileV1::DataPkt::DataPkt()
{

}

EventBasedFileV1::DataPkt::DataPkt(
    const uint64_t inTimeStampUSec,
    const bool inState,
    const float inValue) :
    m_timeStampUSec(inTimeStampUSec)
{
    float val = std::abs(inValue);
    if (inState)
    {
        val = -val;
    }
    uint32_t * newVal = (uint32_t *)&val;
    m_value = *newVal;

    m_reserved = 0;
}

bool
EventBasedFileV1::DataPkt::getState()
{
    uint32_t state = m_value & 0x80000000;
    return (state != 0);
}

float
EventBasedFileV1::DataPkt::getValue()
{
    float * fval = reinterpret_cast<float *>(&m_value);
    return std::abs(*fval);
}

Time
EventBasedFileV1::DataPkt::getTime() const
{
    DurationInSeconds secsFromUnixEpoch(isize_t(m_timeStampUSec / 1000000), 1);
    DurationInSeconds usecs(m_timeStampUSec % 1000000, 1000000);
    secsFromUnixEpoch += usecs;
    return Time(secsFromUnixEpoch);
}


EventBasedFileV1::EventBasedFileV1()
{

}

EventBasedFileV1::EventBasedFileV1(const std::string & inFileName) :
    m_fileName(inFileName),
    m_openForWrite(false)
{
    m_file.open(m_fileName, std::ios::binary | std::ios_base::in);
    if (!m_file.good() || !m_file.is_open())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open file: ", m_fileName);
    }
    readFileFooter();
    m_valid = true;
}

EventBasedFileV1::EventBasedFileV1(const std::string & inFileName, StoredData dataType, bool inIsAnalog) :
    m_analog(false),
    m_fileName(inFileName),
    m_openForWrite(true),
    m_dataType(dataType)
{
    m_file.open(m_fileName, std::ios::binary | std::ios_base::out);
    if (!m_file.good() || !m_file.is_open())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open file: ", m_fileName);
    }

    m_valid = true;

    if (m_dataType == StoredData::GPIO)
    {
        m_analog = inIsAnalog;
    }

}

EventBasedFileV1::~EventBasedFileV1()
{
    closeFileForWriting();
    isx::closeFileStreamWithChecks(m_file, m_fileName);
}

bool
EventBasedFileV1::isValid() const
{
    return m_valid;
}

EventBasedFileV1::StoredData
EventBasedFileV1::getStoredDataType() const
{
    return m_dataType;
}

bool
EventBasedFileV1::isAnalog() const
{
    return m_analog;
}

const std::string &
EventBasedFileV1::getFileName() const
{
    return m_fileName;
}

const std::vector<std::string> 
EventBasedFileV1::getChannelList() const
{
    std::vector<std::string> keys;

    for (auto & pair : m_channelOffsets)
    {
        keys.push_back(pair.first);
    }

    return keys;
}

SpFTrace_t
EventBasedFileV1::getAnalogData(const std::string & inChannelName)
{
    if(!m_analog || m_openForWrite)
    {
        return nullptr;
    }

    auto search = m_channelOffsets.find(inChannelName);
    if(search == m_channelOffsets.end() || m_openForWrite)
    {
        return nullptr;
    }

    std::string headerStr = readChannelHeader(inChannelName);
    json header = json::parse(headerStr);
    isize_t numPkts = header.at("Number of Packets");

    if (numPkts == 0)
    {
        // Calculate number of packets
        std::ios::pos_type current = m_file.tellg();
        isize_t dataBytes = isize_t(m_headerOffset - current);
        numPkts = dataBytes / sizeof(DataPkt);
        ISX_ASSERT(dataBytes == numPkts * sizeof(DataPkt));
    }


    std::unique_ptr<DataPkt[]> data(new DataPkt[numPkts]);
    m_file.read(reinterpret_cast<char*>(data.get()), numPkts * sizeof(DataPkt));

    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error reading file.");
    }

    SpFTrace_t trace = std::make_shared<FTrace_t>(m_timingInfo);
    std::vector<isize_t> droppedFrames;
    isize_t prevIndex = 0;

    for(isize_t i(0); i < numPkts; ++i)
    {
        DataPkt & pkt = data[i];
        Time time = pkt.getTime();
        isize_t index = m_timingInfo.convertTimeToIndex(time);

        // There are numerical errors in the conversion from time to index.
        // We know that samples in the data file are sorted by time, so we would never
        // allow for the index to be less than the prevIndex. If this happens, we get into a
        // nerver-ending loop. Remove this when we have a better conversion of time to index.
        if (index <= prevIndex && prevIndex != 0)
        {
            index = prevIndex + 1;
            if (index >= m_timingInfo.getNumTimes())
            {
                break;
            }
        }

        trace->setValue(index, pkt.getValue());

        if((i == 0) && (index != 0))
        {
            isize_t dropped = 0;
            while (dropped != index)
            {
                droppedFrames.push_back(dropped);
                ++dropped;
            }
        }
        else if ((i != 0) &&  (index != prevIndex + 1))
        {
            isize_t dropped = prevIndex + 1;
            while (dropped != index)
            {
                droppedFrames.push_back(dropped);
                ++dropped;
            }
        }
        prevIndex = index;
    }

    for(isize_t i(0); i < droppedFrames.size(); ++i)
    {
        trace->setValue(droppedFrames.at(i), std::numeric_limits<float>::quiet_NaN());
    }

    trace->setDroppedFrames(droppedFrames);

    return trace;
}

SpLogicalTrace_t
EventBasedFileV1::getLogicalData(const std::string & inChannelName)
{
    auto search = m_channelOffsets.find(inChannelName);
    if(search == m_channelOffsets.end() || m_openForWrite)
    {
        return nullptr;
    }

    std::string headerStr = readChannelHeader(inChannelName);
    json header = json::parse(headerStr);
    isize_t numPkts = header.at("Number of Packets");

    if (m_analog && numPkts == 0)
    {
        // Calculate number of packets
        std::ios::pos_type current = m_file.tellg();
        isize_t dataBytes = isize_t(m_headerOffset - current);
        numPkts = dataBytes / sizeof(DataPkt);
        ISX_ASSERT(dataBytes == numPkts * sizeof(DataPkt));
    }

    std::unique_ptr<DataPkt[]> data(new DataPkt[numPkts]);
    m_file.read(reinterpret_cast<char*>(data.get()), numPkts*sizeof(DataPkt));

    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error reading file.");
    }

    SpLogicalTrace_t logicalTrace = std::make_shared<LogicalTrace>(m_timingInfo, inChannelName);
    for(isize_t i(0); i < numPkts; ++i)
    {
        DataPkt & pkt = data[i];
        float val = pkt.getValue();
        if (m_analog)
        {
            logicalTrace->addValue(pkt.getTime(), val);
        }
        else
        {
            if (val == 0.0f)
            {
                logicalTrace->addValue(pkt.getTime(), pkt.getState());
            }
            else
            {
                logicalTrace->addValue(pkt.getTime(), val * pkt.getState());
            }
        }
        
    }

    return logicalTrace;
}

const isx::TimingInfo 
EventBasedFileV1::getTimingInfo() const
{
    return m_timingInfo;
}

void
EventBasedFileV1::readFileFooter()
{
    if (!m_openForWrite)
    {
        json j = readJsonHeaderAtEnd(m_file, m_headerOffset);
        try
        {
            DataSet::Type type = DataSet::Type(size_t(j["type"]));
            if ((type != DataSet::Type::GPIO) && (type != DataSet::Type::EVENTS))
            {
                ISX_THROW(isx::ExceptionDataIO,
                    "Expected type to be GPIO or EVENTS. Instead got ", size_t(type), ".");
            }
            m_timingInfo = convertJsonToTimingInfo(j["timing info"]);
            m_channelOffsets = j["channel offsets"].get<std::map<std::string, int>>();
            m_dataType = (StoredData)(j["dataType"].get<int>());

            if (j["fileVersion"].get<int>() == 0)
            {
                auto search = m_channelOffsets.find("GPIO4_AI");
                if (search != m_channelOffsets.end())
                {
                    m_analog = true;
                }
            }
            else
            {
                m_analog = j["analog"].get<bool>();
            }

        }
        catch (const std::exception & error)
        {
            ISX_THROW(isx::ExceptionDataIO, "Error parsing header: ", error.what());
        }
        catch (...)
        {
            ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing header.");
        }
    }
}

std::string
EventBasedFileV1::readChannelHeader(const std::string & inChannelName)
{
    if (!m_openForWrite)
    {
        m_file.seekg(m_channelOffsets.at(inChannelName), std::ios_base::beg);

        json j = readJson(m_file);

        ISX_ASSERT(inChannelName == j.at("channel"));

        return j.dump();
    }

    return std::string();
}


void
EventBasedFileV1::setTimingInfo(const isx::TimingInfo & inTimingInfo)
{
    m_timingInfo = inTimingInfo;
}

void
EventBasedFileV1::writeChannelHeader(
    const std::string & inChannel,
    const std::string & inMode,
    const std::string & inTriggerFollow,
    const isx::isize_t inNumPackets)
{
    json j;
    j["channel"] = inChannel;
    j["mode"] = inMode;
    j["GPIO Trigger/Follow"] = inTriggerFollow;
    j["Number of Packets"] = inNumPackets;

    int offset = (int)m_file.tellp();

    writeJson(j, m_file);

    m_channelOffsets[inChannel] = offset;
}

void
EventBasedFileV1::writeDataPkt(const DataPkt & inData)
{
    m_file.write((char *)&inData, sizeof(inData));

    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Error writing output data file: ", m_fileName);
    }

    m_file.flush();
}

void
EventBasedFileV1::closeFileForWriting()
{
    if (m_openForWrite && !m_closedForWriting)
    {
        writeFileFooter();
        m_closedForWriting = true;
    }
}

void
EventBasedFileV1::writeFileFooter()
{
    json j;
    try
    {
        j["type"] = (m_dataType == StoredData::GPIO) ? size_t(DataSet::Type::GPIO) : size_t(DataSet::Type::EVENTS);
        j["channel offsets"] = m_channelOffsets;
        j["timing info"] = convertTimingInfoToJson(m_timingInfo);
        j["producer"] = getProducerAsJson();
        j["fileVersion"] = s_fileVersion;
        j["dataType"] = (int)m_dataType;
        j["analog"] = m_analog;
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO,
            "Unknown error while generating file header.");
    }

    writeJsonHeaderAtEnd(j, m_file);
    m_file.flush();
    m_file.close();
}

std::string
EventBasedFileV1::getExtraProperties() const
{
    return "null";
}

void
EventBasedFileV1::setExtraProperties(const std::string & inProperties)
{
    ISX_ASSERT("Cannot set extra properties of V1 events file.");
}

EventBasedFileV1::StoredData
EventBasedFileV1::getDataType() const
{
    return m_dataType;
}

} // namespace isx
