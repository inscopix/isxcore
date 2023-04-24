#include "isxCoreFwd.h"
#include "isxDataSet.h"
#include "isxException.h"
#include "isxJsonUtils.h"
#include "isxMovieFactory.h"
#include "isxCellSetFactory.h"
#include "isxVesselSetFactory.h"
#include "isxPathUtils.h"
#include "isxNVistaHdf5Movie.h"
#include "isxReportUtils.h"
#include "isxGpio.h"
#include "isxEvents.h"
#include "isxMetadata.h"
#include "isxMosaicMovie.h"

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
        const std::string & inExtraPropsStr,
        const isx::DataSet::Type inType)
{
    const std::string acqInfoStr = (inType == isx::DataSet::Type::NVISION_MOVIE) ?
        isx::getNVisionAcquisitionInfoFromExtraProps(inExtraPropsStr) : isx::getAcquisitionInfoFromExtraProps(inExtraPropsStr);
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

        // If exposure time exists, insert after sample rate if already in metadata
        if (it.key() == "Exposure Time (ms)")
        {
            auto sampleRateIt = std::find_if(inMetadata.begin(), inMetadata.end(),
                                             [](const std::pair<std::string, std::string> &element)
                                             {
                                                return element.first == "Sample Rate (Hz)";
                                             });

            // If sample rate in metadata
            if (sampleRateIt != inMetadata.end())
            {
                inMetadata.insert(++sampleRateIt, std::pair<std::string, std::string>(it.key(), value));
                continue;
            }
        }
        inMetadata.emplace_back(it.key(), value);
    }
}

void
addSampleRateToStream(
        isx::DurationInSeconds inStep,
        std::stringstream & inStream)
{
    // TODO : This is a temporary workaround for handling files that have 0 step time,
    // but should be handled more generally when this meta-data display is cleaned up.
    if (inStep.getNum() == 0)
    {
        inStream << "Inf";
    }
    else
    {
        inStream << inStep.getInverse().toDouble();
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
    else if (isNVisionMovieFileExtension(inFileName))
    {
        return DataSet::Type::NVISION_MOVIE;
    }
    else if (isBehavioralMovieFileExtension(inFileName))
    {
        return DataSet::Type::BEHAVIOR;
    }
    else if (extension == "imu")
    {
        return DataSet::Type::IMU;
    }
    else if (isGpioFileExtension(inFileName))
    {
        return DataSet::Type::GPIO;
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

        // If IMU with magnetometer, show both
        if (m_type == Type::IMU && m_secondaryTimingInfo.isValid())
        {
            addSampleRateToStream(m_timingInfo.getStep(), ss);
            metadata.push_back(std::pair<std::string, std::string>("Accelerometer Sample Rate (Hz)", ss.str()));
            ss.str("");

            addSampleRateToStream(m_secondaryTimingInfo.getStep(), ss);
            metadata.push_back(std::pair<std::string, std::string>("Magnetometer Sample Rate (Hz)", ss.str()));
            ss.str("");
        }
        else
        {
            addSampleRateToStream(m_timingInfo.getStep(), ss);
            metadata.push_back(std::pair<std::string, std::string>("Sample Rate (Hz)", ss.str()));
            ss.str("");
        }

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

        if (m_type != Type::NVISION_MOVIE) // isxb movies currently cannot have cropped or blank samples
        {
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

            ss << m_timingInfo.getBlankCount();
            metadata.push_back(std::pair<std::string, std::string>("Number of Blank Samples", ss.str()));
            ss.str("");

            if (m_timingInfo.getBlankCount() > 0)
            {
                const std::vector<isize_t> blankFrames = m_timingInfo.getBlankFrames();
                for ( auto & df : blankFrames)
                {
                    ss << df << " ";
                }
                metadata.push_back(std::pair<std::string, std::string>("Blank Samples", ss.str()));
                ss.str("");
            }
        }
    }

    // Spacing Info
    if (m_type == DataSet::Type::MOVIE || m_type == DataSet::Type::BEHAVIOR || m_type == DataSet::Type::NVISION_MOVIE
            || m_type == DataSet::Type::IMAGE || m_type == DataSet::Type::CELLSET || m_type == DataSet::Type::VESSELSET)
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
        ss.str("");
    }

    // Extra properties (from nVista 3).
    if (!m_extraProps.empty())
    {
        try
        {
            // Use metadata util function to check if dataset has metadata 
            // about pixel scaling and calculate the conversion ratio
            // Note: cannot do this in addMetadataFromExtraProps since it's a static function
            // and the metadata utils require passing in an object with a getExtraProperties function
            auto dataSet = this;
            if (isx::hasPixelScaling(dataSet))
            {
                ss << isx::getMicronsPerPixel(dataSet);
                metadata.push_back(std::pair<std::string, std::string>("Microns per Pixel", ss.str()));
                ss.str("");
            }

            addMetadataFromExtraProps(metadata, ss, m_extraProps, m_type);
        }
        catch (const std::exception & inError)
        {
            ISX_LOG_ERROR("Failed to parse metadata from extra properties with error: ", inError.what());
        }
    }

    return metadata;
}

std::string
DataSet::getExtraProperties()
{
    readMetaData();
    return m_extraProps;
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
    case Type::VESSELSET:
        return std::string("Vessel Set");
    case Type::BEHAVIOR:
        return std::string("Behavioral Movie");
    case Type::NVISION_MOVIE:
        return std::string("nVision Movie");
    case Type::GPIO:
        return std::string("GPIO");
    case Type::EVENTS:
        return std::string("Event Set");
    case Type::IMAGE:
        return std::string("Image");
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
    else if (m_type == Type::VESSELSET)
    {
        const SpVesselSet_t vesselSet = readVesselSet(m_fileName);
        m_timingInfo = vesselSet->getTimingInfo();
        m_spacingInfo = vesselSet->getSpacingInfo();
        m_dataType = isx::DataType::F32;
        m_extraProps = vesselSet->getExtraProperties();
        m_hasMetaData = true;
    }
    else if (m_type == Type::BEHAVIOR)
    {
        Variant v;
        bool hasAllMetaData =
            getPropertyValue(PROP_MOVIE_START_TIME, v)
        && getPropertyValue(PROP_BEHAV_GOP_SIZE, v)
        && getPropertyValue(PROP_BEHAV_NUM_FRAMES, v);

        const std::string ext = getExtension(m_fileName);

        if (hasAllMetaData)
        {
            SpMovie_t movie = readBehavioralMovie(m_fileName, getProperties());
            m_timingInfo = movie->getTimingInfo();
            m_spacingInfo = movie->getSpacingInfo();
            m_dataType = movie->getDataType();
            m_extraProps = movie->getExtraProperties();
            m_hasMetaData = true;
        }
    }
    else if (m_type == Type::NVISION_MOVIE)
    {
        SpMovie_t movie = readNVisionMovie(m_fileName);
        m_timingInfo = movie->getTimingInfo();
        m_spacingInfo = movie->getSpacingInfo();
        m_dataType = movie->getDataType();
        m_extraProps = movie->getExtraProperties();
        m_hasMetaData = true;
    }
    else if (m_type == Type::GPIO || m_type == Type::IMU)
    {
        const SpGpio_t gpio = readGpio(m_fileName);
        m_timingInfo = gpio->getTimingInfo();
        m_extraProps = gpio->getExtraProperties();

        // Store magnetometer timing info for metadata
        if (m_type == Type::IMU)
        {
            std::vector<std::string> channels = gpio->getChannelList();
            auto channelIt = std::find(channels.begin(), channels.end(), "Mag x");
            if (channelIt != channels.end())
            {
                isx::TimingInfo magTimingInfo = gpio->getTimingInfo("Mag x");

                if (magTimingInfo.getStep() > 0)
                {
                    m_secondaryTimingInfo = magTimingInfo;
                }
            }
        }

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

void
DataSet::setExtraProperties(const std::string & inProperties)
{
    if (!fileExists())
    {
        ISX_LOG_ERROR("Tried to read metadata from dataset with missing file: ", m_fileName);
        return;
    }

    readMetaData();

    if (m_type == Type::MOVIE || m_type == Type::IMAGE)
    {
        SpWritableMovie_t movie = std::make_shared<isx::MosaicMovie>(
                m_fileName, true);

        movie->setExtraProperties(inProperties);
        movie->closeForWriting();
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
            acqInfo["Animal Description"] = animal->at("description");
        }

        const auto microscope = extraProps.find("microscope");
        const bool isMulticolor = hasMulticolorMetadata(inExtraPropsStr);
        if (microscope != extraProps.end())
        {
            acqInfo["Microscope Focus"] = microscope->at("focus");
            acqInfo["Microscope Gain"] = microscope->at("gain");

            const auto microscopeLed = microscope->find("led");
            const bool microscopeLedExists = microscopeLed != microscope->end();
            ISX_ASSERT(microscopeLedExists, "No led section within microscope metadata");
            if (microscopeLedExists)
            {
                json powerLed1 = microscopeLed->at("exPower");
                json powerLed2 = microscopeLed->at("ogPower");
                if (isMulticolor)
                {
                    const auto dualColor = microscope->find("dualColor");
                    const bool dualEnable = dualColor->at("enabled");

                    if (dualEnable)
                    {
                        powerLed2 = microscopeLed->at("exPower2");
                    }
                }

                const std::string led1Name = isMulticolor ? "EX LED 1" : "EX LED";
                const std::string led2Name = isMulticolor ? "EX LED 2" : "OG LED";
                acqInfo["Microscope " + led1Name + " Power (mw/mm^2)"] = powerLed1;
                acqInfo["Microscope " + led2Name + " Power (mw/mm^2)"] = powerLed2;
            }

            acqInfo["Microscope Serial Number"] = microscope->at("serial");
            acqInfo["Microscope Type"] = microscope->at("type");

            acqInfo["Exposure Time (ms)"] = microscope->at("exp");

            const auto customMetadata = microscope->find("customMetadata");
            if (customMetadata != microscope->end())
            {
                for (auto it = customMetadata->begin(); it != customMetadata->end(); it++)
                {
                    acqInfo[it.key()] = *it;
                }
            }
        }

        const auto name = extraProps.find("name");
        if (name != extraProps.end())
        {
            acqInfo["Session Name"] = extraProps.at("name");
        }

        const auto personnel = extraProps.find("personnel");
        if (personnel != extraProps.end())
        {
            acqInfo["Experimenter Name"] = personnel->at("name");
        }

        // Note: Probe properties are not displayed in the metadata view because we are displaying 
        // the integrated baseplate type at the bottom of the view, and since that 
        // is a changeable property, seeing these values could confuse users (solves task IDPS-498).
        // Metadata view checks for these properties by checking that the key name begins with "Probe"
        const auto probe = extraProps.find("probe");
        if (probe != extraProps.end())
        {
            acqInfo["Probe Diameter (mm)"] = probe->at("diameter");
            acqInfo["Probe Flip"] = probe->at("flip");
            acqInfo["Probe Length (mm)"] = probe->at("length");
            acqInfo["Probe Pitch"] = probe->at("pitch");
            acqInfo["Probe Rotation (degrees)"] = probe->at("rotation");
            acqInfo["Probe Type"] = probe->at("type");
        }

        const auto producer = extraProps.find("producer");
        if (producer != extraProps.end())
        {
            const std::vector<std::string> versionTokens = splitString(producer->at("versionBE"), '-');
            acqInfo["Acquisition SW Version"] = versionTokens.at(0);
        }

        const auto idps = extraProps.find("idps");
        if (idps != extraProps.end())
        {
            const auto efocus = idps->find("efocus");
            if (efocus != idps->end())
            {
                acqInfo["efocus"] = efocus->get<uint16_t>();
            }

            const auto channel = idps->find("channel");
            if (channel != idps->end())
            {
                acqInfo["channel"] = channel->get<std::string>();
            }

            const auto interpolatedFrames = idps->find("interpolatedFrames");
            if (interpolatedFrames != idps->end())
            {
                std::stringstream ss;
                for (auto f : interpolatedFrames->get<std::vector<size_t>>())
                {
                    ss << f << " ";
                }
                acqInfo["Interpolated Samples"] = ss.str();
            }

            const auto motionCorrection = idps->find("mc_padding");
            if (motionCorrection != idps->end())
            {
                acqInfo["Motion correction padding"] = motionCorrection->get<bool>();
            }

            // cell set metadata
            const auto cellset = idps->find("cellset");
            if (cellset != idps->end())
            {
                const auto cellIdentificationMethod = cellset->find("method");
                if (cellIdentificationMethod != cellset->end())
                {
                    acqInfo["Cell Identification Method"] = cellIdentificationMethod->get<std::string>();
                }

                const auto cellSetUnits = cellset->find("units");
                if (cellSetUnits != cellset->end())
                {
                    acqInfo["Trace Units"] = cellSetUnits->get<std::string>();
                }
            }

            // vessel set metadata
            const auto vesselset = idps->find("vesselset");
            if (vesselset != idps->end())
            {
                const auto vesselSetType = vesselset->find("type");
                if (vesselSetType != vesselset->end())
                {
                    acqInfo["Vessel Set Type"] = vesselSetType->get<std::string>();
                }

                const auto vesselSetUnits = vesselset->find("units");
                if (vesselSetUnits != vesselset->end())
                {
                    acqInfo["Trace Units"] = vesselSetUnits->get<std::string>();
                }
                
                const auto vesselSetTimeIncrement = vesselset->find("timeIncrement");
                if (vesselSetTimeIncrement != vesselset->end())
                {
                    acqInfo["Time Increment (s)"] = vesselSetTimeIncrement->get<double>();
                }

                const auto vesselSetTimeWindow = vesselset->find("timeWindow");
                if (vesselSetTimeWindow != vesselset->end())
                {
                    acqInfo["Time Window (s)"] = vesselSetTimeWindow->get<double>();
                }

                const auto vesselSetInvalidWindows = vesselset->find("invalidWindows");
                if (vesselSetInvalidWindows != vesselset->end())
                {
                    std::stringstream ss;
                    for (auto f : vesselSetInvalidWindows->get<std::vector<int>>())
                    {
                        ss << f << " ";
                    }
                    acqInfo["Windows with Invalid Frame Error"] = ss.str();
                }

                const auto vesselSetClippedVessels = vesselset->find("clippedVessels");
                acqInfo["Vessels with Clipping Error"] = (vesselSetClippedVessels != vesselset->end()) ? vesselSetClippedVessels->get<std::string>() : "{}";

                const auto vesselSetNoSignificantVessels = vesselset->find("noSignificantVessels");
                acqInfo["Vessels with No Significant Pixels Error"] = (vesselSetNoSignificantVessels != vesselset->end()) ? vesselSetNoSignificantVessels->get<std::string>() : "{}";

                const auto vesselSetDirectionChangedVessels = vesselset->find("directionChangedVessels");
                acqInfo["Vessels with Direction Change Error"] = (vesselSetDirectionChangedVessels != vesselset->end()) ? vesselSetDirectionChangedVessels->get<std::string>() : "{}";
            }
        }
        
        const auto processingInterface = extraProps.find("processingInterface");
        if (processingInterface != extraProps.end())
        {
            const auto & recordingUUID = processingInterface->find("recordingUUID");
            if (recordingUUID != processingInterface->end())
            {
                acqInfo["Recording UUID"] = recordingUUID->get<std::string>();
            }
        }
    }

    return acqInfo.dump();
}

std::string
getNVisionAcquisitionInfoFromExtraProps(const std::string & inExtraPropsStr)
{
    json acqInfo;
    const json extraProps = json::parse(inExtraPropsStr);

    if (extraProps != nullptr)
    {
        verifyJsonKey(extraProps, "cameraName");
        const auto cameraName = extraProps.at("cameraName").get<std::string>();

        const auto userInterface = extraProps.find("userInterface");
        if (userInterface != extraProps.end())
        {
            const auto animal = userInterface->find("animal");
            if (animal != userInterface->end() && !animal->empty())
            {
                acqInfo["Animal Sex"] = animal->at("sex");
                acqInfo["Animal Date of Birth"] = animal->at("dob");
                acqInfo["Animal ID"] = animal->at("id");
                acqInfo["Animal Species"] = animal->at("species");
                acqInfo["Animal Weight"] = animal->at("weight");
                acqInfo["Animal Description"] = animal->at("description");
            }

            const auto customMetadata = userInterface->find("customMetadata");
            if (customMetadata != userInterface->end())
            {
                for (auto it = customMetadata->begin(); it != customMetadata->end(); it++)
                {
                    acqInfo[it.key()] = it->at("value");
                }
            }
        }

        const auto processingInterface = extraProps.find("processingInterface");
        if (processingInterface != extraProps.end())
        {
            const auto & recordingUUID = processingInterface->find("recordingUUID");
            if (recordingUUID != processingInterface->end())
            {
                acqInfo["Recording UUID"] = recordingUUID->get<std::string>();
            }

            const auto system = processingInterface->find("system");
            if (system != processingInterface->end())
            {
                bool paired = false;
                const auto behavior = system->find("behavior");
                if (behavior != system->end())
                {
                    paired = behavior->at("paired").get<bool>();

                    const auto behaviorSystem = behavior->find("system");
                    if (behaviorSystem != behavior->end())
                    {
                        acqInfo["Behavior Session Name"] = behaviorSystem->at("sessionName");
                    }

                    const auto behaviorSoftware = behavior->find("software");
                    if (behaviorSoftware != behavior->end())
                    {
                        acqInfo["Behavior Acquisition SW Version"] = behaviorSoftware->at("behaviorIdas");
                    }
                }

                acqInfo["Miniscope Paired"] = paired;
                if (paired)
                {
                    acqInfo["Miniscope Synced"] = behavior->at("synced");

                    const auto miniscope = system->find("miniscope");
                    if (miniscope != system->end())
                    {
                        const auto miniscopeSystem = miniscope->find("system");
                        if (miniscopeSystem != miniscope->end())
                        {
                            acqInfo["Miniscope Session Name"] = miniscopeSystem->at("sessionName");
                            acqInfo["Miniscope Serial Number"] = miniscopeSystem->at("systemSerial");
                        }

                        const auto miniscopeSoftware = miniscope->find("software");
                        if (miniscopeSoftware != miniscope->end())
                        {
                            acqInfo["Miniscope Acquisition SW Version"] = miniscopeSoftware->at("miniscopeIdas");
                        }
                    }
                }
            }
            
            const auto camera = processingInterface->find(cameraName);
            if (camera != processingInterface->end())
            {
                acqInfo["Camera Name"] = camera->at("cameraAlias");
                acqInfo["Camera Serial Number"] = camera->at("cameraSerial");

                const auto sensorParams = camera->find("sensorParams");
                if (sensorParams != camera->end())
                {
                    acqInfo["Camera Brightness"] = sensorParams->at("brightness");
                    acqInfo["Camera Contrast"] = sensorParams->at("contrast");
                    acqInfo["Camera Saturation"] = sensorParams->at("saturation");
                    acqInfo["Camera Gain"] = sensorParams->at("gain");
                }

                const auto exposure = sensorParams->find("exposure");
                if (exposure != sensorParams->end() && exposure->at("type") == "manual")
                {
                    acqInfo["Camera Exposure Time (ms)"] = exposure->at("value");
                }
            }
        }
    }
    
    return acqInfo.dump();
}

} // namespace isx
