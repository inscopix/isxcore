#include "isxDataSet.h"
#include "isxException.h"
#include "isxGroup.h"
#include "isxJsonUtils.h"

#include <fstream>

namespace isx
{

DataSet::DataSet()
    : m_valid(false)
    , m_parent(nullptr)
{
}

DataSet::DataSet(
        const std::string & inName,
        Type inType,
        const std::string & inFileName)
    : m_valid(true)
    , m_name(inName)
    , m_type(inType)
    , m_fileName(inFileName)
    , m_parent(nullptr)
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
        (m_type == inOther.m_type);
}

void
DataSet::serialize(std::ostream & strm) const
{
    strm << "DataSet(" <<
        "path = " << getPath() << ", " <<
        "type = " << int(m_type) << ", " <<
        "fileName = " << m_fileName << ")";
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

} // namespace isx
