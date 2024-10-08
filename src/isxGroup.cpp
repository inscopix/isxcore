#include "isxGroup.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxDataSet.h"
#include "isxSeries.h"
#include "isxPathUtils.h"
#include "isxProject.h"

#include "json.hpp"

#include <algorithm>
#include <set>

namespace isx
{

using json = nlohmann::json;

Group::Group()
    : m_valid(false)
    , m_modified(false)
    , m_container(nullptr)
{
}

Group::Group(const std::string & inName)
    : m_valid(true)
    , m_modified(false)
    , m_container(nullptr)
    , m_name(inName)
{
}

Group::~Group() = default;

ProjectItem::Type
Group::getItemType() const
{
    return ProjectItem::Type::GROUP;
}

bool
Group::isValid() const
{
    return m_valid;
}

const std::string &
Group::getName() const
{
    return m_name;
}

void
Group::setName(const std::string & inName)
{
    std::string uniqueName = getUniqueName(inName);
    if (m_name != uniqueName)
    {
        setUniqueName(uniqueName);
    }   
}

void 
Group::setUniqueName(const std::string & inName)
{
    m_name = inName;
    setModified();
}

ProjectItem *
Group::getGroupMember(isize_t inIndex) const
{
    if (inIndex >= m_items.size())
    {
        ISX_THROW(ExceptionDataIO, "There is no group member with index: ", inIndex);
    }

    return m_items.at(inIndex).get();
}
    
size_t
Group::getNumGroupMembers() const
{
    return m_items.size();
}
    
std::vector<ProjectItem *>
Group::getGroupMembers() const
{
    std::vector<ProjectItem *> ret;
    for (const auto & i : m_items)
    {
        ret.push_back(i.get());
    }
    return ret;
}

void
Group::insertGroupMember(std::shared_ptr<ProjectItem> inItem, const isize_t inIndex)
{
    if (inItem.get() == static_cast<ProjectItem *>(this))
    {
        ISX_THROW(ExceptionDataIO, "An item cannot be inserted in itself.");
    }

    auto container = getContainer();
    while (container != nullptr)
    {
        if (inItem.get() == container)
        {
            ISX_THROW(ExceptionDataIO, "The inserted item is an ancestor of this.");
        }
        container = container->getContainer();
    }

    // Throw if the item is still in another container
    if (inItem->getContainer())
    {
        ISX_THROW(ExceptionDataIO, "New item is still inside of another container: ", inItem->getName());
    }

    auto index = std::min(inIndex, m_items.size());

    // Make sure the item will have a unique name in the project
    std::string uniqueName = getUniqueName(inItem->getName());
    if(uniqueName != inItem->getName())
    {
        inItem->setName(uniqueName);
    }

    inItem->setContainer(this);
    m_items.insert(m_items.begin() + index, inItem);
    setModified();
}

SpProjectItem_t
Group::removeGroupMember(ProjectItem * inItem)
{
    auto it = std::find_if(m_items.begin(), m_items.end(), [inItem](const SpProjectItem_t & s)
        {
            return s.get() == inItem;
        });
    
    if (it == m_items.end())
    {
        ISX_THROW(ExceptionDataIO, "Could not find item with the name: ", inItem->getName());
    }
    auto item = *it;
    item->setContainer(nullptr);
    m_items.erase(it);
    setModified();
    return item;
}
    
isize_t
Group::getMemberIndex() const
{
    isize_t index = 0;
    ISX_ASSERT(m_container, "Orphaned child does not have a owning group.");

    if (m_container == nullptr)
    {
        return index;
    }
    for (const auto & m : m_container->getGroupMembers())
    {
        if (m == this)
        {
            return index;
        }
        ++index;
    }
    ISX_ASSERT(false, "Non-orphaned child cannot be found in parent.");
    return index;
}
    
void
Group::setContainer(ProjectItem * inContainer)
{
    if (inContainer->getItemType() != ProjectItem::Type::GROUP)
    {
        ISX_THROW(Exception, "Group can only be in another group.");
    }
    else
    {
        m_container = static_cast<Group *>(inContainer);
    }
}

ProjectItem *
Group::getContainer() const
{
    return m_container;
}

bool 
Group::isNameUsed(const std::string & inName) const 
{
    if (m_name == inName)
    {
        return true;
    }
    for (const auto & i : m_items)
    {
        if (i->isNameUsed(inName))
        {
            return true;
        }
    }
    return false;
}

bool
Group::isModified() const
{
    if (m_modified)
    {
        return true;
    }
    for (const auto & i : m_items)
    {
        if (i->isModified())
        {
            return true;
        }
    }
    return false;
}

void 
Group::setModified()
{
    m_modified = true;
    saveTemporaryChanges();
}

void
Group::setUnmodified()
{
    m_modified = false;
    for (const auto & i : m_items)
    {
        i->setUnmodified();
    }
}
    
std::string
Group::toJsonString(const bool inPretty, const std::string & inPathToOmit) const
{
    json jsonObj;
    jsonObj["itemType"] = size_t(getItemType());
    jsonObj["name"] = m_name;
    jsonObj["items"] = json::array();
    for (const auto & item : m_items)
    {
        jsonObj["items"].push_back(json::parse(item->toJsonString(inPretty, inPathToOmit)));
    }
    if (inPretty)
    {
        return jsonObj.dump(4);
    }
    return jsonObj.dump();
}

std::shared_ptr<Group>
Group::fromJsonString(const std::string & inString, const std::string & inAbsolutePathToPrepend)
{
    const json jsonObj = json::parse(inString);
    const ProjectItem::Type itemType = ProjectItem::Type(size_t(jsonObj.at("itemType")));
    ISX_ASSERT(itemType == ProjectItem::Type::GROUP);
    const std::string name = jsonObj.at("name");
    auto ret = std::make_shared<Group>(name);
    for (const auto & jsonItem : jsonObj.at("items"))
    {
        std::shared_ptr<ProjectItem> item;
        const ProjectItem::Type itemType = ProjectItem::Type(isize_t(jsonItem.at("itemType")));
        switch (itemType)
        {
            case ProjectItem::Type::GROUP:
            {
                item = Group::fromJsonString(jsonItem.dump(), inAbsolutePathToPrepend);
                break;
            }
            case ProjectItem::Type::SERIES:
            {
                item = Series::fromJsonString(jsonItem.dump(), inAbsolutePathToPrepend);
                break;
            }
            default:
            {
                ISX_THROW(ExceptionDataIO, "Project item type not recognized: ", size_t(itemType));
            }
        }
        ret->insertGroupMember(item, ret->getNumGroupMembers());
    }
    return ret;
}

bool
Group::operator ==(const ProjectItem & other) const
{
    if (other.getItemType() != ProjectItem::Type::GROUP)
    {
        return false;
    }
    auto otherGroup = static_cast<const Group *>(&other);

    bool equal = (m_name == otherGroup->m_name)
        && (m_items.size() == otherGroup->m_items.size());
    for (size_t i = 0; equal && i < m_items.size(); ++i)
    {
        equal &= *m_items.at(i) == *otherGroup->m_items.at(i);
    }
    return equal;
}
    
bool
Group::isGroupMember(const ProjectItem * inItem) const
{
    return isGroupMember(inItem->getName());
}

bool
Group::isGroupMember(const std::string & inName) const
{
    return std::any_of(m_items.begin(), m_items.end(), [inName](const SpProjectItem_t & s)
        {
            return s->getName() == inName;
        });
}

std::string 
Group::findUniqueName(const std::string & inRequestedName)
{
    std::string uniqueName = inRequestedName;

    isx::isize_t i(1);

    while (isNameUsed(uniqueName))
    {
        uniqueName = appendNumberToPath(inRequestedName, i, 3);
        ++i;
    }

    return uniqueName;
}

std::vector<std::string>
Group::getUsedFileNames() const
{
    std::vector<std::string> usedFileNames;

    for (const auto & i : m_items)
    {
        const auto descendants = i->getUsedFileNames();
        usedFileNames.insert(usedFileNames.end(), descendants.begin(), descendants.end());
    }

    return usedFileNames;
}

std::string
Group::makeUniqueFilePath(const std::string & inInitialPath)
{
    std::string uniquePath = inInitialPath;

    const std::string dir = getDirName(uniquePath);
    const std::string base = dir + "/" + getBaseName(uniquePath);
    const std::string extension = getExtension(uniquePath);

    std::set<std::string> filesToBeVerified;

    auto filesInSpecifiedDirectory = getAllDirFiles(dir);
    filesToBeVerified.insert(filesInSpecifiedDirectory.begin(), filesInSpecifiedDirectory.end());

    auto filesOfCurrentGroup = getUsedFileNames();
    filesToBeVerified.insert(filesOfCurrentGroup.begin(), filesOfCurrentGroup.end());

    isize_t i = 1;
    while (filesToBeVerified.end() != filesToBeVerified.find(uniquePath))
    {
        uniquePath = appendNumberToPath(base, i, 3) + "." + extension;
        ++i;
    }

    return uniquePath;
}


void 
Group::setSaveTempProjectCallback(SaveTempProjectCB_t inCallback)
{
    m_saveTempProjectCB = inCallback;
}

void 
Group::saveTemporaryProject()
{
    if (m_saveTempProjectCB)
    {
        m_saveTempProjectCB();
    }
}
    
std::ostream &
operator<<(::std::ostream & inStream, const Group & inGroup)
{
    inStream << inGroup.toJsonString(true);
    return inStream;
}
} // namespace isx
