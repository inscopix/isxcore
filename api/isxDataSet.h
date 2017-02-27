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
class DataSet : public ProjectItem
{
public:

    /// type of DataSet Properties
    using Properties = std::map<std::string, Variant>;

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
    
    /// Get the history for this dataset
    /// \return the historical details
    const HistoricalDetails & getHistory() const;

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

    /// \return the number of derived datasets 
    ///
    isize_t getNumDerivedDataSets() const;

    /// \return     All the derived data sets of this.
    ///
    std::vector<DataSet *> getDerivedDataSets() const;

    /// Insert a derived data set into this.
    ///
    /// \param  inDataSet   The data set to add.
    /// \throw  ExceptionDataIO If this item already has a child with the name of the data set.
    void insertDerivedDataSet(std::shared_ptr<DataSet> & inDataSet);

    /// Remove a derived data set by name.
    ///
    /// \param  inName  The name of the data set to remove.
    /// \return         The removed data set.
    /// \throw  ExceptionDataIO If a data set with the given name cannot be found.
    std::shared_ptr<DataSet> removeDerivedDataSet(const std::string & inName);

    /// Set the previous dataset that gave origin to this one
    /// \param inDataSet the previous dataset
    void setPrevious(const std::shared_ptr<DataSet> & inDataSet);

    /// Serialize vector of DataSets and derived DataSets to a JSON string
    /// \return JSON string for the DataSet(s)
    /// \param inPath project path for the dataset
    /// \param inDataSets vector of datasets
    /// \param inDerivedDataSets vector of derived datasets
    static
    std::string
    toJsonString(
        const std::string & inPath,
        const std::vector<const DataSet *> & inDataSets,
        const std::vector<const DataSet *> & inDerivedDataSets);

    /// Create DataSet(s) from a JSON string
    /// \param inDataSetJson string containing JSON info for one or two DataSets
    /// \param outPath is set to the full path (in project) of the original DataSet
    /// \param outOriginals the original dataset described in the JSON string
    /// \param outDeriveds the derived dataset (if there is such in the JSON string)
    static
    void
    fromJsonString(const std::string & inDataSetJson,
        std::string & outPath,
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

    /// \return whether this data set is contained in a series or not
    ///
    bool isPartOfSeries() const;          

    /// \return a vector containing all historical data sets
    ///
    std::vector<std::shared_ptr<DataSet>> getHistoricalDataSets() const;

    // Overrides: see isxProjectItem.h for docs.
    ProjectItem::Type getItemType() const override;

    bool isValid() const override;

    std::string getName() const override;

    void setName(const std::string & inName) override;

    ProjectItem * getMostRecent() const override;
    
    ProjectItem * getPrevious() const override;

    ProjectItem * getParent() const override;

    void setParent(ProjectItem * inParent) override;

    std::vector<ProjectItem *> getChildren() const override;

    size_t getNumChildren() const override;

    void insertChild(std::shared_ptr<ProjectItem> inItem, const int inIndex = -1) override;

    std::shared_ptr<ProjectItem> removeChild(const std::string & inName) override;

    bool isModified() const override;

    void setUnmodified() override;

    std::string toJsonString(const bool inPretty = false, const std::string & inPathToOmit = std::string()) const override;

    bool operator ==(const ProjectItem & other) const override;

    bool hasHistory() const override;

    isize_t getNumHistoricalItems() const override;

    bool isHistorical() const override;

    std::string getHistoricalDetails() const override;

private:

    /// True if this data set is valid.
    bool m_valid;

    /// True if this has unsaved changes.
    bool m_modified;

    /// True if this is an historical data set;
    bool m_historical = false; 

    /// The name of this data set.
    std::string m_name;

    /// The type of this data set.
    Type m_type;

    /// The file name of this data set.
    std::string m_fileName;

    /// The derived data sets.
    std::vector<std::shared_ptr<DataSet>> m_derived;

    /// The parent item to which this belongs.
    ProjectItem * m_parent;

    /// The previous dataset that was used to create this one. 
    std::shared_ptr<DataSet> m_previous;

    /// Historical details for the dataset
    HistoricalDetails m_history;

    /// Dataset properties
    Properties m_properties;

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

    /// Set this dataset as historical
    void setHistorical();  

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
