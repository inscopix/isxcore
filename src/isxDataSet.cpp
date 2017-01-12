#include "isxCoreFwd.h"
#include "isxDataSet.h"
#include "isxException.h"
#include "isxGroup.h"
#include "isxJsonUtils.h"

#include <fstream>


namespace isx
{
    
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
        const Properties & inProperties)
    : m_valid(true)
    , m_modified(false)
    , m_name(inName)
    , m_type(inType)
    , m_fileName(inFileName)
    , m_parent(nullptr)
    , m_properties(inProperties)
{
}

bool
DataSet::isValid() const
{
    return m_valid;
}

DataSet::Type
DataSet::getType() const
{
    return m_type;
}

std::string
DataSet::getName() const
{
    return m_name;
}

std::string
DataSet::getFileName() const
{
    return m_fileName;
}

Group *
DataSet::getParent() const
{
    return m_parent;
}

void
DataSet::setParent(Group * inParent)
{
    m_parent = inParent;
}

std::string
DataSet::getPath() const
{
    if (m_parent)
    {
        if (m_parent->getName() == "/")
        {
            return m_parent->getName() + m_name;
        }
        else
        {
            return m_parent->getPath() + "/" + m_name;
        }
    }
    return m_name;
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

bool
DataSet::operator ==(const DataSet & inOther) const
{
    bool sameParent = true;
    if (m_parent)
    {
        if (!inOther.m_parent)
        {
            return false;
        }
        sameParent &= (*m_parent == *(inOther.m_parent));
    }
    return sameParent &&
        (m_name == inOther.m_name) &&
        (m_type == inOther.m_type) &&
        (m_fileName == inOther.m_fileName);
}

void
DataSet::serialize(std::ostream & strm) const
{
    strm << "DataSet(" <<
        "path = " << getPath() << ", " <<
        "type = " << int(m_type) << ", " <<
        "fileName = " << m_fileName << ")";
}

const DataSet::Properties & 
DataSet::getProperties() const
{
    return m_properties;
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
    const std::vector<DataSet *> & inDataSets,
    const std::vector<DataSet *> & inDerivedDataSets)
{
    json jv;
    ISX_ASSERT(inDataSets.size() == inDerivedDataSets.size());
    
    for (isize_t i = 0; i < inDataSets.size(); ++i)
    {
        json j;
        j["original"] = convertDataSetToJson(inDataSets[i]);
        if (inDerivedDataSets[i])
        {
            j["derived"] = convertDataSetToJson(inDerivedDataSets[i]);
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
        UpGroup_t bogus(new Group{""});
        createDataSetFromJson(bogus.get(), j["dataSets"][i]["original"]);
        ISX_ASSERT(bogus->getDataSets().size() == 1);
        outOriginals.push_back(*(bogus->getDataSets().back()));

        isize_t derived_present = j["dataSets"][i].count("derived");
        if (derived_present)
        {
            createDataSetFromJson(bogus.get(), j["dataSets"][i]["derived"]);
            ISX_ASSERT(bogus->getDataSets().size() == 2);
            outDeriveds.push_back(*(bogus->getDataSets().back()));
        }
    }
}
    
} // namespace isx
