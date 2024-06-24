#ifndef ISX_SERIES_UTILS_H
#define ISX_SERIES_UTILS_H

#include "isxTimingInfo.h"
#include "isxDataSet.h"

namespace isx
{

class SpacingInfo;

/// Checks that the data set type is compatible with a reference.
///
/// \param  inRef       The reference data set type.
/// \param  inNew       The new data set type.
/// \param  outMessage  The reason why the data set type is not compatible.
/// \return             True if the new data set type is compatible for a series, false otherwise.
bool checkSeriesDataSetType(
    const DataSet::Type inRef,
    const DataSet::Type inNew,
    std::string & outMessage);

/// Checks that the data type is consistent with that of a reference in a series.
///
/// \param  inRef       The reference data type.
/// \param  inNew       The new data type.
/// \param  outMessage  The reason why the timing info is inconsistent.
/// \return             True if the new data type is not consistent with the reference, false otherwise.
bool checkSeriesDataType(
    const DataType inRef,
    const DataType inNew,
    std::string & outMessage);

/// Checks that the timing info is consistent with that of a reference in a series.
///
/// \param  inRef       The reference timing info.
/// \param  inNew       The new timing info.
/// \param  outMessage  The reason why the timing info is inconsistent.
/// \return             True if the new timing info is consistent with the reference, false otherwise.
bool checkSeriesTimingInfo(
    const TimingInfo & inRef,
    const TimingInfo & inNew,
    std::string & outMessage);

/// Checks that the spacing info is consistent with that of a reference in a series.
///
/// \param  inRef       The reference spacing info.
/// \param  inNew       The new spacing info.
/// \param  outMessage  The reason why the timing info is inconsistent.
/// \return             True if the new spacing info is consistent with the reference, false otherwise.
bool checkSeriesSpacingInfo(
    const SpacingInfo & inRef,
    const SpacingInfo & inNew,
    std::string & outMessage);

/// Checks that the history details are consistent with those of a reference in a series.
///
/// \param  inRef       The reference history.
/// \param  inNew       The new history.
/// \param  outMessage  The error message.
/// \return             True if the new history is consistent with the reference, false otherwise.
bool checkSeriesHistory(
    const HistoricalDetails & inRef,
    const HistoricalDetails & inNew,
    std::string & outMessage);

/// \param  inExisting  The existing movies in the series sorted by time.
/// \param  inNew       The new movie to add to the series.
/// \param  outMessage  The reason why the new movie cannot be added to the series.
/// \return             True if the new movie can be added to the series, false otherwise.
bool checkNewMemberOfSeries(
        const std::vector<SpMovie_t> & inExisting,
        const SpMovie_t & inNew,
        std::string & outMessage);

/// \param  inExisting  The existing cellsets in the series sorted by time.
/// \param  inNew       The new cellset to add to the series.
/// \param  outMessage  The reason why the new cellset cannot be added to the series.
/// \return             True if the new cellset can be added to the series, false otherwise.
bool checkNewMemberOfSeries(
        const std::vector<SpCellSet_t> & inExisting,
        const SpCellSet_t & inNew,
        std::string & outMessage);

/// \param  inExisting  The existing vesselsets in the series sorted by time.
/// \param  inNew       The new vesselset to add to the series.
/// \param  outMessage  The reason why the new vesselset cannot be added to the series.
/// \return             True if the new vesselset can be added to the series, false otherwise.
bool checkNewMemberOfSeries(
    const std::vector<SpVesselSet_t> & inExisting,
    const SpVesselSet_t & inNew,
    std::string & outMessage);

/// \param  inExisting  The existing gpios in the series sorted by time.
/// \param  inNew       The new gpio to add to the series.
/// \param  outMessage  The reason why the new gpio cannot be added to the series.
/// \return             True if the new gpio can be added to the series, false otherwise.
bool checkNewMemberOfSeries(
        const std::vector<SpGpio_t> & inExisting,
        const SpGpio_t & inNew,
        std::string & outMessage);

/// \param  inExisting  The existing events in the series sorted by time.
/// \param  inNew       The new events to add to the series.
/// \param  outMessage  The reason why the new events cannot be added to the series.
/// \return             True if the new events can be added to the series, false otherwise.
bool checkNewMemberOfSeries(
        const std::vector<SpEvents_t> & inExisting,
        const SpEvents_t & inNew,
        std::string & outMessage);

/// \param  inMembers   The potential series members to check.
/// \param  outMessage  The reason why the members do not form a series.
/// \return             True if the members form a series, false otherwise.
template <typename T>
bool
checkSeriesMembers(const std::vector<std::shared_ptr<T>> & inMembers, std::string & outMessage)
{
    std::vector<std::shared_ptr<T>> checked;
    for (const auto & m : inMembers)
    {
        if (!checkNewMemberOfSeries(checked, m, outMessage))
        {
            return false;
        }
        checked.push_back(m);
    }
    return true;
}

/// \return The timing info for series without gaps from many consistent
///         timing infos.
TimingInfo makeGaplessTimingInfo(const TimingInfos_t & inTis);

/// \param  inType      The type of series (e.g. CellSetSeries).
/// \param  inFilePaths The paths of the files in the series.
/// \return             A string that represents the file names of a series.
std::string makeSeriesFilePathString(const std::string & inType, const std::vector<std::string> & inFilePaths);

} // namespace isx

#endif // ISX_SERIES_UTILS_H
