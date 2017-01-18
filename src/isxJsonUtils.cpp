#include "isxJsonUtils.h"
#include "isxException.h"

#include <fstream>

namespace isx
{

json
convertRatioToJson(const Ratio & inRatio)
{
    json j;
    j["type"] = "Ratio";
    j["num"] = inRatio.getNum();
    j["den"] = inRatio.getDen();
    return j;
}

Ratio
convertJsonToRatio(const json & j)
{
    int64_t num = j["num"];
    int64_t den = j["den"];
    return Ratio(num, den);
}

json
convertTimeToJson(const Time & inTime)
{
    json j;
    j["type"] = "Time";
    j["secsSinceEpoch"] = convertRatioToJson(inTime.getSecsSinceEpoch());
    j["utcOffset"] = inTime.getUtcOffset();
    return j;
}

Time
convertJsonToTime(const json & j)
{
    DurationInSeconds secsSinceEpoch = convertJsonToRatio(j["secsSinceEpoch"]);
    int32_t utcOffset = j["utcOffset"];
    return Time(secsSinceEpoch, utcOffset);
}

json
convertTimingInfoToJson(const TimingInfo & inTimingInfo)
{
    json j;
    j["type"] = "TimingInfo";
    j["numTimes"] = inTimingInfo.getNumTimes();
    j["period"] = convertRatioToJson(inTimingInfo.getStep());
    j["start"] = convertTimeToJson(inTimingInfo.getStart());
    std::vector<isize_t> droppedFrames = inTimingInfo.getDroppedFrames();
    j["dropped"] = droppedFrames;
    return j;
}

TimingInfo
convertJsonToTimingInfo(const json & j)
{
    isize_t numTimes = j["numTimes"];
    DurationInSeconds step = convertJsonToRatio(j["period"]);
    Time start = convertJsonToTime(j["start"]);
    isize_t dropped_present = j.count("dropped");
    std::vector<isize_t> droppedFrames;
    if (dropped_present)
    {
        droppedFrames = j["dropped"].get<std::vector<isize_t>>();
    }    
    return TimingInfo(start, step, numTimes, droppedFrames);
}

json
convertSizeInPixelsToJson(const SizeInPixels_t & inSizeInPixels)
{
    json j;
    j["type"] = "SizeInPixels";
    j["x"] = inSizeInPixels.getX();
    j["y"] = inSizeInPixels.getY();
    return j;
}

SizeInPixels_t
convertJsonToSizeInPixels(const json & j)
{
    isize_t x = j["x"];
    isize_t y = j["y"];
    return SizeInPixels_t(x, y);
}

SizeInMicrons_t
convertJsonToSizeInMicrons(const json & j)
{
    Ratio x = convertJsonToRatio(j["x"]);
    Ratio y = convertJsonToRatio(j["y"]);
    return SizeInMicrons_t(x, y);
}

PointInMicrons_t
convertJsonToPointInMicrons(const json & j)
{
    Ratio x = convertJsonToRatio(j["x"]);
    Ratio y = convertJsonToRatio(j["y"]);
    return PointInMicrons_t(x, y);
}

json
convertSizeInMicronsToJson(const SizeInMicrons_t & inSizeInMicrons)
{
    json j;
    j["type"] = "SizeInMicrons";
    j["x"] = convertRatioToJson(inSizeInMicrons.getX());
    j["y"] = convertRatioToJson(inSizeInMicrons.getY());
    return j;
}

json
convertPointInMicronsToJson(const PointInMicrons_t & inPointInMicrons)
{
    json j;
    j["type"] = "PointInMicrons";
    j["x"] = convertRatioToJson(inPointInMicrons.getX());
    j["y"] = convertRatioToJson(inPointInMicrons.getY());
    return j;
}

json
convertSpacingInfoToJson(const SpacingInfo & inSpacingInfo)
{
    json j;
    j["type"] = "SpacingInfo";
    j["numPixels"] = convertSizeInPixelsToJson(inSpacingInfo.getNumPixels());
    j["pixelSize"] = convertSizeInMicronsToJson(inSpacingInfo.getPixelSize());
    j["topLeft"] = convertPointInMicronsToJson(inSpacingInfo.getTopLeft());
    return j;
}

SpacingInfo
convertJsonToSpacingInfo(const json & j)
{
    SizeInPixels_t numPixels = convertJsonToSizeInPixels(j["numPixels"]);
    SizeInMicrons_t pixelSize = convertJsonToSizeInMicrons(j["pixelSize"]);
    PointInMicrons_t topLeft = convertJsonToPointInMicrons(j["topLeft"]);
    return SpacingInfo(numPixels, pixelSize, topLeft);
}

json 
convertHistoryToJson(const HistoricalDetails & inHistory)
{
    json outJson;
    outJson["operation"] = inHistory.getOperation();
    outJson["inputParameters"] = json::parse(inHistory.getInputParameters());
    return outJson;
}

HistoricalDetails 
convertJsonToHistory(const json & j)
{
    std::string operationName = j["operation"];
    std::string inputParams = j["inputParameters"].dump(4);
    HistoricalDetails outHistory(operationName, inputParams);
    return outHistory;
}

json 
convertPropertiesToJson(const DataSet::Properties & inProperties)
{
    json outJson = json::object();
    for (auto & p : inProperties)
    {
        json o = json::parse(p.second.toString());
        outJson[p.first] = o;
    }
    return outJson;
}

DataSet::Properties 
convertJsonToProperties(const json & j)
{
    DataSet::Properties properties;
    for (json::const_iterator it = j.begin(); it != j.end(); ++it) 
    {
        Variant v;
        if (it.key() != DataSet::PROP_MOVIE_START_TIME)
        {
            v = Variant(it.value().get<float>());
        }
        else 
        {
            v = Variant(convertJsonToTime(it.value()));
        }
        properties[it.key()] = v;
    }
    return properties;
}

json
readJsonHeader(std::istream & inStream, bool inNullTerm)
{
    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error while reading JSON header.");
    }

    inStream.seekg(std::ios_base::beg);
    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error while seeking to JSON header.");
    }

    json jsonObject;
    try
    {
        if (inNullTerm)
        {
            std::string jsonStr;
            std::getline(inStream, jsonStr, '\0');
            jsonObject = json::parse(jsonStr);
        }
        else
        {
            inStream >> jsonObject;
        }
    }
    catch (const std::exception & error)
    {
        ISX_THROW(isx::ExceptionDataIO, "Error while parsing JSON header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing JSON header.");
    }

    return jsonObject;
}

void
writeJsonHeader(
        const json & inJsonObject,
        std::ostream & inStream,
        bool inNullTerm)
{
    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open stream when writing JSON header.");
    }

    inStream.seekp(std::ios_base::beg);
    inStream << std::setw(4) << inJsonObject;
    if (inNullTerm)
    {
        inStream << '\0';
    }

    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Failed to write JSON header.");
    }
}

} // namespace isx
