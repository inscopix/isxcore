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
class Group : public std::enable_shared_from_this<Group>
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

    /// Create an empty group in this group.
    ///
    /// If a group with the given name exists in this group, this will fail.
    ///
    /// \param  inName      The name of the group to create.
    /// \return             The group created.
    ///
    /// \throw  isx::ExceptionDataIO    If a group with the given name
    ///                                 already exists.
    SpGroup_t createGroup(const std::string & inName);

    /// Add a group to this group.
    ///
    /// \param  inGroup     The group to add to this.
    ///
    /// \throw  isx::ExceptionDataIO    If a data set with the same name
    ///                                 already exists in this group.
    void addGroup(const SpGroup_t & inGroup);

    /// Get the groups in this group.
    ///
    /// \param  inRecurse   If true, recursively get data sets from groups
    ///                     in this group.
    /// \return             The groups in this group.
    std::vector<SpGroup_t> getGroups(bool inRecurse = false) const;

    /// Get a group with the given name in this group.
    ///
    /// \param  inName  The name of the sub-group to get.
    /// \return         The sub-group retrieved.
    ///
    /// \throw  isx::ExceptionDataIO    If there is no data set with the
    ///                                 given name in this group.
    SpGroup_t getGroup(const std::string & inName) const;

    /// Remove a sub-group with the given name.
    ///
    /// \param  inName  The name of the group to remove.
    ///
    /// \throw  isx::ExceptionDataIO    If there is no data set with the
    ///                                 given name in this group.
    void removeGroup(const std::string & inName);

    /// Create a data set in this group.
    ///
    /// If a data set with the given name already exists in this group,
    /// this will fail.
    /// If there is an existing data set anywhere in this group's tree
    /// (recursing down from its root ancestor) with the given file name
    /// this will fail.
    ///
    /// \param  inName      The name of the data set to create.
    /// \param  inType      The type of the data set to create.
    /// \param  inFileName  The file name of the data set to create.
    /// \return             The data set created.
    ///
    /// \throw  isx::ExceptionDataIO    If a data set with the given name
    ///                                 already exists.
    /// \throw  isx::ExceptionFileIO    If a data set with the given file
    ///                                 name already exists in this group's tree.
    SpDataSet_t createDataSet(
            const std::string & inName,
            DataSet::Type inType,
            const std::string & inFileName);

    /// Adds a data set to this group.
    ///
    /// This will fail if there is a data set in this group that has the
    /// same name as the the name of the given data set.
    ///
    /// \param  inDataSet   The data set to add.
    ///
    /// \throw  isx::ExceptionDataIO    If a data set with the same name
    ///                                 already exists in this group.
    void addDataSet(const SpDataSet_t & inDataSet);

    /// Get the data sets in this group.
    ///
    /// If the data sets are recursive option is true the order of the data
    /// sets will be the same as the order of a breadth first search of the
    /// group tree.
    ///
    /// \param  inRecurse   If true, recursively get data sets from groups
    ///                     in this group.
    /// \return             The data sets in this group.
    std::vector<SpDataSet_t> getDataSets(bool inRecurse = false) const;

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
    std::vector<SpDataSet_t> getDataSets(
            DataSet::Type inType,
            bool inRecurse = false) const;

    /// Get a data set from this group by name.
    ///
    /// \param  inName      The name of the data set to retrieve.
    /// \return             The data set retrieved.
    ///
    /// \throw  isx::ExceptionDataIO    If there is no data set with the
    ///                                 given name in this group.
    SpDataSet_t getDataSet(const std::string & inName) const;

    /// Removes a data set from this group.
    ///
    /// \param  inName      The name of the data set to remove.
    ///
    /// \throw  isx::ExceptionDataIO    If there is no data set with the
    ///                                 given name in this group.
    void removeDataSet(const std::string & inName);

    /// \return     True if this group is valid.
    ///
    bool isValid() const;

    /// \return     The name of this group.
    ///
    std::string getName() const;

    /// \return     True if this has a parent.
    ///
    bool hasParent() const;

    /// \return     The parent of this group.
    ///
    SpGroup_t getParent() const;

    /// Set the parent of this group.
    ///
    /// \param  inParent    The new parent of this group.
    ///
    /// \throw  isx::ExceptionDataIO    If the parent already contains a group
    ///                                 with the same name as this.
    void setParent(SpGroup_t & inParent);

    /// \return     The path of this group from the root group.
    ///
    std::string getPath() const;

    /// \return     The root parent of this group.
    ///
    std::shared_ptr<const Group> getRootParent() const;

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
    bool isFileName(const std::string & inFileName) const;

    /// Exact comparison.
    ///
    /// \param  inOther     The group with which to compare.
    /// \return             True if the groups are exactly equal.
    bool operator==(const Group & inOther) const;

private:

    /// True if this data set is valid.
    bool m_valid;

    /// The name of this group.
    std::string m_name;

    /// The parent group to which this belongs.
    SpGroup_t m_parent;

    /// The groups in this group.
    std::vector<SpGroup_t> m_groups;

    /// The data sets in this group.
    std::vector<SpDataSet_t> m_dataSets;

};

} // namespace isx

#endif // ISX_GROUP_H
