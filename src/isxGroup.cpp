#include "isxGroup.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxDataSet.h"
#include "isxSeries.h"

#include "json.hpp"

namespace isx
{

using json = nlohmann::json;

Group::Group()
    : m_valid(false)
    , m_modified(false)
    , m_parent(nullptr)
{
}

Group::Group(const std::string & inName)
    : m_valid(true)
    , m_modified(false)
    , m_parent(nullptr)
    , m_name(inName)
{
}

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

ProjectItem * 
Group::getMostRecent() const 
{
    return nullptr;
}

ProjectItem *
Group::getParent() const
{
    return m_parent;
}

void
Group::setParent(ProjectItem * inParent)
{
    m_parent = inParent;
}

std::vector<ProjectItem *>
Group::getChildren() const
{
    std::vector<ProjectItem *> outChildren;
    for (const auto & child : m_items)
    {
        outChildren.push_back(child.get());
    }
    return outChildren;
}

size_t
Group::getNumChildren() const
{
    return m_items.size();
}

void
Group::insertChild(std::shared_ptr<ProjectItem> inItem, const int inIndex)
{
    validateItemToBeInserted(inItem.get());

    const std::string name = inItem->getName();
    if (isChild(name))
    {
        ISX_THROW(ExceptionDataIO, "There is already an item with the name: ", name);
    }

    // If the child still has a parent, this should remove it
    ProjectItem * parent = inItem->getParent();
    if (parent != nullptr && parent->isChild(name))
    {
        parent->removeChild(name);
    }

    const isize_t destIndex = convertIndex(inIndex, m_items.size());

    inItem->setParent(this);
    m_items.insert(m_items.begin() + destIndex, inItem);
    m_modified = true;
}

std::shared_ptr<ProjectItem>
Group::removeChild(const std::string & inName)
{
    for (auto it = m_items.begin(); it != m_items.end(); ++it)
    {
        if ((*it)->getName() == inName)
        {
            std::shared_ptr<ProjectItem> item = *it;
            item->setParent(nullptr);
            m_items.erase(it);
            m_modified = true;
            return item;
        }
    }
    ISX_THROW(ExceptionDataIO, "Could not find item with the name: ", inName);
}

bool
Group::isModified() const
{
    return m_modified || areChildrenModified();
}

void
Group::setUnmodified()
{
    m_modified = false;
    setChildrenUnmodified();
}

std::string
Group::toJsonString(const bool inPretty) const
{
    json jsonObj;
    jsonObj["itemType"] = size_t(getItemType());
    jsonObj["name"] = m_name;
    jsonObj["items"] = json::array();
    for (const auto & item : m_items)
    {
        jsonObj["items"].push_back(json::parse(item->toJsonString()));
    }
    if (inPretty)
    {
        return jsonObj.dump(4);
    }
    return jsonObj.dump();
}

std::shared_ptr<Group>
Group::fromJsonString(const std::string & inString)
{
    const json jsonObj = json::parse(inString);
    const ProjectItem::Type itemType = ProjectItem::Type(size_t(jsonObj["itemType"]));
    ISX_ASSERT(itemType == ProjectItem::Type::GROUP);
    const std::string name = jsonObj["name"];
    auto outGroup = std::make_shared<Group>(name);
    for (const auto & jsonItem : jsonObj["items"])
    {
        std::shared_ptr<ProjectItem> item;
        const ProjectItem::Type itemType = ProjectItem::Type(isize_t(jsonItem["itemType"]));
        switch (itemType)
        {
            case ProjectItem::Type::GROUP:
            {
                item = Group::fromJsonString(jsonItem.dump());
                break;
            }
            case ProjectItem::Type::SERIES:
            {
                item = Series::fromJsonString(jsonItem.dump());
                break;
            }
            case ProjectItem::Type::DATASET:
            {
                item = DataSet::fromJsonString(jsonItem.dump());
                break;
            }
            default:
            {
                ISX_THROW(ExceptionDataIO, "Project item type not recognized: ", size_t(itemType));
            }
        }
        outGroup->insertChild(item);
    }
    return outGroup;
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
Group::hasHistory() const 
{
    return false;
}

isize_t 
Group::getNumHistoricalItems() const
{
    return 0;
}

bool 
Group::isHistorical() const
{
    return false;
}

} // namespace isx
