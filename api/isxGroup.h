#ifndef ISX_GROUP_H
#define ISX_GROUP_H

#include "isxDataSet.h"

#include <vector>
#include <memory>

namespace isx
{

/// Encapsulates a group which contains data sets and other groups.
///
/// A name in a group must refer either to a data set or a group.
///
/// All data sets are initially ordered by their time of creation and
/// allgroups are initially ordered by their time of creation.
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

    /// Create a new group and it to this group.
    ///
    /// This returns a raw pointer to the new created group after it
    /// has been created and added to this group.
    /// This will fail if a group with the same name as the given group
    /// exists in this group.
    ///
    /// \param  inPath      The path of the data set to create.
    /// \return             A raw pointer to the new group after
    ///                     it has been added to this group.
    ///
    /// \throw  isx::ExceptionDataIO    If a group with the given name
    ///                                 already exists.
    Group * createGroup(const std::string & inPath);

    /// Get the groups in this group.
    ///
    /// \param  inRecurse   If true, recursively get data sets from groups
    ///                     in this group.
    /// \return             The groups in this group.
    std::vector<Group *> getGroups(bool inRecurse = false) const;

    /// Get a group with the given name in this group.
    ///
    /// \param  inName  The name of the sub-group to get.
    /// \return         The sub-group retrieved.
    ///
    /// \throw  isx::ExceptionDataIO    If there is no data set with the
    ///                                 given name in this group.
    Group * getGroup(const std::string & inName) const;

    /// Remove a sub-group with the given name.
    ///
    /// Before removing the group, this sets the parent of the input group
    /// to null.
    ///
    /// \param  inName  The name of the group to remove.
    ///
    /// \throw  isx::ExceptionDataIO    If there is no data set with the
    ///                                 given name in this group.
    void removeGroup(const std::string & inName);

    /// Create a new data set and adds it to this group.
    ///
    /// This returns a raw pointer to the new data set after it has been
    /// created and added to this group.
    /// This will fail if a data set with the given name already exists in
    /// this group.
    ///
    /// \param  inName      The name of the data set.
    /// \param  inType      The type of the data set.
    /// \param  inFileName  The file name of the data set.
    /// \param  inProperties The properties for the data set.
    /// \return             A raw pointer to the new data set after
    ///                     ownership has been taken by this group.
    ///
    /// \throw  isx::ExceptionDataIO    If a data set with the given name
    ///                                 already exists.
    DataSet * createDataSet(
        const std::string & inName,
        DataSet::Type inType,
        const std::string & inFileName, 
        const std::map<std::string, float> & inProperties = std::map<std::string, float>());

    /// Get the data sets in this group.
    ///
    /// If the data sets are recursive option is true the order of the data
    /// sets will be the same as the order of a breadth first search of the
    /// group tree.
    ///
    /// \param  inRecurse   If true, recursively get data sets from groups
    ///                     in this group.
    /// \return             The data sets in this group.
    std::vector<DataSet *> getDataSets(bool inRecurse = false) const;

    /// Get the data sets of a given type in this group.
    ///
    /// If the data sets are recursive option is true the order of the data
    /// sets will be the same as the order of a breadth first search of the
    /// group tree.
    ///
    /// \param  inType      The type of data set to get.
    /// \param  inRecurse   If true, recursively get data sets from groups
    ///                     in this group.
    /// \return             The data sets of a given type in this group.
    std::vector<DataSet *> getDataSets(
            DataSet::Type inType,
            bool inRecurse = false) const;

    /// Get a data set from this group by name.
    ///
    /// \param  inName      The name of the data set to retrieve.
    /// \return             The data set retrieved.
    ///
    /// \throw  isx::ExceptionDataIO    If there is no data set with the
    ///                                 given name in this group.
    DataSet * getDataSet(const std::string & inName) const;

    /// Removes a data set from this group.
    ///
    /// Before removing the group, this sets the parent of the input group
    /// to null.
    ///
    /// \param  inName      The name of the data set to remove.
    ///
    /// \throw  isx::ExceptionDataIO    If there is no data set with the
    ///                                 given name in this group.
    void removeDataSet(const std::string & inName);

    /// Exact comparison.
    ///
    /// \param  inOther     The group with which to compare.
    /// \return             True if the groups are exactly equal.
    bool operator==(const Group & inOther) const;

    // Overrides
    bool isValid() const override;

    std::string getName() const override;

    Group * getParent() const override;

    void setParent(Group * inParent) override;

    std::string getPath() const override;

    bool isModified() const override;

    void setUnmodified() override;

private:

    /// True if this data set is valid.
    bool m_valid;

    /// True if this has unsaved changes.
    bool m_modified;

    /// The name of this group.
    std::string m_name;

    /// The parent group to which this belongs.
    Group * m_parent;

    /// The groups in this group.
    std::vector<std::unique_ptr<Group>> m_groups;

    /// The data sets in this group.
    std::vector<std::unique_ptr<DataSet>> m_dataSets;

    /// \return     The root parent of this group.
    ///
    Group * getRootParent();

    /// Checks if the name is already taken by a group or data set.
    ///
    /// \param  inName  The name to check.
    /// \return         True if there is already a group or data set with the given name.
    ///
    bool isName(const std::string & inName) const;

    /// Checks if the file name is already used by a data set in this group's tree.
    ///
    /// \param  inFileName  The file name to check.
    /// \return             True if this group's tree contains a data set with
    ///                     given file name.
    bool isFileName(const std::string & inFileName);

};

} // namespace isx

#endif // ISX_GROUP_H
