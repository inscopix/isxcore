#include "isxProjectItem.h"
#include "isxGroup.h"
#include "isxSeries.h"


namespace isx
{

Group * ProjectItem::getRoot()
{
    ProjectItem * topMostItem = this;
    Group * root = nullptr;

    while (topMostItem != nullptr)
    {
        ProjectItem::Type type = topMostItem->getItemType();
        ProjectItem * pi = nullptr;

        switch (type)
        {
            case ProjectItem::Type::GROUP:
            {
                pi = topMostItem->getContainer();
                break;
            }

            case ProjectItem::Type::SERIES:
            {
                Series * s = (Series *) topMostItem;
                pi = s->getContainer() ? s->getContainer() : s->getParent();
                break;
            }
        }

        if (!pi && (topMostItem->getItemType() == ProjectItem::Type::GROUP))
        {
            root = reinterpret_cast<Group *> (topMostItem);
        }

        topMostItem = pi;
    }

    return root;
}


std::string ProjectItem::getUniqueName(const std::string & inRequestedName)
{
    Group * root = getRoot();
    return root ? root->findUniqueName(inRequestedName) : inRequestedName;
}

}
