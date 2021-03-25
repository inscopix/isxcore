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

    /// type of callback used to set the series containing this data set
    /// as modified
    using ModifiedCB_t = std::function<void()>;

    /// The type of data set.
    ///
    enum class Type
    {
        MOVIE = 0,
        CELLSET,
        BEHAVIOR,
        GPIO, 
		IMAGE,
        EVENTS,
        METRICS,
        IMU,
        VESSELSET
    };

    // Property names as used in the property map
    static const std::string PROP_DATA_MIN;     ///< Min range of data in a movie dataset
    static const std::string PROP_DATA_MAX;     ///< Max range of data in a movie dataset
    static const std::string PROP_MOVIE_START_TIME;      ///< Movie start time - used for behavioral movies
    static const std::string PROP_BEHAV_GOP_SIZE;        ///< Behavioral movie GOP size
    static const std::string PROP_BEHAV_NUM_FRAMES;      ///< Behavioral movie num frames
    static const std::string PROP_MOVIE_FRAME_RATE;      ///< Movie frame rate - used only for TIF files with no XML

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
    /// \param  inImported  True if the data set is imported, false otherwise.
    /// \param  inCallback  Callback to call when this data set is modified.
    DataSet(const std::string & inName,
            Type inType,
            const std::string & inFileName,
            const HistoricalDetails & inHistory,
            const Properties & inProperties = Properties(),
            const bool inImported = true, 
            ModifiedCB_t inCallback = nullptr);

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

    /// Set the extra properties of this DataSet
    /// \param  inProperties    The extra properties formatted as a JSON string.
    void setExtraProperties(const std::string & inProperties);

    /// Set the integrated Base plate meta data
    /// \param  inIntegratedBasePlate    The extra properties formatted as a JSON string.
    void setIntegratedBasePlate(const std::string & inIntegratedBasePlate);

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

    /// Sets the containing series modified flag to true
    //
    void setModified();

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

    /// \return the extra properties meta data
    ///
    std::string getExtraProperties();

    /// \return     True if this is imported, false otherwise.
    ///
    bool isImported() const;

    /// Delete the file that this represents if this is not imported.
    ///
    void deleteFile() const;

    /// \return     True if this dataset's file can be found on the filesystem.
    ///
    bool fileExists() const;

    /// Tries to locate this dataset's file in the given directory and updates
    /// the file name accordingly.
    ///
    /// \param  inDirectory The path of the directory in which to look.
    /// \return             True if the file was located.
    bool locateFile(const std::string & inDirectory);
    /// \return a string describing the data set type
    /// \param inType the data set type to convert to string
    static std::string getTypeString(Type inType);

    /// Set the Set Modified callback
    /// \param inCallback the callback function
    void setModifiedCallback(ModifiedCB_t inCallback);

private:

    /// True if this data set is valid.
    bool m_valid;

    /// The name of this data set.
    std::string m_name;

    /// The type of this data set.
    Type m_type;

    /// The file name of this data set.
    std::string m_fileName;

    /// Historical details for the dataset
    HistoricalDetails m_history;

    /// Dataset properties
    Properties m_properties;

    /// Dataset properties read from the nVista file
    Properties m_readOnlyProperties;

    /// The timing info of this data set.
    TimingInfo m_timingInfo;

    /// Timing info of secondary data in this data set.
    TimingInfo m_secondaryTimingInfo;

    /// The spacing info of this data set.
    SpacingInfo m_spacingInfo;

    /// The data type of this data set.
    DataType m_dataType = DataType::U16;

    /// True if the meta data has been read.
    bool m_hasMetaData = false;

    /// True if this was imported, false otherwise.
    /// Imported data sets should not delete the files they own.
    bool m_imported = true;

    /// Callback to set the modified flag of the containig series to true 
    ModifiedCB_t m_modifiedCB = nullptr;

    /// The extra properties as a JSON string, which should be updated by readMetaData.
    std::string m_extraProps;

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
/// \param  inProperties    The properties for for the movie file.
/// \return                 The data set type.
///
/// \throw  ExceptionFileIO     If the read fails.
/// \throw  ExceptionDataIO     If the file format is not recognized.
DataSet::Type readDataSetType(const std::string & inFileName, const DataSet::Properties & inProperties);

using SpDataSetProperties_t = std::shared_ptr<DataSet::Properties>;

/// Get acquisition information from extra properties.
///
/// \param  inExtraPropsStr The extra properties JSON string.
/// \return                 The acquisition info JSON string.
std::string
getAcquisitionInfoFromExtraProps(const std::string & inExtraPropsStr);

} // namespace isx

#endif // ISX_DATA_SET_H
