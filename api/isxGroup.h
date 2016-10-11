#ifndef ISX_GROUP_H
#define ISX_GROUP_H

#include "isxDataSet.h"

#include <vector>
#include <memory>
#include <string>

namespace isx
{

/// Encapsulates a group which contains data sets and other groups.
///
/// A name in a group must refer either to a data set or a group.
///
/// All data sets are initially ordered by their time of creation and
/// allgroups are initially ordered by their time of creation.
class Group
{
public:

    /// The type of group.
    ///
    enum class Type
    {
        GENERAL,        /// A general group just for organization.
        DATASET,            /// A group that stores a data set and its derived datasets, etc.
        DERIVED             /// A group that stores derived data sets.
    };

    /// Empty constructor.
    ///
    /// Creates a valid C++ object, but not a valid Group.
    Group();

    /// Create a group with the given name.
    ///
    /// \param  inName      The name of this group.
    /// \param  inType      The type of group.
    Group(const std::string & inName, const Type inType = Type::GENERAL);

    /// \return     The type of this group.
    ///
    Type getType() const;

    /// Create a new group and add it to this group.
    ///
    /// This returns a raw pointer to the new created group after it
    /// has been created and added to this group.
    /// This will fail if a group with the same name as the given group
    /// exists in this group.
    ///
    /// \param  inPath      The path of the data set to create.
    /// \param  inType      The type of group.
    /// \return             A raw pointer to the new group after
    ///                     it has been added to this group.
    ///
    /// \throw  isx::ExceptionDataIO    If a group with the given name
    ///                                 already exists.
    Group * createGroup(const std::string & inPath, const Type inType = Type::GENERAL);

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

    /// Checks that a group with a given name exists in this group.
    ///
    /// \param  inName  The name of the group to check for.
    /// \return         True if the group exists.
    bool isGroup(const std::string & inName) const;

    /// Checks that a data set with a given name exists in this group.
    ///
    /// \param  inName  The name of the data set to check for.
    /// \return         True if the data set exists.
    bool isDataSet(const std::string & inName) const;

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
        const DataSet::Properties & inProperties = DataSet::Properties());

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

    /// \return     True if this is valid.
    ///
    bool isValid() const;

    /// \return     The name of this data set.
    ///
    std::string getName() const;

    /// \return     The parent of this group.
    ///
    Group * getParent() const;

    /// Set the parent of this data set.
    ///
    /// This simply updates the parent of this data set and does not move
    /// this data set into the given parent group,
    ///
    /// \param  inParent    The new parent of this data set.
    void setParent(Group * inParent);

    /// \return     The path of this group from the root group.
    ///
    std::string getPath() const;

    /// Returns whether the item has been modified since last save.
    ///
    bool isModified() const;

    /// Sets the item flag indicating wehther there are unsaved changes to false.
    ///
    void setUnmodified();

private:

    /// True if this data set is valid.
    bool m_valid;

    /// True if this has unsaved changes.
    bool m_modified;

    /// The name of this group.
    std::string m_name;

    /// The type of this group.
    Type m_type;

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
