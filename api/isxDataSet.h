#ifndef ISX_DATA_SET_H
#define ISX_DATA_SET_H

#include "isxObject.h"

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
/// This is described by a data set type, a name, a file name and dictionary
/// of various properties.
/// It may also have a parent Group which is its owner.
class DataSet : public Object
{
public:

    /// type of DataSet Properties
    using Properties = std::map<std::string, float>;

    /// The type of data set.
    ///
    /// TODO sweet : add image and trace when we have files for them.
    enum class Type
    {
        MOVIE = 0,
        CELLSET,
        BEHAVIOR
    };

    // Property names as used in the property map
    static const std::string PROP_DATA_MIN;     ///< Min range of data in a movie dataset
    static const std::string PROP_DATA_MAX;     ///< Max range of data in a movie dataset
    static const std::string PROP_VIS_MIN;      ///< Min visualization range [0..1]
    static const std::string PROP_VIS_MAX;      ///< Max visualization range [0..1]

    /// Empty constructor.
    ///
    /// This creates a valid C++ object, but invalid DataSet.
    DataSet();

    /// Create a data set of with the given name, type and file name.
    ///
    /// \param  inName      The name of the data set.
    /// \param  inType      The type of the data set.
    /// \param  inFileName  The file name of the data set.
    /// \param  inProperties The property map for the dataset
    DataSet(const std::string & inName,
            Type inType,
            const std::string & inFileName,
            const Properties & inProperties = Properties());

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

    /// Get the property map
    /// \return the property map
    const Properties & getProperties() const;

    /// Set the Properties of this DataSet
    /// \param inDataSetProperties new Properties to set
    void setProperties(const std::shared_ptr<Properties> & inDataSetProperties);

    /// Merge Properties into this DataSet's properties.
    /// Will only change those properties that are actually defined in inDataSetProperties.
    /// Will overwrite any existing Properties if they are defined in inDataSetProperties.
    /// \param inDataSetProperties Properties to merge
    void
    mergeProperties(const Properties & inDataSetProperties);

    /// Get the property value
    /// \return whether the property was found or not
    /// \param inPropertyName the name of the property
    /// \param outValue the found value
    bool getPropertyValue(const std::string & inPropertyName, float & outValue) const;

    /// Sets a property in the map
    /// \param inPropertyName name
    /// \param inValue value
    void setPropertyValue(const std::string & inPropertyName, float inValue);

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

    // Overrides
    void serialize(std::ostream & strm) const override;

    /// Serialize DataSet(s) to a JSON string
    /// \return JSON string for the DataSet(s)
    /// \param inOriginal original dataset 
    /// \param inDerived derived dataset (i.e. cellset coming from a movie)
    static
    std::string
    toJsonString(const DataSet * inOriginal, const DataSet * inDerived = nullptr);

    /// Create DataSet(s) from a JSON string
    /// \param inDataSetJson string containing JSON info for one or two DataSets
    /// \param outPath is set to the full path (in project) of the original DataSet
    /// \param outOriginal the original dataset described in the JSON string
    /// \param outDerived the derived dataset (if there is such in the JSON string)
    static
    void
    fromJsonString(const std::string & inDataSetJson, std::string & outPath, DataSet & outOriginal, DataSet & outDerived);

private:

    /// True if this data set is valid.
    bool m_valid;

    /// True if this has unsaved changes.
    bool m_modified;

    /// The name of this data set.
    std::string m_name;

    /// The type of this data set.
    Type m_type;

    /// The file name of this data set.
    std::string m_fileName;

    /// The parent group to which this belongs.
    Group * m_parent;

    /// Dataset properties
    Properties m_properties;

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

using SpDataSetProperties_t = std::shared_ptr<DataSet::Properties>;

} // namespace isx

#endif // ISX_DATA_SET_H
