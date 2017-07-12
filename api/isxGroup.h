#ifndef ISX_GROUP_H
#define ISX_GROUP_H

#include "isxProjectItem.h"

#include <iostream>

namespace isx
{

class Project;

/// A group organizes items within a project.
///
/// This acts similarly to a directory on a file system.
class Group : public ProjectItem
{
public:

    /// Empty constructor.
    ///
    /// Creates a valid C++ object, but not a valid Group.
    Group();

    /// Create a group with the given name.
    ///
    /// \param  inName      The name of this group.
    Group(const std::string & inName);
    
    /// Destructor.
    ///
    ~Group();

    /// Create a group from a serialized JSON string.
    ///
    /// \param  inString    The serialized JSON string.
    /// \param  inAbsolutePathToPrepend The path the is preprended to any relative filenames in the group
    /// \return             The deserialized group.
    ///
    /// \throw  ExceptionDataIO If the string cannot be parsed.
    static SpGroup_t fromJsonString(const std::string & inString, const std::string & inAbsolutePathToPrepend = std::string());
    
    /// \return number of this Group's members.
    ///
    isize_t
    getNumGroupMembers() const;
    
    /// \return vector containing raw pointers to this Group's members.
    ///
    std::vector<ProjectItem *>
    getGroupMembers() const;
    
    /// \return Retrieve this Group's members by index.
    ///
    ProjectItem *
    getGroupMember(isize_t inIndex) const;
    
    /// Insert a new item into this Group as a member.
    /// \param inItem   New item to be inserted.
    /// \param inIndex  Position at which to insert into the group.
    ///
    /// \throw ExceptionDataIO  If inItem is the same as this Group instance.
    /// \throw ExceptionDataIO  If inItem is an ancestor of this Group.
    /// \throw ExceptionDataIO  If this Group already has a member with the same name as inItem's name.
    /// \throw ExceptionDataIO  If inItem is still in another Group.
    void
    insertGroupMember(SpProjectItem_t inItem, const isize_t inIndex);
    
    /// Remove a Group member
    /// \param inItem   Member to remove.
    /// \return The project item that was removed.
    SpProjectItem_t
    removeGroupMember(ProjectItem * inItem);

    /// \return index of this Series in its owning Group
    ///
    isize_t
    getMemberIndex() const;

    /// \return a unique name in the tree under this node composed of the requested name and 
    /// and a three digit number
    /// \param inRequestedName the requested name
    std::string 
    findUniqueName(const std::string & inRequestedName);

   
    // Overrides: see isxProjectItem.h for docs.
    ProjectItem::Type getItemType() const override;

    bool isValid() const override;

    const std::string & getName() const override;

    void setName(const std::string & inName) override;

    void setContainer(ProjectItem * inContainer) override;
    
    ProjectItem * getContainer() const override;
    
    bool isModified() const override;
    
    void setUnmodified() override;

    std::string toJsonString(const bool inPretty = false, const std::string & inPathToOmit = std::string()) const override;
    
    bool operator ==(const ProjectItem & other) const override;

    bool isNameUsed(const std::string & inName) const override;
    
private:
    
    bool
    isGroupMember(const ProjectItem * inItem) const;

    bool
    isGroupMember(const std::string & inName) const;

    void 
    setUniqueName(const std::string & inName);
    

    /// True if this data set is valid.
    bool m_valid;

    /// True if this has unsaved changes.
    bool m_modified;

    /// The container that holds this group.
    Group * m_container = nullptr;

    /// The name of this group.
    std::string m_name;

    /// The items in this group.
    std::vector<SpProjectItem_t> m_items;

};

/// STL overload for print to a stream.
///
/// \param   inStream   The output stream to which to print.
/// \param   inGroup    The group to print.
/// \return             The modified stream.
std::ostream & operator<<(::std::ostream & inStream, const Group & inGroup);

} // namespace isx

#endif // ISX_GROUP_H
