#include "isxCoreFwd.h"
#include "isxDataSet.h"
#include "isxException.h"
#include "isxJsonUtils.h"
#include "isxMovieFactory.h"
#include "isxCellSetFactory.h"
#include "isxPathUtils.h"
#include "isxNVistaHdf5Movie.h"
#include "isxReportUtils.h"
#include "isxGpio.h"
#include "isxEvents.h"

#include "json.hpp"

#include <fstream>
#include <ctype.h>
#include <algorithm>
#include <iomanip>

using json = nlohmann::json;

namespace
{

void
addMetadataFromExtraProps(
        isx::DataSet::Metadata & inMetadata,
        std::stringstream & inStream,
        const std::string & inExtraPropsStr)
{
    const std::string acqInfoStr = isx::getAcquisitionInfoFromExtraProps(inExtraPropsStr);
    const json acqInfo = json::parse(acqInfoStr);

    for (json::const_iterator it = acqInfo.cbegin(); it != acqInfo.cend(); ++it)
    {
        std::string value;
        const json::value_t type = it->type();
        if (type == json::value_t::string)
        {
            value = it->get<std::string>();
        }
        else
        {
            if (type == json::value_t::number_float)
            {
                inStream << it->get<double>();
            }
            else
            {
                inStream << *it;
            }
            value = inStream.str();
            inStream.str("");
        }
        inMetadata.push_back(std::pair<std::string, std::string>(it.key(), value));
    }
}

} // namespace

namespace isx
{

const std::string DataSet::PROP_DATA_MIN = "dmin";
const std::string DataSet::PROP_DATA_MAX = "dmax";
const std::string DataSet::PROP_MOVIE_START_TIME    = "movieStartTime";
const std::string DataSet::PROP_BEHAV_NUM_FRAMES    = "numFrames";
const std::string DataSet::PROP_BEHAV_GOP_SIZE      = "gopSize";
const std::string DataSet::PROP_MOVIE_FRAME_RATE    = "movieFrameRate";

DataSet::DataSet()
    : m_valid(false)
{
}

DataSet::DataSet(
        const std::string & inName,
        Type inType,
        const std::string & inFileName,
        const HistoricalDetails & inHistory,
        const Properties & inProperties,
        const bool inImported,
        ModifiedCB_t inCallback)
    : m_valid(true)
    , m_name(inName)
    , m_type(inType)
    , m_fileName(inFileName)
    , m_history(inHistory)
    , m_properties(inProperties)
    , m_imported(inImported)
    , m_modifiedCB(inCallback)
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
    setModified();
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
        setModified();
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
        m_properties[inPropertyName] = inValue;
        setModified();

        if (inPropertyName == PROP_MOVIE_START_TIME ||
            inPropertyName == PROP_MOVIE_FRAME_RATE)
        {
            // Force update of timing info next time the metadata is requested
            m_hasMetaData = false;
        }
    }
}

DataSet::Type
readDataSetType(const std::string & inFileName, const DataSet::Properties & inProperties)
{
    std::string extension = isx::getExtension(inFileName);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    if (isNVistaImagingFileExtension(inFileName))
    {
        const isx::SpMovie_t movie = readMovie(inFileName, inProperties);
        if (movie->getTimingInfo().getNumTimes() > 1)
        {
            return DataSet::Type::MOVIE;
        }
        else
        {
            return DataSet::Type::IMAGE;
        }
    }
    else if (extension == "isxd")
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
            ISX_THROW(ExceptionDataIO, "Error parsing data file header: ", error.what());
        }
        catch (...)
        {
            ISX_THROW(ExceptionDataIO, "Unknown error while parsing data file header.");
        }
    }
    else if (isBehavioralMovieFileExtension(inFileName))
    {
        return DataSet::Type::BEHAVIOR;
    }
    else
    {
        ISX_THROW(ExceptionFileIO, "File extension not supported: ", extension);
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
    setModified();
}

void 
DataSet::setModified()
{
    if (m_modifiedCB)
    {
        m_modifiedCB();
    }
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
    ss << std::fixed << std::setprecision(3);

    // File Name
    metadata.push_back(std::pair<std::string, std::string>("File Name", m_fileName));

    // For images, we only want to display the start time, as decided in MOS-1549.
    // For TIFF images, that start time is bogus and does not matter for playback
    // so we just skip it.
    const bool isTiff = isTiffFileExtension(m_fileName);
    const bool isImage = m_type == Type::IMAGE;

    // Timing Info
    if (!(isImage && isTiff))
    {
        ss << m_timingInfo.getStart();
        metadata.push_back(std::pair<std::string, std::string>("Start Time", ss.str()));
        ss.str("");
    }

    if (!isImage)
    {
        ss << m_timingInfo.getEnd();
        metadata.push_back(std::pair<std::string, std::string>("End Time", ss.str()));
        ss.str("");

        ss << m_timingInfo.getDuration().toDouble();
        metadata.push_back(std::pair<std::string, std::string>("Duration (s)", ss.str()));
        ss.str("");

        // TODO : This is a temporary workaround for handling files that have 0 step time,
        // but should be handled more generally when this meta-data display is cleaned up.
        const isx::DurationInSeconds step = m_timingInfo.getStep();
        if (step.getNum() == 0)
        {
            ss << "Inf";
        }
        else
        {
            ss << m_timingInfo.getStep().getInverse().toDouble();
        }
        metadata.push_back(std::pair<std::string, std::string>("Sample Rate (Hz)", ss.str()));
        ss.str("");

        ss << m_timingInfo.getNumTimes();
        metadata.push_back(std::pair<std::string, std::string>("Number of Time Samples", ss.str()));
        ss.str("");

        ss << m_timingInfo.getDroppedCount();
        metadata.push_back(std::pair<std::string, std::string>("Number of Dropped Samples", ss.str()));
        ss.str("");

        if (m_timingInfo.getDroppedCount() > 0)
        {
            const std::vector<isize_t> droppedFrames = m_timingInfo.getDroppedFrames();
            for ( auto & df : droppedFrames)
            {
                ss << df << " ";
            }
            metadata.push_back(std::pair<std::string, std::string>("Dropped Samples", ss.str()));
            ss.str("");
        }

        ss << m_timingInfo.getCroppedCount();
        metadata.push_back(std::pair<std::string, std::string>("Number of Cropped Samples", ss.str()));
        ss.str("");

        if (m_timingInfo.getCroppedCount() > 0)
        {
            const IndexRanges_t croppedFrames = m_timingInfo.getCropped();
            for (size_t c = 0; c < croppedFrames.size(); ++c)
            {
                ss << croppedFrames[c];
                if (c < (croppedFrames.size() - 1))
                {
                    ss << ", ";
                }
            }
            metadata.push_back(std::pair<std::string, std::string>("Cropped Samples", ss.str()));
            ss.str("");
        }
    }

    // Spacing Info
    if (m_type == DataSet::Type::MOVIE || m_type == DataSet::Type::BEHAVIOR
            || m_type == DataSet::Type::IMAGE || m_type == DataSet::Type::CELLSET)
    {
        ss << m_spacingInfo.getNumPixels();
        metadata.push_back(std::pair<std::string, std::string>("Number of Pixels", ss.str()));
        ss.str("");
    }

    // Additional properties
    for (auto & p : m_readOnlyProperties)
    {
        Variant::MetaType type = p.second.getType();
        ISX_ASSERT(type == Variant::MetaType::STRING);
        metadata.push_back(std::pair<std::string, std::string>(p.first, p.second.value<std::string>()));
    }

    // Extra properties (from nVista 3).
    if (!m_extraProps.empty())
    {
        try
        {
            addMetadataFromExtraProps(metadata, ss, m_extraProps);
        }
        catch (const std::exception & inError)
        {
            ISX_LOG_ERROR("Failed to parse metadata from extra properties with error: ", inError.what());
        }
    }

    return metadata;
}

bool
DataSet::isImported() const
{
    return m_imported;
}

void
DataSet::deleteFile() const
{
    if (!isImported())
    {
        reportDeleteDataFile(m_fileName);
        if (std::remove(m_fileName.c_str()) != 0)
        {
            ISX_LOG_ERROR("Failed to delete file ", m_fileName);
        }
    }
}

bool
DataSet::fileExists() const
{
    return pathExists(m_fileName);
}

bool
DataSet::locateFile(const std::string & inDirectory)
{
    bool located = false;
    const std::string fileName = isx::getFileName(m_fileName);
    const std::string newFilePath = inDirectory + "/" + fileName;
    if (pathExists(newFilePath))
    {
        try
        {
            // Properties are set here because if the file is TIF, these are needed and do not impact the result of 
            // readDataSetType (which depends on the number of frames)
            isx::DataSet::Properties props; 
            props[PROP_MOVIE_START_TIME] = isx::Variant(isx::Time());
            props[PROP_MOVIE_FRAME_RATE] = isx::Variant(20.f);
            if (m_type == readDataSetType(newFilePath, props))
            {
                m_fileName = newFilePath;
                located = true;
                setModified();
            }
        }
        catch (const std::exception & e)
        {
            ISX_LOG_WARNING("Failed to locate file with error: ", e.what());
        }
    }
    return located;
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
    outJson["imported"] = m_imported;

    if (inPretty)
    {
        return outJson.dump(4);
    }
    return outJson.dump();
}

/*static*/
std::string 
DataSet::getTypeString(Type inType)
{
    switch (inType)
    {
    case Type::MOVIE: 
        return std::string("Movie");
    case Type::CELLSET:
        return std::string("Cell Set");
    case Type::BEHAVIOR:
        return std::string("Behavioral Movie");
    case Type::GPIO:
        return std::string("GPIO");
    case Type::EVENTS:
        return std::string("Event Set");
    default: 
        return std::string("");
    }
    
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
    const bool imported = jsonObj.at("imported");

    auto outDataSet = std::make_shared<DataSet>(name, dataSetType, fileName, hd, properties, imported);
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
    if (!fileExists())
    {
        ISX_LOG_ERROR("Tried to read metadata from dataset with missing file: ", m_fileName);
        return;
    }

    if (m_type == Type::MOVIE || m_type == Type::IMAGE)
    {
        const SpMovie_t movie = readMovie(m_fileName, getProperties());
        m_timingInfo = movie->getTimingInfo();
        m_spacingInfo = movie->getSpacingInfo();
        m_dataType = movie->getDataType();
        m_extraProps = movie->getExtraProperties();
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
        m_extraProps = cellSet->getExtraProperties();
        m_hasMetaData = true;
    }
    else if (m_type == Type::BEHAVIOR)
    {
        Variant v;
        bool hasAllMetaData =
            getPropertyValue(PROP_MOVIE_START_TIME, v)
        && getPropertyValue(PROP_BEHAV_GOP_SIZE, v)
        && getPropertyValue(PROP_BEHAV_NUM_FRAMES, v);

        if (hasAllMetaData)
        {
            const SpMovie_t movie = readBehavioralMovie(m_fileName, getProperties());
            m_timingInfo = movie->getTimingInfo();
            m_spacingInfo = movie->getSpacingInfo();
            m_dataType = movie->getDataType();
            m_hasMetaData = true;
        }
    }
    else if (m_type == Type::GPIO)
    {
        const SpGpio_t gpio = readGpio(m_fileName);
        m_timingInfo = gpio->getTimingInfo();
        m_dataType = isx::DataType::F32;
        m_hasMetaData = true;
    }
    else if (m_type == Type::EVENTS)
    {
        const SpEvents_t events = readEvents(m_fileName);
        m_timingInfo = events->getTimingInfo();
        m_dataType = isx::DataType::F32;
        m_extraProps = events->getExtraProperties();
        m_hasMetaData = true;
    }
}

std::string 
DataSet::getHistory() const
{
    std::string str = "Operation Name: " + m_history.getOperation() + "\n"
        + "Input Parameters: \n" + m_history.getInputParameters();

    return str;
}

void 
DataSet::setModifiedCallback(ModifiedCB_t inCallback)
{
    m_modifiedCB = inCallback;
}

std::string
getAcquisitionInfoFromExtraProps(const std::string & inExtraPropsStr)
{
    json acqInfo;
    const json extraProps = json::parse(inExtraPropsStr);

    if (extraProps != nullptr)
    {
        const auto animal = extraProps.find("animal");
        if (animal != extraProps.end())
        {
            acqInfo["Animal Sex"] = animal->at("sex");
            acqInfo["Animal Date of Birth"] = animal->at("dob");
            acqInfo["Animal ID"] = animal->at("id");
            acqInfo["Animal Species"] = animal->at("species");
            acqInfo["Animal Weight"] = animal->at("weight");
        }

        const auto microscope = extraProps.find("microscope");
        if (microscope != extraProps.end())
        {
            acqInfo["Microscope Binning Mode"] = microscope->at("binMode");
            acqInfo["Microscope Focus"] = microscope->at("focus");
            acqInfo["Microscope Gain"] = microscope->at("gain");

            const auto microscopeLed = microscope->find("led");
            acqInfo["Microscope DI LED Power"] = microscopeLed->at("diPower");
            acqInfo["Microscope EX LED Power"] = microscopeLed->at("exPower");
            acqInfo["Microscope OG LED Power"] = microscopeLed->at("ogPower");

            acqInfo["Microscope Sensor Mode"] = microscope->at("sensorMode");
            acqInfo["Microscope Serial Number"] = microscope->at("serial");
            acqInfo["Microscope Type"] = microscope->at("type");
        }

        acqInfo["Session Name"] = extraProps.at("name");

        const auto personnel = extraProps.find("personnel");
        if (personnel != extraProps.end())
        {
            acqInfo["Experimenter Name"] = personnel->at("name");
        }

        const auto probe = extraProps.find("probe");
        if (probe != extraProps.end())
        {
            acqInfo["Probe Diameter (mm)"] = probe->at("diameter");
            acqInfo["Probe Flip"] = probe->at("flip");
            acqInfo["Probe ID"] = probe->at("id");
            acqInfo["Probe Length (mm)"] = probe->at("length");
            acqInfo["Probe Name"] = probe->at("name");
            acqInfo["Probe Pitch"] = probe->at("pitch");
            acqInfo["Probe Rotation"] = probe->at("rotation");
            acqInfo["Probe Type"] = probe->at("type");
        }

        const auto producer = extraProps.find("producer");
        if (producer != extraProps.end())
        {
            acqInfo["Acquisition SW Version (BE)"] = producer->at("versionBE");
            acqInfo["Acquisition SW Version (FE)"] = producer->at("versionFE");
        }
    }

    return acqInfo.dump();
}

} // namespace isx
