#ifndef ISX_PROJECT_ITEM_H
#define ISX_PROJECT_ITEM_H

#include <string>
#include <memory>

namespace isx
{

// Forward declare needed because the parent of project item can only
// be a group at the moment.
class Group;

/// Encapsulates the interface to an item in a project.
///
/// This just defines the basic access to the item's name and parent.
class ProjectItem
{
public:

    /// \return     True if this is valid.
    ///
    virtual bool isValid() const = 0;

    /// \return     The name of this data set.
    ///
    virtual std::string getName() const = 0;

    /// \return     The parent of this group.
    ///
    virtual Group * getParent() const = 0;

    /// \return     The path of this group from the root group.
    ///
    virtual std::string getPath() const = 0;

protected:

    /// Set the parent of this data set.
    ///
    /// This simply updates the parent of this data set and does not move
    /// this data set into the given parent group,
    /// Use Group::removeProjectItem and Group::addProjectItem for that.
    ///
    /// \param  inParent    The new parent of this data set.
    virtual void setParent(Group * inParent) = 0;

}; // class ProjectItem

} // namespace isx

#endif // ISX_PROJECT_ITEM_H
