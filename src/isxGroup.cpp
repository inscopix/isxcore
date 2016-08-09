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

SpGroup_t
Group::createGroup(const std::string & inName)
{
    SpGroup_t group = std::make_shared<Group>(inName);
    addGroup(group);
    return group;
}

void
Group::addGroup(const SpGroup_t & inGroup)
{
    const std::string name = inGroup->getName();
    if (isName(name))
    {
        ISX_THROW(isx::ExceptionDataIO,
                "There is already an item with the name: ", name);
    }
    inGroup->m_parent = shared_from_this();
    m_groups.push_back(inGroup);
}

std::vector<SpGroup_t>
Group::getGroups(bool inRecurse) const
{
    std::vector<SpGroup_t> outGroups;
    std::vector<SpGroup_t>::const_iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        outGroups.push_back(*it);
        if (inRecurse)
        {
            std::vector<SpGroup_t> subGroups = (*it)->getGroups(true);
            outGroups.insert(outGroups.end(), subGroups.begin(), subGroups.end());
        }
    }
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
    std::vector<SpGroup_t>::iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        if ((*it)->m_name == inName)
        {
            it = m_groups.erase(it);
            return;
        }
    }
    ISX_THROW(isx::ExceptionDataIO, "Could not find group with name: ", inName);
}

SpDataSet_t
Group::createDataSet(
        const std::string & inName,
        DataSet::Type inType,
        const std::string & inFileName)
{
    SpDataSet_t dataSet = std::make_shared<DataSet>(inName, inType, inFileName);
    addDataSet(dataSet);
    return dataSet;
}

void
Group::addDataSet(const SpDataSet_t & inDataSet)
{
    const std::string name = inDataSet->getName();
    if (isName(name))
    {
        ISX_THROW(isx::ExceptionDataIO,
                "There is already an item with the name: ", name);
    }
    const std::string fileName = inDataSet->getFileName();
    if (isFileName(fileName))
    {
        ISX_THROW(isx::ExceptionFileIO,
                "There is already a data set with the file name: ", fileName);
    }
    SpGroup_t parent = shared_from_this();
    inDataSet->setParent(parent);
    m_dataSets.push_back(inDataSet);
}

std::vector<SpDataSet_t>
Group::getDataSets(bool inRecurse) const
{
    std::vector<SpDataSet_t> outDataSets;
    outDataSets.insert(outDataSets.end(), m_dataSets.begin(), m_dataSets.end());

    if (inRecurse)
    {
        std::vector<SpGroup_t>::const_iterator it;
        for (it = m_groups.begin(); it != m_groups.end(); ++it)
        {
            std::vector<SpDataSet_t> dataSets = (*it)->getDataSets(true);
            outDataSets.insert(outDataSets.end(), dataSets.begin(), dataSets.end());
        }
    }

    return outDataSets;
}

std::vector<SpDataSet_t>
Group::getDataSets(
        DataSet::Type inType,
        bool inRecurse) const
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

    if (inRecurse)
    {
        std::vector<SpGroup_t>::const_iterator it;
        for (it = m_groups.begin(); it != m_groups.end(); ++it)
        {
            std::vector<SpDataSet_t> dataSets = (*it)->getDataSets(inType, true);
            outDataSets.insert(outDataSets.end(), dataSets.begin(), dataSets.end());
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
    std::vector<SpDataSet_t>::iterator it;
    for (it = m_dataSets.begin(); it != m_dataSets.end(); ++it)
    {
        if ((*it)->getName() == inName)
        {
            it = m_dataSets.erase(it);
            return;
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

SpGroup_t
Group::getParent() const
{
    return m_parent;
}

SpGroup_t
Group::getRootParent()
{
    if (m_parent)
    {
        return m_parent->getRootParent();
    }
    else
    {
        return shared_from_this();
    }
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
Group::isName(const std::string & inName) const
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
Group::isFileName(const std::string & inFileName)
{
    std::shared_ptr<const Group> root = getRootParent();
    std::vector<SpDataSet_t> dataSets = root->getDataSets(true);
    std::vector<SpDataSet_t>::const_iterator it;
    for (it = dataSets.begin(); it != dataSets.end(); ++it)
    {
        if ((*it)->getFileName() == inFileName)
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
