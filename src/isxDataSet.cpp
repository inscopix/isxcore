#include "isxDataSet.h"
#include "isxFileUtils.h"
#include "isxException.h"
#include "isxGroup.h"

namespace isx
{

DataSet::DataSet()
    : m_valid(false)
{
}

DataSet::DataSet(
        const std::string & inName,
        Type inType,
        const std::string & inFileName)
    : m_valid(false)
    , m_name(inName)
    , m_type(inType)
    , m_fileName(inFileName)
{
    if (doesPathExist(inFileName))
    {
        ISX_THROW(isx::ExceptionFileIO,
                "The file name already exists: " + inFileName);
    }
    m_valid = true;
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

bool
DataSet::hasParent() const
{
    return bool(m_parent);
}

SpGroup_t
DataSet::getParent() const
{
    return m_parent;
}

void
DataSet::setParent(SpGroup_t & inParent)
{
    inParent->addDataSet(shared_from_this());
    m_parent = inParent;
}

std::string
DataSet::getPath() const
{
    if (m_parent)
    {
        return m_parent->getPath() + "/" + m_name;
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

} // namespace isx
