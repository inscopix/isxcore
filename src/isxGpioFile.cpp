#include "isxGpioFile.h"
#include "isxJsonUtils.h"
#include <limits>

namespace isx
{

GpioFile::DataPkt::DataPkt()
{

}

GpioFile::DataPkt::DataPkt(
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
}

bool 
GpioFile::DataPkt::getState()
{
    uint32_t state = m_value & 0x80000000;
    return (state != 0);
}

float 
GpioFile::DataPkt::getValue()
{
    float * fval = (float *)&m_value;
    return std::abs(*fval);
}

Time 
GpioFile::DataPkt::getTime() const
{
    DurationInSeconds secsFromUnixEpoch(isize_t(m_timeStampUSec / 1000000), 1);
    DurationInSeconds usecs(m_timeStampUSec % 1000000, 1000000);
    secsFromUnixEpoch += usecs;
    return Time(secsFromUnixEpoch);
}


GpioFile::GpioFile()
{

}

GpioFile::GpioFile(const std::string & inFileName) : 
    m_fileName(inFileName)
{
    m_file.open(m_fileName, std::ios::binary | std::ios_base::in);
    if (!m_file.good() || !m_file.is_open())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open GPIO file for reading: ", m_fileName);
    }
    readFileFooter();
    m_valid = true;
}

GpioFile::~GpioFile()
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
GpioFile::isValid() const
{
    return m_valid;
}

bool 
GpioFile::isAnalog() const
{
    return m_analog;
}

const std::string &
GpioFile::getFileName() const
{
    return m_fileName;
}

const isize_t 
GpioFile::numberOfChannels()
{
    return m_channelOffsets.size();
}

const std::vector<std::string> 
GpioFile::getChannelList() const
{
    std::vector<std::string> keys;
    
    for (auto & pair : m_channelOffsets)
    {
        keys.push_back(pair.first);
    }

    return keys;
}

SpFTrace_t 
GpioFile::getAnalogData()
{
    if(!m_analog)
    {
        return nullptr;
    }

    readChannelHeader("GPIO4_AI");

    // Calculate number of packets
    std::ios::pos_type current = m_file.tellg();
    isize_t dataBytes = isize_t(m_headerOffset - current);
    isize_t recordedNumTimes = dataBytes / sizeof(DataPkt); 
    ISX_ASSERT(dataBytes == recordedNumTimes * sizeof(DataPkt));

    std::unique_ptr<DataPkt[]> data(new DataPkt[recordedNumTimes]); 
    m_file.read(reinterpret_cast<char*>(data.get()), dataBytes);

    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error reading GPIO file.");
    }

    SpFTrace_t trace = std::make_shared<FTrace_t>(m_timingInfo);
    std::vector<isize_t> droppedFrames;
    isize_t prevIndex = 0;

    for(isize_t i(0); i < recordedNumTimes; ++i)
    {
        DataPkt & pkt = data[i];
        Time time = pkt.getTime();
        isize_t index = m_timingInfo.convertTimeToIndex(time);
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
GpioFile::getLogicalData(const std::string & inChannelName)
{
    auto search = m_channelOffsets.find(inChannelName);
    if(search == m_channelOffsets.end())
    {
        return nullptr;
    }

    std::string headerStr = readChannelHeader(inChannelName);
    json header = json::parse(headerStr);
    isize_t numPkts = header.at("Number of Packets");

    std::unique_ptr<DataPkt[]> data(new DataPkt[numPkts]);  
    m_file.read(reinterpret_cast<char*>(data.get()), numPkts*sizeof(DataPkt));

    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error reading GPIO file.");
    }

    SpLogicalTrace_t logicalTrace = std::make_shared<LogicalTrace>(m_timingInfo, inChannelName);
    for(isize_t i(0); i < numPkts; ++i)
    {
        DataPkt & pkt = data[i];
        float val = pkt.getValue();
        if(val == 0.0f)
        {
            logicalTrace->addValue(pkt.getTime(), pkt.getState());
        }
        else
        {
            logicalTrace->addValue(pkt.getTime(), val * pkt.getState());
        }        
    }

    return logicalTrace;
}

const isx::TimingInfo & 
GpioFile::getTimingInfo() const
{
    return m_timingInfo;
}

void 
GpioFile::readFileFooter()
{
    json j = readJsonHeaderAtEnd(m_file, m_headerOffset);
    try
    {
        DataSet::Type type = DataSet::Type(size_t(j["type"]));
        if (type != DataSet::Type::GPIO)
        {
            ISX_THROW(isx::ExceptionDataIO,
                    "Expected type to be GPIO. Instead got ", size_t(type), ".");
        }
        m_timingInfo = convertJsonToTimingInfo(j["timing info"]);
        m_channelOffsets = j["channel offsets"].get<std::map<std::string, int>>();

        auto search = m_channelOffsets.find("GPIO4_AI");
        if(search != m_channelOffsets.end())
        {
            m_analog = true;
        }
    }
    catch (const std::exception & error)
    {
        ISX_THROW(isx::ExceptionDataIO, "Error parsing GPIO header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing GPIO header.");
    }
}

std::string 
GpioFile::readChannelHeader(const std::string & inChannelName)
{
    m_file.seekg(m_channelOffsets.at(inChannelName), std::ios_base::beg);

    json j = readJson(m_file);

    ISX_ASSERT(inChannelName == j.at("channel"));

    return j.dump();
}


} // namespace isx