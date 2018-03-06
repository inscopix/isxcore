#include "isxEventBasedFileV2.h"
#include "isxJsonUtils.h"
#include <string>
#include "isxLogicalTrace.h"

namespace isx
{


EventBasedFileV2::EventBasedFileV2()
{

}

EventBasedFileV2::EventBasedFileV2(
    const std::string & inFileName, 
    DataSet::Type inType,  
    bool inWrite) :
    m_fileName(inFileName),
    m_dataType(inType),
    m_openForWrite(inWrite)
{
    std::ios_base::openmode mode = std::ios::binary;
    if (inWrite)
    {
        mode |= std::ios_base::out;
    }
    else
    {
        mode |= std::ios_base::in;
    }

    m_file.open(m_fileName, mode);
    if (!m_file.good() || !m_file.is_open())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open file: ", m_fileName);
    }
    readFileFooter();
}

EventBasedFileV2::~EventBasedFileV2()
{
    closeFileForWriting();
    isx::closeFileStreamWithChecks(m_file, m_fileName);
}

SignalType 
EventBasedFileV2::getSignalType(const std::string & inChannelName)
{
    auto search = std::find(m_channelList.begin(), m_channelList.end(), inChannelName);
    if(search == m_channelList.end())
    {
        ISX_THROW(isx::ExceptionUserInput, "Unrecognized channel name: ", inChannelName);
    }

    size_t ind = search - m_channelList.begin();

    SignalType st = SignalType::SPARSE;
    if (m_steps[ind] != DurationInSeconds(0, 1))
    {
        st = SignalType::DENSE;
    }

    return st;
}

bool 
EventBasedFileV2::isValid() const
{
    return m_valid;
}

const std::string & 
EventBasedFileV2::getFileName() const
{
    return m_fileName;
}

const std::vector<std::string> 
EventBasedFileV2::getChannelList() const
{
    return m_channelList;
}

void 
EventBasedFileV2::readAllTraces(std::vector<SpFTrace_t> & inContinuousTraces, std::vector<SpLogicalTrace_t> & inLogicalTraces)
{
    inContinuousTraces.resize(m_channelList.size());
    inLogicalTraces.resize(m_channelList.size());
    TimingInfos_t timingInfos(m_channelList.size());

    for (size_t i(0); i < m_channelList.size(); ++i)
    {
        inContinuousTraces[i] = nullptr;
        inLogicalTraces[i] = nullptr;
        timingInfos[i] = getTimingInfo(m_channelList[i]);
        
        inLogicalTraces[i] = std::make_shared<LogicalTrace>(timingInfos[i], m_channelList[i]);
        
        if (m_steps[i] != DurationInSeconds(0, 1))
        {
            inContinuousTraces[i] = std::make_shared<FTrace_t>(timingInfos[i], m_channelList[i]);

            /// init all values to NaN
            for (isize_t k(0); k < timingInfos[i].getNumTimes(); ++k)
            {
                inContinuousTraces[i]->setValue(k, std::numeric_limits<float>::quiet_NaN());
            }
        }
    } 
    
    size_t pos = 0;
    m_file.seekg(pos, std::ios_base::beg);
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error reading cell id.");
    }

    while (pos < size_t(m_headerOffset))
    {
        DataPkt pkt;
        m_file.read((char *)&pkt, sizeof(DataPkt));

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading file.");
        }

        pos += sizeof(DataPkt);
        
        bool isValueType = (pkt.type == uint32_t(PktType::VALUE));
        
        if (isValueType)
        {
            ISX_ASSERT(pkt.signal < m_channelList.size());
            DurationInSeconds offset(isize_t(pkt.offsetMicroSecs), isize_t(1E6));
            auto ts = m_startTime + offset;

            if (inLogicalTraces[pkt.signal])
            {
                inLogicalTraces[pkt.signal]->addValue(ts, pkt.value);
            }
            
            if (inContinuousTraces[pkt.signal])
            {   
                auto i = timingInfos[pkt.signal].convertTimeToIndex(ts);
                inContinuousTraces[pkt.signal]->setValue(i, pkt.value);
            }
        }
    }

    // Update the dropped frames info for continuous signals
    for (auto & ct : inContinuousTraces)
    {        
        if (ct)
        {
            std::vector<isize_t> droppedFrames;
            auto numSamples = ct->getTimingInfo().getNumTimes();
            for (isize_t i(0); i < numSamples; ++i)
            {
                if (std::isnan(ct->getValue(i)))
                {
                    droppedFrames.push_back(i);
                }
            }
            ct->setDroppedFrames(droppedFrames);
        }
    }

    
}

SpLogicalTrace_t 
EventBasedFileV2::getLogicalData(const std::string & inChannelName)
{
    auto search = std::find(m_channelList.begin(), m_channelList.end(), inChannelName);
    if(search == m_channelList.end() || m_openForWrite)
    {
        return nullptr;
    }

    std::vector<SpFTrace_t> contTraces;
    std::vector<SpLogicalTrace_t> logiTraces;
    readAllTraces(contTraces, logiTraces);

    size_t ind = search - m_channelList.begin();
    return logiTraces[ind];    
}

SpFTrace_t 
EventBasedFileV2::getAnalogData(const std::string & inChannelName)
{
    auto search = std::find(m_channelList.begin(), m_channelList.end(), inChannelName);
    if(search == m_channelList.end() || m_openForWrite)
    {
        return nullptr;
    }

    std::vector<SpFTrace_t> contTraces;
    std::vector<SpLogicalTrace_t> logiTraces;
    readAllTraces(contTraces, logiTraces);

    size_t ind = search - m_channelList.begin();
    return contTraces[ind];
}


const TimingInfo  
EventBasedFileV2::getTimingInfo(const std::string & inChannelName) const
{
    auto search = std::find(m_channelList.begin(), m_channelList.end(), inChannelName);
    if(search == m_channelList.end())
    {
        ISX_THROW(isx::ExceptionUserInput, "File does not contain channel: ", inChannelName);
    }
    size_t i = search - m_channelList.begin();

    if (m_steps[i] == DurationInSeconds(0, 1))
    {
        DurationInSeconds step(1, 1000);
        isize_t numSamples = isize_t((m_endTime - m_startTime).toDouble() / step.toDouble());
        return TimingInfo(m_startTime, step, numSamples);
    }
    else
    {
        DurationInSeconds offset(m_startOffsets[i], 1000000);
        return TimingInfo(m_startTime + offset, m_steps[i], m_numSamples[i]);
    }
}

const isx::TimingInfo  
EventBasedFileV2::getTimingInfo() const
{
    DurationInSeconds step;
    if (!m_channelList.empty())
    {
        step = *(std::max_element(m_steps.begin(), m_steps.end()));        
    }

    isize_t numSamples;
    if (step == DurationInSeconds(0, 1))
    {
        step = DurationInSeconds(1, 100);        
    }
    
    numSamples = isize_t((m_endTime - m_startTime).toDouble() / step.toDouble());
        
    return TimingInfo(m_startTime, step, numSamples);
}

void EventBasedFileV2::writeDataPkt(const DataPkt & inData)
{   
    if (m_openForWrite && !m_closedForWriting)
    {
        m_file.write((char *)&inData, sizeof(inData));

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Error writing output data file: ", m_fileName);
        }

        m_file.flush();

        // Add channel name to channel list
        if (inData.signal >= m_channelList.size())
        {
            std::stringstream ss;
            ss << "signal" << inData.signal;
            m_channelList.push_back(ss.str());
            m_startOffsets.push_back(inData.offsetMicroSecs);
            m_numSamples.push_back(1);
            m_steps.push_back(DurationInSeconds(0, 1));
            if (hasMetrics())
            {
                 m_traceMetrics.push_back(SpTraceMetrics_t());
            }
        }
        else
        {
            ++m_numSamples[inData.signal];
        }
    }    
}

void EventBasedFileV2::closeFileForWriting()
{
    if (m_openForWrite && !m_closedForWriting)
    {       
        writeFileFooter();
        m_closedForWriting = true;
    }
}

void EventBasedFileV2::setChannelList(const std::vector<std::string> & inNewChannelNames)
{
    if (!inNewChannelNames.empty())
    {
        ISX_ASSERT(m_channelList.size() == inNewChannelNames.size());
        m_channelList = inNewChannelNames;
    }
}


void 
EventBasedFileV2::readFileFooter()
{
    if (!m_openForWrite)
    {        
        try
        {
            json j = readJsonHeaderAtEnd(m_file, m_headerOffset);
            if (j.find("fileType") == j.end())
            {
                return;
            }
            
            m_dataType = DataSet::Type(size_t(j["type"]));
            if ((m_dataType != DataSet::Type::GPIO) && (m_dataType != DataSet::Type::EVENTS))
            {
                ISX_THROW(isx::ExceptionDataIO,
                    "Expected type to be GPIO or EVENTS. Instead got ", size_t(m_dataType), ".");
            }

            for (auto & jsteps : j["signalSteps"])
            {
                m_steps.push_back(convertJsonToRatio(jsteps));
            }
            m_startTime = convertJsonToTime(j["global times"].at(0));
            m_endTime = convertJsonToTime(j["global times"].at(1));
            m_channelList = j["channel list"].get<std::vector<std::string>>();
            m_startOffsets = j["startOffsets"].get<std::vector<uint64_t>>();
            m_numSamples = j["numSamples"].get<std::vector<uint64_t>>();
            m_traceMetrics = convertJsonToEventMetrics(j["metrics"]);
            m_valid = true;
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

void 
EventBasedFileV2::writeFileFooter()
{
    json j;
    try
    {
        j["type"] = size_t(m_dataType);
        j["channel list"] = m_channelList;
        j["global times"] = {convertTimeToJson(m_startTime), convertTimeToJson(m_endTime)};
        j["producer"] = getProducerAsJson();
        j["fileVersion"] = s_fileVersion;
        j["fileType"] = int(FileType::V2);

        json jsteps;
        for (auto & s : m_steps)
        {
            jsteps.push_back(convertRatioToJson(s));
        }
        j["signalSteps"] = jsteps;
        j["startOffsets"] = m_startOffsets;
        j["numSamples"] = m_numSamples;
        j["metrics"] = convertEventMetricsToJson(m_traceMetrics);

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

void 
EventBasedFileV2::setTimingInfo(const Time & inStartTime, const Time & inEndTime, const std::vector<DurationInSeconds> & inSteps)
{
    ISX_ASSERT(m_steps.size() == inSteps.size());
    m_startTime = inStartTime;
    m_endTime = inEndTime;
    m_steps = inSteps;
}

bool 
EventBasedFileV2::hasMetrics() const
{
    return !m_traceMetrics.empty();
}

SpTraceMetrics_t 
EventBasedFileV2::getTraceMetrics(isize_t inIndex) const
{
    if (m_traceMetrics.size() > inIndex)
    {
        return m_traceMetrics.at(inIndex);
    }
    return SpTraceMetrics_t();
}

void
EventBasedFileV2::setTraceMetrics(isize_t inIndex, const SpTraceMetrics_t & inMetrics)
{
    if (m_fileClosedForWriting)
    {
        ISX_THROW(isx::ExceptionFileIO,
                  "Writing data after file was closed for writing.", m_fileName);
    }

    if (!hasMetrics())
    {
        m_traceMetrics = EventMetrics_t(m_channelList.size(), SpTraceMetrics_t());
    }
    m_traceMetrics.at(inIndex) = inMetrics;
}




}