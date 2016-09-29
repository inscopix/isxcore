#include "isxGroup.h"
#include "isxException.h"

namespace isx
{

Group::Group()
    : m_valid(false)
    , m_modified(false)
    , m_parent(nullptr)
{
}

Group::Group(const std::string & inName, const Type inType)
    : m_valid(true)
    , m_modified(false)
    , m_name(inName)
    , m_type(inType)
    , m_parent(nullptr)
{
}

Group::Type
Group::getType() const
{
    return m_type;
}

Group *
Group::createGroup(const std::string & inPath, const Type inType)
{
    if (isName(inPath))
    {
        ISX_THROW(isx::ExceptionDataIO,
                "There is already an item with the name: ", inPath);
    }
    std::unique_ptr<Group> g(new Group(inPath, inType));
    g->m_parent = this;
    m_groups.push_back(std::move(g));
    m_modified = true;
    return m_groups.back().get();
}

std::vector<Group *>
Group::getGroups(bool inRecurse) const
{
    std::vector<Group *> outGroups;
    std::vector<std::unique_ptr<Group>>::const_iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        outGroups.push_back((*it).get());
        if (inRecurse)
        {
            std::vector<Group *> groups = (*it)->getGroups(true);
            outGroups.insert(outGroups.end(), groups.begin(), groups.end());
        }
    }
    return outGroups;
}

Group *
Group::getGroup(const std::string & inName) const
{
    std::vector<std::unique_ptr<Group>>::const_iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        if ((*it)->m_name == inName)
        {
            return (*it).get();
        }
    }
    ISX_THROW(isx::ExceptionDataIO,
            "Could not find group with the name: ", inName);
}

void
Group::removeGroup(const std::string & inName)
{
    std::vector<std::unique_ptr<Group>>::iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        if ((*it)->m_name == inName)
        {
            (*it)->m_parent = nullptr;
            it = m_groups.erase(it);
            m_modified = true;
            return;
        }
    }
    ISX_THROW(isx::ExceptionDataIO,
            "Could not find group with the name: ", inName);
}

DataSet * 
Group::createDataSet(
    const std::string & inName, 
    DataSet::Type inType,
    const std::string & inFileName,
    const DataSet::Properties & inProperties)
{
    if (isName(inName))
    {
        ISX_THROW(isx::ExceptionDataIO,
                "There is already an item with the name: ", inName);
    }
    if (isFileName(inFileName))
    {
        ISX_THROW(isx::ExceptionFileIO,
                "There is already a data set with the file name: ", inFileName);
    }
    std::unique_ptr<DataSet> ds(new DataSet(inName, inType, inFileName, inProperties));
    ds->setParent(this);
    m_dataSets.push_back(std::move(ds));
    m_modified = true;
    return m_dataSets.back().get();
}

std::vector<DataSet *>
Group::getDataSets(bool inRecurse) const
{
    std::vector<DataSet *> outDataSets;

    std::vector<std::unique_ptr<DataSet>>::const_iterator it;
    for (it = m_dataSets.begin(); it != m_dataSets.end(); ++it)
    {
        outDataSets.push_back((*it).get());
    }

    if (inRecurse)
    {
        std::vector<std::unique_ptr<Group>>::const_iterator it;
        for (it = m_groups.begin(); it != m_groups.end(); ++it)
        {
            std::vector<DataSet *> dataSets = (*it)->getDataSets(true);
            outDataSets.insert(outDataSets.end(), dataSets.begin(), dataSets.end());
        }
    }

    return outDataSets;
}

std::vector<DataSet *>
Group::getDataSets(
        DataSet::Type inType,
        bool inRecurse) const
{
    std::vector<DataSet *> outDataSets;

    std::vector<std::unique_ptr<DataSet>>::const_iterator it;
    for (it = m_dataSets.begin(); it != m_dataSets.end(); ++it)
    {
        if ((*it)->getType() == inType)
        {
            outDataSets.push_back((*it).get());
        }
    }

    if (inRecurse)
    {
        std::vector<std::unique_ptr<Group>>::const_iterator it;
        for (it = m_groups.begin(); it != m_groups.end(); ++it)
        {
            std::vector<DataSet *> dataSets = (*it)->getDataSets(inType, true);
            outDataSets.insert(outDataSets.end(), dataSets.begin(), dataSets.end());
        }
    }

    return outDataSets;
}

DataSet *
Group::getDataSet(const std::string & inName) const
{
    std::vector<std::unique_ptr<DataSet>>::const_iterator it;
    for (it = m_dataSets.begin(); it != m_dataSets.end(); ++it)
    {
        if ((*it)->getName() == inName)
        {
            return (*it).get();
        }
    }
    ISX_THROW(isx::ExceptionDataIO, "Could not find data set with name: ", inName);
}

void
Group::removeDataSet(const std::string & inName)
{
    std::vector<std::unique_ptr<DataSet>>::iterator it;
    for (it = m_dataSets.begin(); it != m_dataSets.end(); ++it)
    {
        if ((*it)->getName() == inName)
        {
            (*it)->setParent(nullptr);
            it = m_dataSets.erase(it);
            m_modified = true;
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

Group *
Group::getParent() const
{
    return m_parent;
}

void
Group::setParent(Group * inParent)
{
    m_parent = inParent;
}

std::string
Group::getPath() const
{
    if (m_parent)
    {
        if (m_parent->m_name == "/")
        {
            return m_parent->m_name + m_name;
        }
        else
        {
            return m_parent->getPath() + "/" + m_name;
        }
    }
    return m_name;
}

bool
Group::isModified() const
{
    std::vector<std::unique_ptr<Group>>::const_iterator groupIt;
    for (groupIt = m_groups.begin(); groupIt != m_groups.end(); ++groupIt)
    {
        if ((*groupIt)->isModified())
        {
            return true;
        }
    }

    std::vector<std::unique_ptr<DataSet>>::const_iterator dataSetIt;
    for (dataSetIt = m_dataSets.begin(); dataSetIt != m_dataSets.end(); ++dataSetIt)
    {
        if ((*dataSetIt)->isModified())
        {
            return true;
        }
    }

    return m_modified;
}

void
Group::setUnmodified()
{
    m_modified = false;
    std::vector<std::unique_ptr<Group>>::const_iterator groupIt;
    for (groupIt = m_groups.begin(); groupIt != m_groups.end(); ++groupIt)
    {
        (*groupIt)->setUnmodified();        
    }

    std::vector<std::unique_ptr<DataSet>>::const_iterator dataSetIt;
    for (dataSetIt = m_dataSets.begin(); dataSetIt != m_dataSets.end(); ++dataSetIt)
    {
        (*dataSetIt)->setUnmodified();
    }
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

Group *
Group::getRootParent()
{
    if (m_parent)
    {
        return m_parent->getRootParent();
    }
    else
    {
        return this;
    }
}

bool
Group::isName(const std::string & inName) const
{
    std::vector<std::unique_ptr<Group>>::const_iterator groupIt;
    for (groupIt = m_groups.begin(); groupIt != m_groups.end(); ++groupIt)
    {
        if ((*groupIt)->getName() == inName)
        {
            return true;
        }
    }

    std::vector<std::unique_ptr<DataSet>>::const_iterator dataSetIt;
    for (dataSetIt = m_dataSets.begin(); dataSetIt != m_dataSets.end(); ++dataSetIt)
    {
        if ((*dataSetIt)->getName() == inName)
        {
            return true;
        }
    }

    return false;
}

bool
Group::isFileName(const std::string & inFileName)
{
    Group * root = getRootParent();
    std::vector<DataSet *> dataSets = root->getDataSets(true);
    std::vector<DataSet *>::const_iterator it;
    for (it = dataSets.begin(); it != dataSets.end(); ++it)
    {
        if ((*it)->getFileName() == inFileName)
        {
            return true;
        }
    }
    return false;
}

} // namespace isx
