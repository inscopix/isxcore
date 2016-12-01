#include "isxGroup.h"
#include "isxException.h"
#include "isxAssert.h"

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
Group::createGroup(
        const std::string & inPath,
        const Type inType,
        const int inIndex)
{
    if (isName(inPath))
    {
        ISX_THROW(isx::ExceptionDataIO,
                "There is already an item with the name: ", inPath);
    }
    std::unique_ptr<Group> group(new Group(inPath, inType));
    return insertGroup(std::move(group), inIndex);
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

bool
Group::isGroup(const std::string & inName) const
{
    std::vector<std::unique_ptr<Group>>::const_iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        if ((*it)->m_name == inName)
        {
            return true;
        }
    }
    return false;
}

bool
Group::isDataSet(const std::string & inName) const
{
    std::vector<std::unique_ptr<DataSet>>::const_iterator it;
    for (it = m_dataSets.begin(); it != m_dataSets.end(); ++it)
    {
        if ((*it)->getName() == inName)
        {
            return true;
        }
    }
    return false;
}

void
Group::moveGroup(
        const std::string & inName,
        Group * inDest,
        const int inIndex)
{
    // In the case of an internal move, we need to do some insert/erase
    // index adjustment so using indexed for loop instead of iterator
    for (size_t i = 0; i < m_groups.size(); ++i)
    {
        if (m_groups.at(i)->m_name == inName)
        {
            size_t eraseIndex = i;
            int insertIndex = inIndex;

            // An internal move needs some special handling because the size the
            // vector will temporarily change.
            if (this == inDest)
            {
                // Ignore the identity move
                if (int(i) == inIndex)
                {
                    return;
                }

                if (inIndex != -1)
                {
                    // NOTE sweet : If the insertion index is before the original index,
                    // then we need to adjust the erasure index by 1 because there will
                    // be a new element before the original one.
                    //
                    // If we didn't do this, then consider these examples.
                    // groups = {a, b, c}
                    // move(c, 0) -> {c, a, b, null} -> {c, a, null} != {c, a, b}
                    // or
                    // move(c, 1) -> {a, c, b, null} -> {a, c, null} != {a, c, b}
                    if (inIndex < int(i))
                    {
                        eraseIndex = i + 1;
                    }
                    // If the insertion index is after the original index, then we need
                    // to adjust the insertion index by 1 because the insertion occurs
                    // before the specified index.
                    //
                    // If we didn't do this, then consider these examples.
                    // groups = {a, b, c}
                    // move(a, 1) -> {null, a, b, c} -> {a, b, c} != {b, a, c}
                    // or
                    // move(a, 2) -> {null, b, a, c} -> {b, a, c} != {b, c, a}
                    else if (inIndex)
                    {
                        insertIndex = inIndex + 1;
                    }
                }
            }

            inDest->insertGroup(std::move(m_groups.at(i)), insertIndex);
            m_groups.erase(m_groups.begin() + eraseIndex);
            m_modified = true;
            return;
        }
    }
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

isx::DataSet *
Group::getDataSetFromGroup() const
{
    if (getType() != isx::Group::Type::DATASET)
    {
        return nullptr;
    }
    const std::string dsName = getName();
    if (isDataSet(dsName))
    {
        return getDataSet(dsName);
    }
    return nullptr;
}

void
Group::moveDataSet(const std::string & inName, Group * inDest)
{
    std::vector<std::unique_ptr<DataSet>>::iterator it;
    for (it = m_dataSets.begin(); it != m_dataSets.end(); ++it)
    {
        if ((*it)->getName() == inName)
        {
            (*it)->setParent(inDest);
            inDest->m_dataSets.push_back(std::move(*it));
            it = m_dataSets.erase(it);
            m_modified = true;
            return;
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

void
Group::setName(const std::string & inName)
{
    m_name = inName;
    m_modified = true;
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

int
Group::getIndex() const
{
    int index = -1;
    if (m_parent == nullptr)
    {
        return index;
    }
    for (const auto & group : m_parent->m_groups)
    {
        ++index;
        if (group.get() == this)
        {
            return index;
        }
    }
    ISX_ASSERT(false, "Non-orphaned child cannot be found in parent.");
    return index;
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

Group *
Group::insertGroup(std::unique_ptr<Group> inGroup, const int inIndex)
{
    inGroup->m_parent = this;
    size_t index = size_t(inIndex);
    if (inIndex < 0 || index >= m_groups.size())
    {
        index = m_groups.size();
    }
    auto it = m_groups.insert(m_groups.begin() + index, std::move(inGroup));
    m_modified = true;
    return (*it).get();
}

} // namespace isx
