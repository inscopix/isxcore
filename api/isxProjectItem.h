#ifndef ISX_PROJECT_ITEM_H
#define ISX_PROJECT_ITEM_H

#include "isxCore.h"
#include "isxCoreFwd.h"

#include <vector>
#include <memory>
#include <string>

namespace isx
{

class Group;
/// An interface for an item in a project, which can be a group, series or data set.
///
class ProjectItem
{
public:

    /// The type of an item.
    ///
    enum class Type
    {
        GROUP,          /// Organizes other project items like a directory.
        SERIES,         /// Represents a series of data sets.
    };

    virtual ~ProjectItem(){};

    /// \return The type of this item.
    ///
    virtual Type getItemType() const = 0;

    /// \return True if this is valid.
    ///
    virtual bool isValid() const = 0;

    /// \return The name of this data set.
    ///
    virtual const std::string & getName() const = 0;

    /// \param  inName  The new name of the item.
    ///
    virtual void setName(const std::string & inName) = 0;

    /// \param inName The name to test 
    /// \return true if the name is in use in this item's tree
    virtual bool isNameUsed(const std::string & inName) const = 0;

    /// Set the parent container of this item.
    /// \param inContainer The new container for this item.
    ///
    virtual void setContainer(ProjectItem * inContainer) = 0;
    
    /// \return The parent Group of this item.
    ///
    virtual ProjectItem * getContainer() const = 0;
    
    /// \return True if the item has been modified since last save, false otherwise.
    ///
    virtual bool isModified() const = 0;

    /// Sets the item flag indicating whether there are unsaved changes to false.
    ///
    virtual void setUnmodified() = 0;

    /// \param  inPretty    If true make the serialization pretty, ugly otherwise.
    /// \param  inPathToOmit the path from filenames to omit for serialization (to obtain relative paths)
    /// \return             The serialized JSON string of the item.
    virtual std::string toJsonString(const bool inPretty = false, const std::string & inPathToOmit = std::string()) const = 0;

    

    /// Exact comparison.
    ///
    /// \param  inOther     The items with which to compare.
    /// \return             True if the items are exactly equal.
    virtual bool operator==(const ProjectItem & inOther) const = 0;

    /// Create a project item from a serialized JSON string.
    ///
    /// \param  inString    The serialized JSON string.
    /// \return             The deserialized project item.
    ///
    /// \throw  ExceptionDataIO If the string cannot be parsed.
    static std::shared_ptr<ProjectItem> fromJsonString(const std::string & inString);

protected: 

    /// \return a pointer to the root of the tree. 
    /// If none is found, returns a nullptr
    Group * getRoot();

    /// Navigate up to the Project instance that this project item belongs to and check that the project item name
    /// is unique in the project. If not, return a unique name, based on the requested name, with a number appended to the end
    /// \param inRequestedName the desired name for the project item
    /// \return the unique version of item's name
    std::string getUniqueName(const std::string & inRequestedName);

};
} // namespace isx

#endif // ISX_PROJECT_ITEM_H
