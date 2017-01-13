#ifndef ISX_JSON_UTILS_H
#define ISX_JSON_UTILS_H

#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxGroup.h"
#include "isxDataSet.h"

#include "json.hpp"

namespace isx
{

// for convenience
using json = nlohmann::json;

json convertRatioToJson(const Ratio & inRatio);
Ratio convertJsonToRatio(const json & j);

json convertTimeToJson(const Time & inTime);
Time convertJsonToTime(const json & j);

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

json convertPropertiesToJson(const DataSet::Properties & inProperties);
DataSet::Properties convertJsonToProperties(const json & j);

/// Reads a JSON header from an input stream.
///
/// This reads from the beginning of the stream.
///
/// \param  inStream    The input stream from which to read.
/// \param  inNullTerm  If true, read until a null terminator.
/// \return             The JSON structure.
///
/// \throw  isx::ExceptionFileIO    If reading the stream fails.
/// \throw  isx::ExceptionDataIO    If parsing the JSON header fails.
json
readJsonHeader(std::istream & inStream, bool inNullTerm = true);

/// Writes a JSON header to an output stream.
///
/// This writes from the beginning of the stream.
///
/// \param  inJsonObject    The JSON structure.
/// \param  inStream        The output stream to which to write.
/// \param  inNullTerm      If true, write a null terminator at the end
///                         of the header.
///
/// \throw  isx::ExceptionFileIO    If writing the header fails.
void
writeJsonHeader(
        const json & inJsonObject,
        std::ostream & inStream,
        bool inNullTerm = true);

} // namespace isx

#endif // ISX_JSON_UTILS_H
