#include "isxCoreFwd.h"
#include "isxDataSet.h"
#include "isxException.h"
#include "isxJsonUtils.h"
#include "isxMovieFactory.h"
#include "isxCellSetFactory.h"

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
    , m_parent(nullptr)
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
    , m_parent(nullptr)
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

const DataSet::Properties &
DataSet::getProperties() const
{
    return m_properties;
}

const HistoricalDetails & 
DataSet::getHistory() const
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

void
DataSet::insertDerivedDataSet(std::shared_ptr<DataSet> & inDataSet)
{
    validateItemToBeInserted(inDataSet.get());

    const std::string name = inDataSet->getName();
    if (isChild(name))
    {
        ISX_THROW(ExceptionDataIO, "There is already a derived data set with the name: ", name);
    }

    // If the child still has a parent, this should remove it
    ProjectItem * parent = inDataSet->getParent();
    if (parent != nullptr && parent->isChild(name))
    {
        parent->removeChild(name);
    }

    inDataSet->setParent(this);
    m_derived.push_back(inDataSet);
    m_modified = true;
}

std::vector<DataSet *>
DataSet::getDerivedDataSets() const
{
    std::vector<DataSet *> outDataSets;
    for (const auto & dataSet : m_derived)
    {
        outDataSets.push_back(dataSet.get());
    }
    return outDataSets;
}

std::shared_ptr<DataSet>
DataSet::removeDerivedDataSet(const std::string & inName)
{
    for (auto it = m_derived.begin(); it != m_derived.end(); ++it)
    {
        if ((*it)->getName() == inName)
        {
            std::shared_ptr<DataSet> dataSet = *it;
            dataSet->setParent(nullptr);
            m_derived.erase(it);
            m_modified = true;
            return dataSet;
        }
    }
    ISX_THROW(ExceptionDataIO, "Could not find derived data set with name: ", inName);
}

void 
DataSet::setPrevious(const std::shared_ptr<DataSet> & inDataSet)
{
    if (inDataSet)
    {
        inDataSet->setParent(nullptr);
        inDataSet->setName("previous");
    }
    m_previous = inDataSet;
}

DataSet * 
DataSet::getPrevious()
{
    return m_previous.get();
}

DataSet::Type
readDataSetType(const std::string & inFileName)
{
    std::ifstream file(inFileName, std::ios::binary);
    json j = readJsonHeader(file);

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
    return jv.dump();
}

void
DataSet::fromJsonString(
    const std::string & inDataSetJson,
    std::string & outPath,
    std::vector<DataSet> & outOriginals,
    std::vector<DataSet> & outDeriveds)
{
    json j = json::parse(inDataSetJson);
    const isize_t numDataSets = isize_t(j["dataSets"].size());

    for (isize_t i = 0; i < numDataSets; ++i)
    {
        outPath = j["path"];
        const std::string jsonString = (j["dataSets"][i]["original"]).dump();
        std::shared_ptr<DataSet> dataSet = DataSet::fromJsonString(jsonString);
        outOriginals.push_back(DataSet(
                    dataSet->getName(),
                    dataSet->getType(),
                    dataSet->getFileName(),
                    dataSet->getHistory(),
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
                    derived->getHistory(),
                    derived->getProperties()
            ));
        }
    }
}

ProjectItem::Type
DataSet::getItemType() const
{
    return ProjectItem::Type::DATASET;
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

ProjectItem *
DataSet::getParent() const
{
    return m_parent;
}

void
DataSet::setParent(ProjectItem * inParent)
{
    m_parent = inParent;
    m_modified = true;
}

std::vector<ProjectItem *>
DataSet::getChildren() const
{
    std::vector<ProjectItem *> outChildren;
    for (const auto & derivedDataSet : m_derived)
    {
        outChildren.push_back(derivedDataSet.get());
    }
    return outChildren;
}

size_t
DataSet::getNumChildren() const
{
    return m_derived.size();
}

void
DataSet::insertChild(std::shared_ptr<ProjectItem> inItem, const int inIndex)
{
    if (inItem->getItemType() != isx::ProjectItem::Type::DATASET)
    {
        ISX_THROW(ExceptionDataIO, "Only data sets can be inserted into data sets.");
    }

    auto dataSet = std::static_pointer_cast<DataSet>(inItem);
    return insertDerivedDataSet(dataSet);
}

std::shared_ptr<ProjectItem>
DataSet::removeChild(const std::string & inName)
{
    return removeDerivedDataSet(inName);
}

bool
DataSet::isModified() const
{
    return m_modified || areChildrenModified();
}

void
DataSet::setUnmodified()
{
    m_modified = false;
    setChildrenUnmodified();
}

std::string
DataSet::toJsonString(const bool inPretty) const
{
    json outJson;
    outJson["itemType"] = size_t(ProjectItem::Type::DATASET);
    outJson["name"] = m_name;
    outJson["dataSetType"] = isize_t(m_type);
    // TODO sweet : when a dataset in a project is serialized, we should
    // store the relative path to the project file name, so that if that
    // sub-tree gets moved, then the paths are still accurate.
    // For any file names that are above the project file, we should store
    // absolute paths.
    outJson["fileName"] = m_fileName;
    outJson["history"] = convertHistoryToJson(m_history);
    outJson["properties"] = convertPropertiesToJson(m_properties);
    outJson["derived"] = json::array();    
    for (const auto & derived : m_derived)
    {
        outJson["derived"].push_back(json::parse(derived->toJsonString()));
    }

    outJson["previous"] = json::object();
    if(m_previous)
    {
        outJson["previous"] = json::parse(m_previous->toJsonString());
    }

    if (inPretty)
    {
        return outJson.dump(4);
    }
    return outJson.dump();
}

std::shared_ptr<DataSet>
DataSet::fromJsonString(const std::string & inString)
{
    if (inString == json::object().dump())
    {
        return std::shared_ptr<DataSet>();
    }

    const json jsonObj = json::parse(inString);    
    const ProjectItem::Type itemType = ProjectItem::Type(size_t(jsonObj["itemType"]));
    ISX_ASSERT(itemType == ProjectItem::Type::DATASET);
    const std::string name = jsonObj["name"];
    const DataSet::Type dataSetType = DataSet::Type(size_t(jsonObj["dataSetType"]));
    const std::string fileName = jsonObj["fileName"];
    const HistoricalDetails hd = convertJsonToHistory(jsonObj["history"]);
    const DataSet::Properties properties = convertJsonToProperties(jsonObj["properties"]);

    auto outDataSet = std::make_shared<DataSet>(name, dataSetType, fileName, hd, properties);
    for (auto derivedJson : jsonObj["derived"])
    {
        auto derived = fromJsonString(derivedJson.dump());
        outDataSet->insertDerivedDataSet(derived);
    }
    outDataSet->setPrevious(DataSet::fromJsonString(jsonObj["previous"].dump()));
    return outDataSet;
}

bool
DataSet::operator ==(const ProjectItem & other) const
{
    if (other.getItemType() != ProjectItem::Type::DATASET)
    {
        return false;
    }
    auto otherDataSet = static_cast<const DataSet *>(&other);

    bool equal = (m_name == otherDataSet->m_name)
        && (m_type == otherDataSet->m_type)
        && (m_fileName == otherDataSet->m_fileName)
        && (m_properties == otherDataSet->m_properties)
        && (m_derived.size() == otherDataSet->m_derived.size())
        && (m_history == otherDataSet->m_history);
    for (size_t i = 0; equal && i < m_derived.size(); ++i)
    {
        equal &= *m_derived.at(i) == *otherDataSet->m_derived.at(i);
    }
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
    }
    else if (m_type == Type::CELLSET)
    {
        const SpCellSet_t cellSet = readCellSet(m_fileName);
        m_timingInfo = cellSet->getTimingInfo();
        m_spacingInfo = cellSet->getSpacingInfo();
        m_dataType = isx::DataType::F32;
        m_hasMetaData = true;
    }
}

} // namespace isx
