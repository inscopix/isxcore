#ifndef ISX_DATA_SET_H
#define ISX_DATA_SET_H

#include "isxObject.h"
#include "isxProjectItem.h"

#include <string>
#include <memory>
#include <map>

namespace isx
{

// Need forward declare because declaration of DataSet needs knowledge
// of Group and declaration of Group needs knowledge of DataSet.
class Group;

/// Encapsulates a data set within a project.
///
/// This is described by a data set type, a name and a file name.
class DataSet : public ProjectItem
              , public Object
{
public:

    /// The type of data set.
    ///
    /// TODO sweet : add image and trace when we have files for them.
    enum class Type
    {
        MOVIE = 0,
        CELLSET,
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
            const std::string & inFileName,
            const std::map<std::string, float> & inProperties);

    /// \return     The type of this data set.
    ///
    Type getType() const;

    /// \return     The file name of this data set.
    ///
    std::string getFileName() const;

    /// Exact comparison.
    ///
    /// \param  inOther     The data set with which to compare.
    /// \return             True if the data sets are exactly equal.
    bool operator==(const DataSet & inOther) const;

    // Overrides
    bool isValid() const override;

    std::string getName() const override;

    Group * getParent() const override;

    void setParent(Group * inParent) override;

    std::string getPath() const override;

    void serialize(std::ostream & strm) const override;

    std::map<std::string, float> & getProperties() const;

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
    Group * m_parent;

    /// Dataset properties
    std::map<std::string, float> m_properties;

}; // class DataSet

/// Get the Inscopix DataSet type of a file.
///
/// If the file format is not recognized, then this function fails.
///
/// \param  inFileName      The name of the movie file.
/// \return                 The data set type.
///
/// \throw  isx::ExceptionFileIO    If read the file fails.
/// \throw  isx::ExceptionDataIO    If the file format is not recognized.
DataSet::Type readDataSetType(const std::string & inFileName);

} // namespace isx

#endif // ISX_DATA_SET_H
