#include "isxJsonUtils.h"
#include "isxException.h"

#include <iostream>
#include <fstream>
#include <sstream>

// Note aschildan Feb 9, 2017
// Disable operator[] which is unchecked and results in undefined behavior if
// the provided key is not found.  Disabling this forces client code to use the
// at(key) method which throws an exception if the key is not found.
// This makes it easier to deal with unexpected / incompatible JSON.
// See MOS-547 (opening old project files)
// When merging a new version of this json library, please copy this explanation
// and the code related to disabling operator[] to the new version.
// See setting of this #define in json.hpp
#ifndef ISX_JSON_NO_BRACKET_OPERATOR
#error Please disable basic::json operator[]
#endif

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
    int64_t num = j.at("num");
    int64_t den = j.at("den");
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
    DurationInSeconds secsSinceEpoch = convertJsonToRatio(j.at("secsSinceEpoch"));
    int32_t utcOffset = j.at("utcOffset");
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
    isize_t numTimes = j.at("numTimes");
    DurationInSeconds step = convertJsonToRatio(j.at("period"));
    Time start = convertJsonToTime(j.at("start"));
    isize_t dropped_present = j.count("dropped");
    std::vector<isize_t> droppedFrames;
    if (dropped_present)
    {
        droppedFrames = j.at("dropped").get<std::vector<isize_t>>();
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
    isize_t x = j.at("x");
    isize_t y = j.at("y");
    return SizeInPixels_t(x, y);
}

SizeInMicrons_t
convertJsonToSizeInMicrons(const json & j)
{
    Ratio x = convertJsonToRatio(j.at("x"));
    Ratio y = convertJsonToRatio(j.at("y"));
    return SizeInMicrons_t(x, y);
}

PointInMicrons_t
convertJsonToPointInMicrons(const json & j)
{
    Ratio x = convertJsonToRatio(j.at("x"));
    Ratio y = convertJsonToRatio(j.at("y"));
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
    SizeInPixels_t numPixels = convertJsonToSizeInPixels(j.at("numPixels"));
    SizeInMicrons_t pixelSize = convertJsonToSizeInMicrons(j.at("pixelSize"));
    PointInMicrons_t topLeft = convertJsonToPointInMicrons(j.at("topLeft"));
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
    std::string operationName = j.at("operation");
    std::string inputParams = j.at("inputParameters").dump(4);
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
readJson(std::istream & inStream)
{
    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error while reading JSON.");
    }

    inStream.seekg(std::ios_base::beg);
    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error while seeking to JSON.");
    }

    json jsonObject;
    try
    {
        inStream >> jsonObject;
    }
    catch (const std::exception & error)
    {
        ISX_THROW(isx::ExceptionDataIO, "Error while parsing JSON: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing JSON.");
    }

    return jsonObject;
}
    
void
writeJson(
    const json & inJsonObject,
    std::ostream & inStream)
{
    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open stream when writing JSON.");
    }

    inStream.seekp(std::ios_base::beg);
    inStream << std::setw(4) << inJsonObject;

    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Failed to write JSON.");
    }
}
    
json
convertCellNamesToJson(const CellNames_t & inCellNames)
{
    return inCellNames;
}

CellNames_t
convertJsonToCellNames(const json & inJson)
{
    return inJson.get<CellNames_t>();
}

json
convertCellValiditiesToJson(const CellValidities_t & inCellValidities)
{
    return inCellValidities;
}

CellValidities_t
convertJsonToCellValidities(const json & inJson)
{
    return inJson.get<CellValidities_t>();
}
    
json
readJsonHeaderAtEnd(std::istream & inStream, std::ios::pos_type & outHeaderPosition)
{
    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error while reading JSON header at end.");
    }

    isx::isize_t sz = 0;
    inStream.seekg(-int32_t(sizeof(sz)), std::ios_base::end);
    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error while seeking to length of JSON header at end.");
    }
    inStream.read(reinterpret_cast<char *>(&sz), sizeof(sz));
    
    int64_t offset = int64_t(sz) + 1 + sizeof(sz);
    inStream.seekg(-offset, std::ios_base::end);
    outHeaderPosition = inStream.tellg();
    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error while seeking to beginning of JSON header at end.");
    }
    std::string jsonStr;
    json jsonObject;
    try
    {
        std::getline(inStream, jsonStr, '\0');
        jsonObject = json::parse(jsonStr);
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
writeJsonHeaderAtEnd(
    const json & inJsonObject,
    std::ostream & inStream)
{
    std::stringstream st;
    
    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
                  "Failed to access stream when writing JSON footer.");
    }

    st << std::setw(4) << inJsonObject;
    isx::isize_t sz = st.str().length();
    
    inStream << st.str() << '\0';
    inStream.write(reinterpret_cast<char *>(&sz), sizeof(sz));
    
    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Failed to write JSON footer.");
    }
    
}

} // namespace isx
