#ifndef ISX_DATA_SET_H
#define ISX_DATA_SET_H

#include <string>
#include <memory>

namespace isx
{


// Need forward declare because declaration of DataSet needs knowledge
// of Group and declaration of Group needs knowledge of DataSet.
class Group;

/// The type of a shared pointer to a data set.
typedef std::shared_ptr<Group> SpGroup_t;

/// Encapsulates a data set within a project.
///
/// This is described by a data set type, a name and a file name.
class DataSet
{
public:

    /// The type of data item.
    ///
    /// TODO sweet : add image, trace and cell set when we have files
    /// for them.
    enum Type
    {
        MOVIE = 0,
    };

    /// Empty constructor.
    ///
    /// This creates a valid C++ object, but invalid DataSet.
    DataSet();

    /// Create a data set of with the given name, type and file name.
    ///
    /// \param  inName      The name of the data set.
    /// \param  inType      The type of the data set.
    /// \param  inFileName  The file name of the data set.
    DataSet(const std::string & inName,
            Type inType,
            const std::string & inFileName);

    /// \return     True if this is valid.
    ///
    bool isValid() const;

    /// \return     The type of this data set.
    ///
    Type getType() const;

    /// \return     The name of this data set.
    ///
    std::string getName() const;

    /// \return     The file name of this data set.
    ///
    std::string getFileName() const;

    /// \return     The parent of this group.
    ///
    SpGroup_t getParent() const;

    /// Set the parent of this data set.
    ///
    /// This simply updates the parent of this data set and does not move
    /// this data set into the given parent group,
    /// Use Group::removeDataSet and Group::addDataSet for that.
    ///
    /// \param  inParent    The new parent of this data set.
    void setParent(SpGroup_t & inParent);

    /// \return     The path of this group from the root group.
    ///
    std::string getPath() const;

    /// Exact comparison.
    ///
    /// \param  inOther     The data set with which to compare.
    /// \return             True if the data sets are exactly equal.
    bool operator==(const DataSet & inOther) const;

private:

    /// True if this data set is valid.
    bool m_valid;

    /// The name of this data set.
    std::string m_name;

    /// The type of this data set.
    Type m_type;

    /// The file name of this data set.
    std::string m_fileName;

    /// The parent group to which this belongs.
    SpGroup_t m_parent;

}; // class DataSet

/// The type of a shared pointer to a data set.
typedef std::shared_ptr<DataSet> SpDataSet_t;

} // namespace isx

#endif // ISX_DATA_SET_H
