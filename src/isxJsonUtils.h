#ifndef ISX_JSON_UTILS_H
#define ISX_JSON_UTILS_H

#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
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

}

#endif // ISX_JSON_UTILS_H
