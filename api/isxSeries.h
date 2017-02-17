#ifndef ISX_SERIES_H
#define ISX_SERIES_H

#include "isxProjectItem.h"
#include "isxDataSet.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"

namespace isx
{

class TimingInfo;
class SpacingInfo;

/// A series of data sets that are non-overlapping and ordered by start time.
///
/// This is like a meta-data set that efficiently wraps around existing ones.
class Series : public ProjectItem
{
public:

    /// Empty constructor.
    ///
    /// Creates a valid C++ object, but not a valid Series.
    Series();

    /// Create a series with the given name.
    ///
    /// \param  inName      The name of this series.
    Series(const std::string & inName);

    /// \return the number of datasets components in the series
    ///
    isize_t getNumDataSets() const;

    /// \return All the data sets in this series.
    ///
    std::vector<DataSet *> getDataSets() const;

    /// Insert a data set into this series.
    ///
    /// The client must give ownership of DataSet to this method using std::move.
    ///
    /// \param  inDataSet   The data set to insert.
    ///
    /// \throw  ExceptionDataIO If this item already has a child with the name of the data set.
    void insertDataSet(std::shared_ptr<DataSet> & inDataSet);

    /// Remove a data set by name.
    ///
    /// This transfers ownership of the data set from this to the client.
    ///
    /// \param  inName  The name of the data set to remove.
    /// \return         The removed data set.
    ///
    /// \throw  ExceptionDataIO If a data set with the given name cannot be found.
    std::shared_ptr<DataSet> removeDataSet(const std::string & inName);

    /// \return a vector containing all historical series
    ///
    std::vector<std::shared_ptr<Series>> getHistoricalSeries();

    /// Get the history for this series
    /// \return the historical details
    const HistoricalDetails getHistory() const;

    /// Set the previous series that gave origin to this one
    /// \param inSeries the previous series
    void setPrevious(const std::shared_ptr<Series> & inSeries);

    /// Checks that a data set can be added to this series.
    ///
    /// \param  inDataSet   The data set to check.
    /// \param  outMessage  The reason why the data set can't be added to the series.
    /// \return             True if the data set can be added to a series, false otherwise.
    bool checkDataSet(DataSet * inDataSet, std::string & outMessage);

    /// Checks that the data set type is valid for a series.
    ///
    /// \param  inNew       The data set type.
    /// \param  outMessage  The reason why the data set type is not valid.
    /// \return             True if the new data set type is valid for a series,
    ///                     false otherwise.
    static bool checkDataSetType(
            const DataSet::Type inNew,
            std::string & outMessage);

    /// Checks that the data type is consistent with that of a reference in a series.
    ///
    /// \param  inRef       The reference data type.
    /// \param  inNew       The new data type.
    /// \param  outMessage  The reason why the timing info is inconsistent.
    /// \return             True if the new data type is not consistent with
    ///                     the reference, false otherwise.
    static bool checkDataType(
            const DataType inRef,
            const DataType inNew,
            std::string & outMessage);

    /// Checks that the timing info is consistent with that of a reference in a series.
    ///
    /// \param  inRef       The reference timing info.
    /// \param  inNew       The new timing info.
    /// \param  outMessage  The reason why the timing info is inconsistent.
    /// \return             True if the new timing info is consistent with
    ///                     the reference, false otherwise.
    static bool checkTimingInfo(
            const TimingInfo & inRef,
            const TimingInfo & inNew,
            std::string & outMessage);

    /// Checks that the spacing info is consistent with that of a reference in a series.
    ///
    /// \param  inRef       The reference spacing info.
    /// \param  inNew       The new spacing info.
    /// \param  outMessage  The reason why the timing info is inconsistent.
    /// \return             True if the new spacing info is consistent with
    ///                     the reference, false otherwise.
    static bool checkSpacingInfo(
            const SpacingInfo & inRef,
            const SpacingInfo & inNew,
            std::string & outMessage);

    /// Checks that the history details are consistent with those of a reference in a series.
    ///
    /// \param  inRef       The reference history.
    /// \param  inNew       The new history.
    /// \param  outMessage  The error message.
    /// \return             True if the new history is consistent with
    ///                     the reference, false otherwise.
    static bool checkHistory(
            const HistoricalDetails & inRef,
            const HistoricalDetails & inNew,
            std::string & outMessage);

    /// Create a series from a serialized JSON string.
    ///
    /// \param  inString    The serialized JSON string.
    /// \param  inAbsolutePathToPrepend The path the is preprended to any relative filenames in the series
    /// \return             The deserialized series.
    ///
    /// \throw  ExceptionDataIO If the string cannot be parsed.
    static std::shared_ptr<Series> fromJsonString(const std::string & inString, const std::string & inAbsolutePathToPrepend = std::string());     

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

    /// True if this series is valid.
    bool m_valid;

    /// True if this has unsaved changes.
    bool m_modified;

    /// True if this is an historical series
    bool m_historical = false;

    /// The parent of this series.
    ProjectItem * m_parent;

    /// The name of this series.
    std::string m_name;

    /// The data sets in this series.
    std::vector<std::shared_ptr<DataSet>> m_dataSets;

    /// The historical series
    std::shared_ptr<Series>  m_previous;

    /// Set this dataset as historical
    void setHistorical();

};

} // namespace isx

#endif // ISX_SERIES_H
