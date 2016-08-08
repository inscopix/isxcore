#ifndef ISX_GROUP_H
#define ISX_GROUP_H

#include "isxDataSet.h"

#include <vector>
#include <memory>

namespace isx
{

/// Encapsulates a group in a project which contains data sets other groups.
///
/// All data sets are initially ordered by their time of creation.
/// All groups are initially ordered by their time of creation.
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

    /// Add a group to this group.
    ///
    /// \param  inGroup     The group to add to this.
    void addGroup(const SpGroup_t & inGroup);

    /// \return     The groups in this group.
    ///
    std::vector<SpGroup_t> getGroups() const;

    /// Get a group with the given name in this group.
    ///
    /// \param  inName  The name of the sub-group to get.
    /// \return         The sub-group retrieved.
    ///
    /// \throw  isx::ExceptionDataIO    If the group does not exist.
    SpGroup_t getGroup(const std::string & inName) const;

    /// Remove a sub-group with the given name.
    ///
    /// \param  inName  The name of the group to remove.
    ///
    /// \throw  isx::ExceptionDataIO    If the group does not exist.
    void removeGroup(const std::string & inName);

    /// Adds a data set to this group.
    ///
    /// \param  inDataSet   The data set to add.
    void addDataSet(const SpDataSet_t & inDataSet);

    /// \return     The data sets in this group.
    ///
    /// TODO sweet : add recursive option
    std::vector<SpDataSet_t> getDataSets() const;

    /// Get the data sets of a given type in this group.
    ///
    /// \param  inType      The type of data set to get.
    /// \return             The data sets of a given type in this group.
    ///
    /// TODO sweet : add recursive option
    std::vector<SpDataSet_t> getDataSets(DataSet::Type inType) const;

    /// Get a data set from this group by name.
    ///
    /// \param  inName      The name of the data set to retrieve.
    /// \return             The data set retrieved.
    ///
    /// \throw  isx::ExceptionDataIO    If the data set does not exist.
    SpDataSet_t getDataSet(const std::string & inName) const;

    /// Removes a data set from this group.
    ///
    /// \param  inName      The name of the data set to remove.
    ///
    /// \throw  isx::ExceptionDataIO    If the data set does not exist.
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

    /// Checks if the name is already taken by a group or data set.
    ///
    /// \param  inName  The name to check.
    /// \return         True if there is already a group or data set with the given name.
    ///
    bool isNameTaken(const std::string & inName) const;

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
