#include "isxGroup.h"
#include "isxException.h"

namespace isx
{

Group::Group()
    : m_valid(false)
    , m_parent(SpGroup_t())
{
}

Group::Group(const std::string & inName)
    : m_valid(false)
    , m_name(inName)
    , m_parent(SpGroup_t())
{
    m_valid = true;
}

void
Group::addGroup(const SpGroup_t & inGroup)
{
    const std::string name = inGroup->getName();
    if (isNameTaken(name))
    {
        ISX_THROW(isx::ExceptionDataIO,
                "There is already an item with the name: ", name);
    }
    m_groups.push_back(inGroup);
}

std::vector<SpGroup_t>
Group::getGroups() const
{
    return m_groups;
}

SpGroup_t
Group::getGroup(const std::string & inName) const
{
    std::vector<SpGroup_t>::const_iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        if ((*it)->m_name == inName)
        {
            return *it;
        }
    }
    ISX_THROW(isx::ExceptionDataIO, "Could not find group with name: ", inName);
}

void
Group::removeGroup(const std::string & inName)
{
    std::vector<SpGroup_t>::const_iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        if ((*it)->m_name == inName)
        {
            m_groups.erase(it);
            return;
        }
    }
    ISX_THROW(isx::ExceptionDataIO, "Could not find group with name: ", inName);
}

void
Group::addDataSet(const SpDataSet_t & inDataSet)
{
    const std::string name = inDataSet->getName();
    if (isNameTaken(name))
    {
        ISX_THROW(isx::ExceptionDataIO,
                "There is already an item with the name: ", name);
    }
    m_dataSets.push_back(inDataSet);
}

std::vector<SpDataSet_t>
Group::getDataSets() const
{
    return m_dataSets;
}

std::vector<SpDataSet_t>
Group::getDataSets(DataSet::Type inType) const
{
    std::vector<SpDataSet_t> outDataSets;
    std::vector<SpDataSet_t>::const_iterator it;
    for (it = m_dataSets.begin(); it != m_dataSets.end(); ++it)
    {
        if ((*it)->getType() == inType)
        {
            outDataSets.push_back(*it);
        }
    }
    return outDataSets;
}

SpDataSet_t
Group::getDataSet(const std::string & inName) const
{
    std::vector<SpDataSet_t>::const_iterator it;
    for (it = m_dataSets.begin(); it != m_dataSets.end(); ++it)
    {
        if ((*it)->getName() == inName)
        {
            return *it;
        }
    }
    ISX_THROW(isx::ExceptionDataIO, "Could not find data set with name: ", inName);
}

void
Group::removeDataSet(const std::string & inName)
{
    std::vector<SpDataSet_t>::const_iterator it;
    for (it = m_dataSets.begin(); it != m_dataSets.end(); ++it)
    {
        if ((*it)->getName() == inName)
        {
            m_dataSets.erase(it);
        }
    }
    ISX_THROW(isx::ExceptionDataIO, "Could not find data set with name: ", inName);
}

bool
Group::isValid() const
{
    return m_valid;
}

std::string
Group::getName() const
{
    return m_name;
}

bool
Group::hasParent() const
{
    return bool(m_parent);
}

SpGroup_t
Group::getParent() const
{
    return m_parent;
}

void
Group::setParent(SpGroup_t & inParent)
{
    inParent->addGroup(shared_from_this());
    m_parent = inParent;
}

std::string
Group::getPath() const
{
    if (m_parent)
    {
        return m_parent->getPath() + "/" + m_name;
    }
    return m_name;
}

bool
Group::isNameTaken(const std::string & inName) const
{
    for (size_t i = 0; i < m_groups.size(); ++i)
    {
        if (m_groups[i]->getName() == inName)
        {
            return true;
        }
    }
    for (size_t i = 0; i < m_dataSets.size(); ++i)
    {
        if (m_dataSets[i]->getName() == inName)
        {
            return true;
        }
    }
    return false;
}

bool
Group::operator ==(const Group & inOther) const
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
    return sameParent && (m_name == inOther.m_name);
}

} // namespace isx
