#ifndef ISX_SERIES_H
#define ISX_SERIES_H

#include "isxProjectItem.h"
#include "isxDataSet.h"
#include "isxCoreFwd.h"

#include <memory>

namespace isx
{

class TimingInfo;
class SpacingInfo;
class SeriesIdentifier;

    
/// Series contain any number of data sets that are non-overlapping and ordered by start time.
/// They are required to match in terms of type, frame rate (for movies), spacing info, etc.
///
/// UnitarySeries are a special type of Series. A UnitarySeries contains only a single data set.
/// UnitarySeries are used to represent stand-alone data sets in the Project and ProjectModel.
/// Regular Series (not unitary) can also contain a single data set if the use chose to create
/// it that way.
///
/// Series also span a tree with the original imported data at the root. A child C of a parent
/// Series P was created by applying an operation to P, C is a child of P and both C and P are
/// Series.
///
/// Series are ProjectItems, as such they are members of the Group hierarchy in the Project.
/// The Group that a Series belongs to is called its Container.
///
class Series : public ProjectItem, public std::enable_shared_from_this<Series>
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
    
    /// Create a series with a single DataSet.
    ///
    /// \param  inName      The name of the DataSet to create
    /// \param  inType      The type of the data set to create.
    /// \param  inFileName  The file name of the data set to create.
    /// \param  inHistory   The historical details for the dataset
    /// \param  inProperties The property map for the data set to create.
    /// \param  inImported  True if the data set is imported, false otherwise.
    Series(const std::string & inName,
        const DataSet::Type inType,
        const std::string & inFileName,
        const HistoricalDetails & inHistory,
        const DataSet::Properties & inProperties,
        const bool inImported = true);

    /// Create a series with a single DataSet.
    ///
    /// \param  inDataSet    The DataSet to put into this new Series object.
    ///
    Series(const SpDataSet_t & inDataSet);

    /// Dtor
    ///
    ~Series();

    /// \return the number of datasets components in the series
    ///
    isize_t
    getNumDataSets()
    const;

    /// \return All the data sets in this series.
    ///
    std::vector<DataSet *>
    getDataSets()
    const;

    /// \return A data sets in this series by its index.
    ///
    DataSet *
    getDataSet(isize_t inIndex)
    const;
    
    /// \return A unitary Series containing this Series DataSet at a given index
    /// \param inIndex Index for which to retrieve the unitary Series
    SpSeries_t
    getUnitarySeries(isize_t inIndex)
    const;

    /// \return The type of the DataSets in this Series.
    ///
    DataSet::Type
    getType()
    const;

    /// Insert a unitary Series (containing exactly one data set) into this series.
    /// This Series takes over ownership of the DataSet in the unitary Series.
    ///
    /// \param  inUnitarySeries   The Series to be inserted.
    ///
    /// \throw  ExceptionDataIO If this item already has a child with the name of the data set.
    /// \throw  ExceptionSeries If this Series is unitary or if inUnitarySeries is not unitary.
    void
    insertUnitarySeries(const SpSeries_t & inUnitarySeries);

    /// Remove a DataSet given a unitary Series.
    ///
    /// This transfers ownership of the unitary series from this to the client.
    ///
    /// \param  inUnitarySeries  The unitary Series to remove.
    /// \return                The removed unitary Series.
    ///
    /// \throw  ExceptionDataIO If the given Series cannot be found.
    SpSeries_t
    removeDataSet(const Series * inUnitarySeries);

    /// Remove a DataSet.
    ///
    /// This transfers ownership of the unitary series from this to the client.
    ///
    /// \param  inDataSet  The DataSet to remove.
    /// \return            The removed unitary Series.
    ///
    /// \throw  ExceptionDataIO If the given Series cannot be found.
    SpSeries_t
    removeDataSet(const DataSet * inDataSet);
    
    /// \return index of this Series in its owning Group.
    ///
    isize_t
    getMemberIndex() const;
    
    /// \return number of children.
    isize_t
    getNumChildren() const;
    
    /// Add a child to this Series.
    /// \return True if the child was added.
    bool
    addChild(SpSeries_t inSeries);

    /// Add a child to this series only if it passes a compatibility check.
    ///
    /// This is used when importing a series into the project tree and not
    /// in the root group.
    ///
    /// \param  inSeries        The series to add.
    /// \param  outErrorMessage The reason why the compatibility check failed.
    /// \return                 True if the compatibility check passed and the
    ///                         child was inserted, false otherwise.
    bool
    addChildWithCompatibilityCheck(SpSeries_t inSeries, std::string & outErrorMessage);

    /// \return True if this can be a parent for a derived series.
    ///
    bool
    isASuitableParent(std::string & outErrorMessage) const;

    /// Retrieve child by index.
    /// \return child Series at given index.
    /// \param inIndex index of child to retrieve.
    Series *
    getChild(isize_t inIndex) const;

    /// Retrieve child by name.
    /// \return child Series for given name.
    /// \param inName name of child to retrieve.
    Series *
    getChild(const std::string & inName) const;
    
    /// \return all children of this Series
    /// \param  inRecurse   If true, recursively get all children.
    std::vector<Series *>
    getChildren(const bool inRecurse = false) const;
    
    /// \return parent of this Series
    ///
    Series *
    getParent() const;
    
    /// Remove child by index.
    /// \return Series containing the child that was removed.
    /// \param inIndex index of the child to remove.
    SpSeries_t
    removeChild(isize_t inIndex);

    /// Get the history for this series
    /// \return string containing the history
    ///
    std::string
    getHistory() const;
    
    /// \return HistoricalDetails for this Series
    ///
    HistoricalDetails
    getHistoricalDetails() const;
    
    /// Checks that a data set can be added to this series.
    ///
    /// \param  inDataSet   The data set to check.
    /// \param  outMessage  The reason why the data set can't be added to the series.
    /// \return             True if the data set can be added to a series, false otherwise.
    bool
    checkDataSet(DataSet * inDataSet, std::string & outMessage);

    /// Checks that the data set type is compatible with this series.
    ///
    /// \param  inRef       The reference data set type.
    /// \param  inNew       The new data set type.
    /// \param  outMessage  The reason why the data set type is not compatible.
    /// \return             True if the new data set type is compatible for a series,
    ///                     false otherwise.
    static
    bool
    checkDataSetType(
        const DataSet::Type inRef,
        const DataSet::Type inNew,
        std::string & outMessage);

    /// Checks that the data type is consistent with that of a reference in a series.
    ///
    /// \param  inRef       The reference data type.
    /// \param  inNew       The new data type.
    /// \param  outMessage  The reason why the timing info is inconsistent.
    /// \return             True if the new data type is not consistent with
    ///                     the reference, false otherwise.
    static
    bool
    checkDataType(
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
    static
    bool
    checkTimingInfo(
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
    static
    bool
    checkSpacingInfo(
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
    static
    bool
    checkHistory(
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
    static SpSeries_t fromJsonString(const std::string & inString, const std::string & inAbsolutePathToPrepend = std::string());

    /// \return this Series' unique identifier
    /// Can be used with SeriesIdentifier::getSeries
    ///
    std::string
    getUniqueIdentifier()
    const;
    
    /// \return whether this is a "unitary" Series meaning it contains only a single
    ///         DataSet and it can't be modified (no additional DataSets can be inserted)
    bool
    isUnitary()
    const;
    
    /// \return weak reference to this Series
    ///
    WpSeries_t
    getWeakRef();

    /// Delete the files that this represents.
    ///
    void deleteFiles() const;

    /// \return     True if this series has been processed or is part of a
    ///             series that has been processed, false otherwise.
    ///             Note that this actually checks for children in this or
    ///             its containing series and assumes those children could
    ///             have only come from processing.
    bool isProcessed() const;

    /// \return     True if this is a member of a series that has been
    ///             processed, false otherwise.
    bool isAMemberOfASeries() const;

    std::string
    toJsonString(const bool inPretty = false, const std::string & inPathToOmit = std::string())
    const
    override;

    // Overrides: see isxProjectItem.h for docs.
    ProjectItem::Type
    getItemType()
    const
    override;

    bool
    isValid()
    const
    override;

    const
    std::string &
    getName()
    const
    override;

    void
    setName(const std::string & inName)
    override;

    void
    setContainer(ProjectItem * inContainer)
    override;

    ProjectItem *
    getContainer()
    const
    override;

    bool
    isModified()
    const
    override;
    
    void
    setUnmodified()
    override;
    
    bool
    operator ==(const ProjectItem & other)
    const
    override;
    
private:
    
    bool
    hasUnitarySeries(const Series * inUnitarySeries)
    const;
    
    void
    setParent(Series * inSeries);

    bool
    checkSeriesIsTemporallyContained(const SpSeries_t inSeries) const;

    bool
    checkSeriesHasSameNumPixels(const SpSeries_t inSeries) const;

    /// True if this series is valid.
    bool                                m_valid;

    /// True if this has unsaved changes.
    bool                                m_modified;
    
    /// The single DataSet in a "unitary" Series.  Unitary Series
    /// cannot be changed to non-unitary and vice versa.
    /// Unitary Series cannot be modified (can't insert additional DataSets).
    SpDataSet_t                         m_dataSet;

    /// The container that holds this series.
    ProjectItem *                       m_container;

    /// The name of this series.
    std::string                         m_name;

    /// The elements in this series. Each is a unitary series (contains exactly one DataSet).
    ///
    std::vector<SpSeries_t>             m_unitarySeries;

    /// The parent of this Series
    Series *                            m_parent = nullptr;

    /// Children of this Series
    std::vector<SpSeries_t>             m_children;

    std::unique_ptr<SeriesIdentifier>   m_identifier;
};

/// STL overload for print to a stream.
///
/// \param   inStream   The output stream to which to print.
/// \param   inSeries   The series to print.
/// \return             The modified stream.
std::ostream & operator<<(::std::ostream & inStream, const Series & inSeries);

} // namespace isx

#endif // ISX_SERIES_H
