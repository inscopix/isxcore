#include "isxCoreFwd.h"
#include "isxDataSet.h"
#include "isxException.h"
#include "isxJsonUtils.h"
#include "isxMovieFactory.h"
#include "isxCellSetFactory.h"
#include "isxPathUtils.h"
#include "isxNVistaHdf5Movie.h"

#include <fstream>
#include "json.hpp"

namespace isx
{

using json = nlohmann::json;

const std::string DataSet::PROP_DATA_MIN = "dmin";
const std::string DataSet::PROP_DATA_MAX = "dmax";
const std::string DataSet::PROP_VIS_MIN  = "vmin";
const std::string DataSet::PROP_VIS_MAX  = "vmax";
const std::string DataSet::PROP_MOVIE_START_TIME = "movieStartTime";

DataSet::DataSet()
    : m_valid(false)
    , m_modified(false)
    , m_series(nullptr)
{
}

DataSet::DataSet(
        const std::string & inName,
        Type inType,
        const std::string & inFileName,
        const HistoricalDetails & inHistory,
        const Properties & inProperties)
    : m_valid(true)
    , m_modified(false)
    , m_name(inName)
    , m_type(inType)
    , m_fileName(inFileName)
    , m_series(nullptr)
    , m_history(inHistory)
    , m_properties(inProperties)
{
}

DataSet::Type
DataSet::getType() const
{
    return m_type;
}

std::string
DataSet::getFileName() const
{
    return m_fileName;
}

void 
DataSet::setFileName(const std::string & inFileName)
{
    m_fileName = inFileName;
    m_modified = true;
}

const DataSet::Properties &
DataSet::getProperties() const
{
    return m_properties;
}

const HistoricalDetails & 
DataSet::getHistoricalDetails() const
{
    return m_history;
}

void
DataSet::setProperties(const SpDataSetProperties_t & inDataSetProperties)
{
    if (inDataSetProperties)
    {
        m_properties = *inDataSetProperties;
    }
}

void
DataSet::mergeProperties(const Properties & inDataSetProperties)
{
    for (auto p: inDataSetProperties)
    {
        setPropertyValue(p.first, p.second);
    }
}

bool
DataSet::getPropertyValue(const std::string & inPropertyName, Variant & outValue) const
{
    bool found = false;
    if (m_properties.find(inPropertyName) != m_properties.end())
    {
        found = true;
        outValue = m_properties.at(inPropertyName);
    }
    return found;
}

void
DataSet::setPropertyValue(const std::string & inPropertyName, Variant inValue)
{
    if (m_properties[inPropertyName] != inValue)
    {
        m_modified = true;
        m_properties[inPropertyName] = inValue;
    }
}

DataSet::Type
readDataSetType(const std::string & inFileName)
{
    std::ifstream file(inFileName, std::ios::binary);
    std::ios::pos_type dummy;
    json j = readJsonHeaderAtEnd(file, dummy);
    try
    {
        return DataSet::Type(int(j["type"]));
    }
    catch (const std::exception & error)
    {
        ISX_THROW(isx::ExceptionDataIO, "Error parsing data file header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing data file header.");
    }
}

std::string
DataSet::toJsonString(
    const std::string & inPath,
    const std::string & inDerivedPath,
    const std::string & inTitle,
    const std::vector<const DataSet *> & inDataSets,
    const std::vector<const DataSet *> & inDerivedDataSets)
{
    json jv;
    ISX_ASSERT(inDataSets.size() == inDerivedDataSets.size());

    for (isize_t i = 0; i < inDataSets.size(); ++i)
    {
        json j;
        j["original"] = json::parse(inDataSets.at(i)->toJsonString());
        if (inDerivedDataSets[i])
        {
            j["derived"] = json::parse(inDerivedDataSets.at(i)->toJsonString());
        }
        jv["dataSets"].push_back(j);
    }
    jv["path"] = inPath;
    jv["derivedPath"] = inDerivedPath;
    jv["title"] = inTitle;
    return jv.dump();
}

void
DataSet::fromJsonString(
    const std::string & inDataSetJson,
    std::string & outPath,
    std::string & outDerivedPath,
    std::string & outTitle,
    std::vector<DataSet> & outOriginals,
    std::vector<DataSet> & outDeriveds)
{
    json j = json::parse(inDataSetJson);
    const isize_t numDataSets = isize_t(j["dataSets"].size());

    for (isize_t i = 0; i < numDataSets; ++i)
    {
        outPath = j["path"];
        outDerivedPath = j["derivedPath"];
        outTitle = j["title"];
        const std::string jsonString = (j["dataSets"][i]["original"]).dump();
        std::shared_ptr<DataSet> dataSet = DataSet::fromJsonString(jsonString);
        outOriginals.push_back(DataSet(
                    dataSet->getName(),
                    dataSet->getType(),
                    dataSet->getFileName(),
                    dataSet->getHistoricalDetails(),
                    dataSet->getProperties()
        ));

        isize_t derived_present = j["dataSets"][i].count("derived");
        if (derived_present)
        {
            const std::string jsonString = (j["dataSets"][i]["derived"]).dump();
            std::shared_ptr<DataSet> derived = DataSet::fromJsonString(jsonString);
            outDeriveds.push_back(DataSet(
                    derived->getName(),
                    derived->getType(),
                    derived->getFileName(),
                    derived->getHistoricalDetails(),
                    derived->getProperties()
            ));
        }
    }
}

bool
DataSet::isValid() const
{
    return m_valid;
}

std::string
DataSet::getName() const
{
    return m_name;
}

void
DataSet::setName(const std::string & inName)
{
    m_name = inName;
    m_modified = true;
}

bool
DataSet::isModified() const
{
    return m_modified;
}

void
DataSet::setUnmodified()
{
    m_modified = false;
}

DataSet::Metadata 
DataSet::getMetadata() 
{
    if (!m_hasMetaData)
    {
        readMetaData();
    }

    Metadata metadata; 
    std::stringstream ss;
    
    // File Name
    metadata.push_back(std::pair<std::string, std::string>("File Name", m_fileName));

    // Timing Info
    ss << m_timingInfo.getStart();
    metadata.push_back(std::pair<std::string, std::string>("Start time", ss.str()));
    ss.str("");
    
    ss << m_timingInfo.getEnd();
    metadata.push_back(std::pair<std::string, std::string>("End time", ss.str()));
    ss.str("");

    ss << m_timingInfo.getDuration().toDouble();
    metadata.push_back(std::pair<std::string, std::string>("Duration In Seconds", ss.str()));
    ss.str("");

    ss << m_timingInfo.getStep().toDouble();
    metadata.push_back(std::pair<std::string, std::string>("Step In Seconds", ss.str()));
    ss.str("");

    ss << m_timingInfo.getNumTimes();
    metadata.push_back(std::pair<std::string, std::string>("Number Of Time Samples", ss.str()));
    ss.str("");

    ss << m_timingInfo.getDroppedCount();
    metadata.push_back(std::pair<std::string, std::string>("Number Of Dropped Frames", ss.str()));
    ss.str("");

    const std::vector<isize_t> droppedFrames = m_timingInfo.getDroppedFrames();
    for ( auto & df : droppedFrames)
    {
        ss << df << " ";
    }    
    metadata.push_back(std::pair<std::string, std::string>("Dropped Frames", ss.str()));
    ss.str("");

    ss << m_timingInfo.getCroppedCount();
    metadata.push_back(std::pair<std::string, std::string>("Number Of Cropped Frames", ss.str()));
    ss.str("");

    const IndexRanges_t croppedFrames = m_timingInfo.getCropped();
    for (size_t c = 0; c < croppedFrames.size(); ++c)
    {
        ss << croppedFrames[c];
        if (c < (croppedFrames.size() - 1))
        {
            ss << ", ";
        }
    }
    metadata.push_back(std::pair<std::string, std::string>("Cropped Frames", ss.str()));
    ss.str("");

    // Spacing Info
    SizeInMicrons_t size = m_spacingInfo.getPixelSize();
    ss << "(" << size.getX().toDouble() << ", " << size.getY().toDouble() << ")";
    metadata.push_back(std::pair<std::string, std::string>("Pixel Size In Microns", ss.str()));
    ss.str("");

    ss << m_spacingInfo.getNumRows();
    metadata.push_back(std::pair<std::string, std::string>("Number Of Rows", ss.str()));
    ss.str("");

    ss << m_spacingInfo.getNumColumns();
    metadata.push_back(std::pair<std::string, std::string>("Number Of Columns", ss.str()));
    ss.str("");

    // Additional properties
    for (auto & p : m_readOnlyProperties)
    {
        Variant::MetaType type = p.second.getType();
        ISX_ASSERT(type == Variant::MetaType::STRING);
        metadata.push_back(std::pair<std::string, std::string>(p.first, p.second.value<std::string>()));
    }

    return metadata;

}

std::string
DataSet::toJsonString(const bool inPretty, const std::string & inPathToOmit) const
{
    json outJson;
    outJson["name"] = m_name;
    outJson["dataSetType"] = isize_t(m_type);

    std::string fileName = m_fileName;
    if(!inPathToOmit.empty())
    {
        std::string::size_type p = m_fileName.find(inPathToOmit, 0);
        if(p != std::string::npos)
        {
            ISX_ASSERT(p == 0);
            fileName = m_fileName.substr(p + inPathToOmit.size() + 1); 
        }
    }

    outJson["fileName"] = fileName;
    outJson["history"] = convertHistoryToJson(m_history);
    outJson["properties"] = convertPropertiesToJson(m_properties);

    if (inPretty)
    {
        return outJson.dump(4);
    }
    return outJson.dump();
}

std::shared_ptr<DataSet>
DataSet::fromJsonString(const std::string & inString, const std::string & inAbsolutePathToPrepend)
{
    if (inString == json::object().dump())
    {
        return std::shared_ptr<DataSet>();
    }

    const json jsonObj = json::parse(inString);    
    const std::string name = jsonObj.at("name");
    const DataSet::Type dataSetType = DataSet::Type(size_t(jsonObj.at("dataSetType")));
    
    std::string fileName = jsonObj.at("fileName");
    if(isRelative(fileName) && !inAbsolutePathToPrepend.empty())
    {
        fileName = inAbsolutePathToPrepend + "/" + fileName;
    }

    const HistoricalDetails hd = convertJsonToHistory(jsonObj.at("history"));
    const DataSet::Properties properties = convertJsonToProperties(jsonObj.at("properties"));

    auto outDataSet = std::make_shared<DataSet>(name, dataSetType, fileName, hd, properties);
    return outDataSet;
}

bool
DataSet::operator ==(const DataSet & other) const
{
    auto otherDataSet = static_cast<const DataSet *>(&other);

    bool equal = (m_name == otherDataSet->m_name)
        && (m_type == otherDataSet->m_type)
        && (m_fileName == otherDataSet->m_fileName)
        && (m_properties == otherDataSet->m_properties)
        && (m_history == otherDataSet->m_history);
    return equal;
}

const TimingInfo &
DataSet::getTimingInfo()
{
    if (!m_hasMetaData)
    {
        readMetaData();
    }
    return m_timingInfo;
}

const SpacingInfo &
DataSet::getSpacingInfo()
{
    if (!m_hasMetaData)
    {
        readMetaData();
    }
    return m_spacingInfo;
}

DataType
DataSet::getDataType()
{
    if (!m_hasMetaData)
    {
        readMetaData();
    }
    return m_dataType;
}

void
DataSet::readMetaData()
{
    if (m_type == Type::MOVIE)
    {
        const SpMovie_t movie = readMovie(m_fileName);
        m_timingInfo = movie->getTimingInfo();
        m_spacingInfo = movie->getSpacingInfo();
        m_dataType = movie->getDataType();
        m_hasMetaData = true;

        const auto nVistaMovie = std::dynamic_pointer_cast<isx::NVistaHdf5Movie>(movie);
        if(nVistaMovie)
        {
            m_readOnlyProperties = nVistaMovie->getAdditionalProperties();     
        }
    }
    else if (m_type == Type::CELLSET)
    {
        const SpCellSet_t cellSet = readCellSet(m_fileName);
        m_timingInfo = cellSet->getTimingInfo();
        m_spacingInfo = cellSet->getSpacingInfo();
        m_dataType = isx::DataType::F32;
        m_hasMetaData = true;
    }
    else if (m_type == Type::BEHAVIOR)
    {
        
        Variant v;
        if(getPropertyValue(PROP_MOVIE_START_TIME, v))
        {
            Time startTime = v.value<Time>();
            const SpMovie_t movie = readBehavioralMovie(m_fileName, startTime);
            m_timingInfo = movie->getTimingInfo();
            m_spacingInfo = movie->getSpacingInfo();
            m_dataType = movie->getDataType();
            m_hasMetaData = true;
        }
        
    }
}

std::string 
DataSet::getHistory() const
{
    std::string str = "Operation Name: " + m_history.getOperation() + "\n"
        + "Input Parameters: \n" + m_history.getInputParameters();

    return str;
}
} // namespace isx
