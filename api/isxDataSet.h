#ifndef ISX_DATA_SET_H
#define ISX_DATA_SET_H

#include "isxCore.h"
#include "isxProjectItem.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxVariant.h"
#include "isxHistoricalDetails.h"

#include <string>
#include <map>

namespace isx
{

/// A data set represents a movie, cell set, etc. that is backed by one or many files.
///
class DataSet
{
public:

    /// type of DataSet Properties
    using Properties = std::map<std::string, Variant>;
    /// type of metadata properties
    using Metadata = std::vector<std::pair<std::string, std::string>>;

    /// The type of data set.
    ///
    /// TODO sweet : add image and trace when we have files for them.
    enum class Type
    {
        MOVIE = 0,
        CELLSET,
        BEHAVIOR,
        GPIO
    };

    // Property names as used in the property map
    static const std::string PROP_DATA_MIN;     ///< Min range of data in a movie dataset
    static const std::string PROP_DATA_MAX;     ///< Max range of data in a movie dataset
    static const std::string PROP_VIS_MIN;      ///< Min visualization range [0..1]
    static const std::string PROP_VIS_MAX;      ///< Max visualization range [0..1]
    static const std::string PROP_MOVIE_START_TIME;      ///< Movie start time - used for behavioral movies


    /// Empty constructor.
    ///
    /// This creates a valid C++ object, but invalid DataSet.
    DataSet();

    /// Create a data set of with the given name, type and file name.
    ///
    /// \param  inName      The name of the data set.
    /// \param  inType      The type of the data set.
    /// \param  inFileName  The file name of the data set.
    /// \param  inHistory   The historical details for the dataset
    /// \param  inProperties The property map for the dataset
    DataSet(const std::string & inName,
            Type inType,
            const std::string & inFileName,
            const HistoricalDetails & inHistory,
            const Properties & inProperties = Properties());

    /// \return     The type of this data set.
    ///
    Type getType() const;

    /// \return     The file name of this data set.
    ///
    std::string getFileName() const;

    /// Update the filename associated with this dataset
    /// \param inFileName the new filename
    void setFileName(const std::string & inFileName);

    /// Get the property map
    /// \return the property map
    const Properties & getProperties() const;
    
    /// Get the historical details for this dataset
    /// \return the historical details
    const HistoricalDetails & getHistoricalDetails() const;

    /// Set the Properties of this DataSet
    /// \param inDataSetProperties new Properties to set
    void setProperties(const std::shared_ptr<Properties> & inDataSetProperties);

    /// Merge Properties into this DataSet's properties.
    /// Will only change those properties that are actually defined in inDataSetProperties.
    /// Will overwrite any existing Properties if they are defined in inDataSetProperties.
    /// \param inDataSetProperties Properties to merge
    void mergeProperties(const Properties & inDataSetProperties);

    /// Get the property value
    /// \return whether the property was found or not
    /// \param inPropertyName the name of the property
    /// \param outValue the found value
    bool getPropertyValue(const std::string & inPropertyName, Variant & outValue) const;

    /// Sets a property in the map
    /// \param inPropertyName name
    /// \param inValue value
    void setPropertyValue(const std::string & inPropertyName, Variant inValue);

    /// Serialize vector of DataSets and derived DataSets to a JSON string
    /// \return JSON string for the DataSet(s)
    /// \param inPath project path for the dataset
    /// \param inDerivedPath project path for the derived dataset
    /// \param inTitle title to be shown for the dataset
    /// \param inDataSets vector of datasets
    /// \param inDerivedDataSets vector of derived datasets
    static
    std::string
    toJsonString(
        const std::string & inPath,
        const std::string & inDerivedPath,
        const std::string & inTitle,
        const std::vector<const DataSet *> & inDataSets,
        const std::vector<const DataSet *> & inDerivedDataSets);

    /// Create DataSet(s) from a JSON string
    /// \param inDataSetJson string containing JSON info for one or two DataSets
    /// \param outPath is set to the full path (in project) of the original DataSet
    /// \param outDerivedPath is set to the full path (in project) of the derived DataSet
    /// \param outTitle is set to the title to be shown for the dataset
    /// \param outOriginals the original dataset described in the JSON string
    /// \param outDeriveds the derived dataset (if there is such in the JSON string)
    static
    void
    fromJsonString(const std::string & inDataSetJson,
        std::string & outPath,
        std::string & outDerivedPath,
        std::string & outTitle,
        std::vector<DataSet> & outOriginals,
        std::vector<DataSet> & outDeriveds);

    /// Create a data set from a serialized JSON string.
    ///
    /// \param  inString    The serialized JSON string.
    /// \param  inAbsolutePathToPrepend The path the is preprended to any relative filenames in the dataset
    /// \return             The deserialized data set.
    ///
    /// \throw  ExceptionDataIO If the string cannot be parsed.
    static std::shared_ptr<DataSet> fromJsonString(const std::string & inString, const std::string & inAbsolutePathToPrepend = std::string());

    /// \return The timing info associated with this data set.
    ///
    /// \throw  ExceptionFileIO If the data set file cannot be read.
    /// \throw  ExceptionDataIO If the timing info cannot be parsed.
    const TimingInfo & getTimingInfo();

    /// \return The spacing info associated with this data set.
    ///
    /// \throw  ExceptionFileIO If the data set file cannot be read.
    /// \throw  ExceptionDataIO If the spacing info cannot be parsed.
    const SpacingInfo & getSpacingInfo();

    /// \return     The data type associated with this data set.
    ///
    /// \throw  ExceptionFileIO If the data set file cannot be read.
    /// \throw  ExceptionDataIO If the data type cannot be parsed.
    DataType getDataType();

    /// \return Validity of this DataSet
    ///
    bool isValid() const;

    /// \return Name of this DataSet
    ///
    std::string getName() const;

    /// Set the name of this DataSet
    /// \param inName new name to set for this DataSet
    ///
    void setName(const std::string & inName);

    /// /return Whether this DataSet has been modified.
    ///
    bool isModified() const;

    /// Indicate that this DataSet has been modified.
    ///
    void setUnmodified();

    /// \return JSON representation of this DataSet
    std::string toJsonString(const bool inPretty = false, const std::string & inPathToOmit = std::string()) const;

    /// \return Whether this DataSet is equal to another given DataSet
    /// \param other Other DataSet
    ///
    bool operator ==(const DataSet & other) const;
    
    /// Get history for this DataSet
    /// \return string containing history information
    std::string
    getHistory() const;

    /// \return all metadata for this dataset in the form of key-value pairs
    ///  
    Metadata getMetadata();

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

    /// The Series to which this DataSet belongs.
    Series * m_series;

    /// Historical details for the dataset
    HistoricalDetails m_history;

    /// Dataset properties
    Properties m_properties;

    /// Dataset properties read from the nVista file
    Properties m_readOnlyProperties;

    /// The timing info of this data set.
    TimingInfo m_timingInfo;

    /// The spacing info of this data set.
    SpacingInfo m_spacingInfo;

    /// The data type of this data set.
    DataType m_dataType;

    /// True if the meta data has been read.
    bool m_hasMetaData = false;

    /// Read the meta data from the data set file.
    ///
    /// \throw  ExceptionFileIO     If the read fails.
    /// \throw  ExceptionDataIO     If the file format is not recognized.
    void readMetaData();

}; // class DataSet

/// Get the Inscopix DataSet type of a file.
///
/// If the file format is not recognized, then this function fails.
///
/// \param  inFileName      The name of the movie file.
/// \return                 The data set type.
///
/// \throw  ExceptionFileIO     If the read fails.
/// \throw  ExceptionDataIO     If the file format is not recognized.
DataSet::Type readDataSetType(const std::string & inFileName);

using SpDataSetProperties_t = std::shared_ptr<DataSet::Properties>;

} // namespace isx

#endif // ISX_DATA_SET_H
