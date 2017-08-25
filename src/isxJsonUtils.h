#ifndef ISX_JSON_UTILS_H
#define ISX_JSON_UTILS_H

#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxGroup.h"
#include "isxDataSet.h"
#include "isxCellSet.h"

#include "json.hpp"

namespace isx
{

// for convenience
using json = nlohmann::json;

using CellNames_t = std::vector<std::string>;
using CellStatuses_t = std::vector<CellSet::CellStatus>;
using CellActivities_t = std::vector<bool>;

json convertRatioToJson(const Ratio & inRatio);
Ratio convertJsonToRatio(const json & j);

json convertTimeToJson(const Time & inTime);
Time convertJsonToTime(const json & j);

json convertIndexRangesToJson(const IndexRanges_t & inRanges);
IndexRanges_t convertJsonToIndexRanges(const json & j);

json convertTimingInfoToJson(const TimingInfo & inTimingInfo);
TimingInfo convertJsonToTimingInfo(const json & j);

json convertSizeInPixelsToJson(const SizeInPixels_t & inSizeInPixels);
SizeInPixels_t convertJsonToSizeInPixels(const json & j);

SizeInMicrons_t convertJsonToSizeInMicrons(const json & j);
json convertSizeInMicronsToJson(const SizeInMicrons_t & inSizeInMicrons);

PointInMicrons_t convertJsonToPointInMicrons(const json & j);
json convertPointInMicronsToJson(const PointInMicrons_t & inPointInMicrons);

json convertSpacingInfoToJson(const SpacingInfo & inSpacingInfo);
SpacingInfo convertJsonToSpacingInfo(const json & j);

json convertHistoryToJson(const HistoricalDetails & inHistory);
HistoricalDetails convertJsonToHistory(const json & j);

json convertPropertiesToJson(const DataSet::Properties & inProperties);
DataSet::Properties convertJsonToProperties(const json & j);

json getProducerAsJson();

/// Reads a JSON header from an input stream.
///
/// This reads from the current position of the stream.
///
/// \param  inStream        The input stream from which to read.
/// \return                 The JSON structure.
///
/// \throw  isx::ExceptionFileIO    If reading the stream fails.
/// \throw  isx::ExceptionDataIO    If parsing the JSON header fails.
json
readJson(std::istream & inStream);

/// Writes a JSON header to an output stream.
///
/// This writes from the beginning of the stream.
///
/// \param  inJsonObject    The JSON structure.
/// \param  inStream        The output stream to which to write.
///
/// \throw  isx::ExceptionFileIO    If writing the header fails.
void
writeJson(
    const json & inJsonObject,
    std::ostream & inStream);

json
convertCellNamesToJson(const CellNames_t & inCellNames);
CellNames_t
convertJsonToCellNames(const json & inJson);
json
convertCellStatusesToJson(const CellStatuses_t & inCellStatuses);
CellStatuses_t
convertJsonToCellStatuses(const json & inJson);
json
convertCellActivitiesToJson(const CellActivities_t & inCellActivities);
CellActivities_t
convertJsonToCellActivities(const json & inJson);
    
/// Reads a JSON header from an input stream.
///
json
readJsonHeaderAtEnd(std::istream & inStream, std::ios::pos_type & outHeaderPosition);

/// Writes a JSON header to an output stream.
///
void
writeJsonHeaderAtEnd(
    const json & inJsonObject,
    std::ostream & inStream);

} // namespace isx

#endif // ISX_JSON_UTILS_H
