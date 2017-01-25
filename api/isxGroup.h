#ifndef ISX_GROUP_H
#define ISX_GROUP_H

#include "isxProjectItem.h"

#include <iostream>

namespace isx
{

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

    /// Create a group from a serialized JSON string.
    ///
    /// \param  inString    The serialized JSON string.
    /// \return             The deserialized group.
    ///
    /// \throw  ExceptionDataIO If the string cannot be parsed.
    static std::shared_ptr<Group> fromJsonString(const std::string & inString);

    // Overrides: see isxProjectItem.h for docs.
    ProjectItem::Type getItemType() const override;

    bool isValid() const override;

    std::string getName() const override;

    void setName(const std::string & inName) override;

    ProjectItem * getMostRecent() const override;

    ProjectItem * getParent() const override;

    void setParent(ProjectItem * inParent) override;

    std::vector<ProjectItem *> getChildren() const override;

    size_t getNumChildren() const override;

    void insertChild(std::shared_ptr<ProjectItem> inItem, const int inIndex = -1) override;

    std::shared_ptr<ProjectItem> removeChild(const std::string & inName) override;

    bool isModified() const override;

    void setUnmodified() override;

    std::string toJsonString(const bool inPretty = false) const override;

    bool operator ==(const ProjectItem & other) const override;

    bool hasHistory() const override;

    isize_t getNumHistoricalItems() const override;

    bool isHistorical() const override;

private:

    /// True if this data set is valid.
    bool m_valid;

    /// True if this has unsaved changes.
    bool m_modified;

    /// The parent of this group.
    ProjectItem * m_parent;

    /// The name of this group.
    std::string m_name;

    /// The items in this group.
    std::vector<std::shared_ptr<ProjectItem>> m_items;

};

} // namespace isx

#endif // ISX_GROUP_H
