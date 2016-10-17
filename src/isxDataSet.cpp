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
DataSet::getPropertyValue(const std::string & inPropertyName, float & outValue) const
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
DataSet::setPropertyValue(const std::string & inPropertyName, float inValue)
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

/*static*/
std::string
DataSet::toJsonString(const DataSet * inOriginal, const DataSet * inDerived)
{
    json j;
    j["original"]["path"] = inOriginal->getPath();
    j["original"]["dataset"] = convertDataSetToJson(inOriginal);
    if(inDerived)
    {
        j["derived"]["path"] = inDerived->getPath();
        j["derived"]["dataset"] = convertDataSetToJson(inDerived);
    }
    return j.dump();
}

/*static*/
void
DataSet::fromJsonString(const std::string & inDataSetJson, std::string & outPath, DataSet & outOriginal, DataSet & outDerived)
{
    json j = json::parse(inDataSetJson);
    outPath = j["original"]["path"];
    UpGroup_t bogus(new Group{""});
    createDataSetFromJson(bogus.get(), j["original"]["dataset"]);
    outOriginal = **(bogus->getDataSets().begin());
    
    isize_t derived_present = j.count("derived");
    if(derived_present)
    {
        createDataSetFromJson(bogus.get(), j["derived"]["dataset"]);
        outDerived = *(bogus->getDataSets().back());
    }    
    
}
} // namespace isx
