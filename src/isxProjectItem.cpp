#include "isxProjectItem.h"
#include "isxAssert.h"
#include "isxException.h"
#include "isxGroup.h"
#include "isxDataSet.h"
#include "isxSeries.h"

#include "json.hpp"

namespace isx
{

using json = nlohmann::json;

ProjectItem::~ProjectItem()
{
}

ProjectItem *
ProjectItem::getChild(const std::string & inName) const
{
    for (const auto & child : getChildren())
    {
        if (child->getName() == inName)
        {
            return child;
        }
    }
    ISX_THROW(ExceptionDataIO, "Could not find child with the name: ", inName);
}

ProjectItem *
ProjectItem::getChild(const isize_t inIndex) const
{
    const std::vector<ProjectItem *> children = getChildren();
    if (inIndex >= children.size())
    {
        ISX_THROW(ExceptionDataIO, "The index is greater than or equal to the number of children.");
    }
    return children.at(inIndex);
}

bool
ProjectItem::isChild(const std::string & inName) const
{
    for (const auto & child : getChildren())
    {
        if (child->getName() == inName)
        {
            return true;
        }
    }
    return false;
}

std::string
ProjectItem::getPath() const
{
    const ProjectItem * parent = getParent();
    const std::string name = getName();
    if (parent == nullptr)
    {
        return name;
    }
    const std::string parentName = parent->getName();
    if (parentName == "/")
    {
        return parentName + name;
    }
    else
    {
        return parent->getPath() + "/" + name;
    }
}

int
ProjectItem::getIndex() const
{
    int index = -1;
    const ProjectItem * parent = getParent();
    if (parent == nullptr)
    {
        return index;
    }
    for (const auto & child : parent->getChildren())
    {
        ++index;
        if (child == this)
        {
            return index;
        }
    }
    ISX_ASSERT(false, "Non-orphaned child cannot be found in parent.");
    return index;
}

std::shared_ptr<ProjectItem>
ProjectItem::fromJsonString(const std::string & inString)
{
    json jsonObj = json::parse(inString);
    ProjectItem::Type itemType = ProjectItem::Type(isize_t(jsonObj["itemType"]));
    switch (itemType)
    {
        case ProjectItem::Type::GROUP:
        {
            return Group::fromJsonString(jsonObj.dump());
        }
        case ProjectItem::Type::SERIES:
        {
            return Series::fromJsonString(jsonObj.dump());
        }
        case ProjectItem::Type::DATASET:
        {
            return DataSet::fromJsonString(jsonObj.dump());
        }
        default:
        {
            ISX_THROW(ExceptionDataIO, "Project item type not recognized: ", size_t(itemType));
        }
    }
}

void
ProjectItem::validateItemToBeInserted(const ProjectItem * inItem) const
{
    if (inItem == this)
    {
        ISX_THROW(ExceptionDataIO, "An item cannot be inserted in itself.");
    }

    ProjectItem * parent = getParent();
    while (parent != nullptr)
    {
        if (inItem == parent)
        {
            ISX_THROW(ExceptionDataIO, "The inserted item is an ancestor of this.");
        }
        parent = parent->getParent();
    }
}

bool
ProjectItem::areChildrenModified() const
{
    for (const auto & child : getChildren())
    {
        if (child->isModified())
        {
            return true;
        }
    }
    return false;
}

void
ProjectItem::setChildrenUnmodified()
{
    for (const auto & child : getChildren())
    {
        child->setUnmodified();
    }
}

size_t
ProjectItem::convertIndex(const int inIndex, const size_t inSize)
{
    size_t index = size_t(inIndex);
    if (inIndex < 0 || index > inSize)
    {
        index = inSize;
    }
    return index;
}

::std::ostream &
operator<<(::std::ostream & inStream, const ProjectItem & inItem)
{
    inStream << inItem.toJsonString(true);
    return inStream;
}

} // namespace isx
