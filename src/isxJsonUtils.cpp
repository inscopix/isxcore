#include "isxJsonUtils.h"

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
    return j;
}

TimingInfo
convertJsonToTimingInfo(const json & j)
{
    isize_t numTimes = j["numTimes"];
    DurationInSeconds step = convertJsonToRatio(j["period"]);
    Time start = convertJsonToTime(j["start"]);
    return TimingInfo(start, step, numTimes);
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
convertGroupToJson(const SpGroup_t & inGroup)
{
    json outJson;
    outJson["type"] = "Group";
    outJson["name"] = inGroup->getName();

    outJson["groups"] = json::array();
    std::vector<SpGroup_t> groups = inGroup->getGroups();
    std::vector<SpGroup_t>::const_iterator groupIt;
    for (groupIt = groups.begin(); groupIt != groups.end(); ++groupIt)
    {
        json group = convertGroupToJson(*groupIt);
        outJson["groups"].push_back(group);
    }

    outJson["dataSets"] = json::array();
    std::vector<SpDataSet_t> dataSets = inGroup->getDataSets();
    std::vector<SpDataSet_t>::const_iterator dataSetIt;
    for (dataSetIt = dataSets.begin(); dataSetIt != dataSets.end(); ++dataSetIt)
    {
        json dataSet = convertDataSetToJson(*dataSetIt);
        outJson["dataSets"].push_back(dataSet);
    }

    return outJson;
}

SpGroup_t
convertJsonToGroup(const json & inJson)
{
    std::string name = inJson["name"];

    SpGroup_t outGroup = std::make_shared<Group>(name);

    json groups = inJson["groups"];
    for (json::iterator it = groups.begin(); it != groups.end(); ++it)
    {
        outGroup->addGroup(convertJsonToGroup(*it));
    }

    json dataSets = inJson["dataSets"];
    for (json::iterator it = dataSets.begin(); it != dataSets.end(); ++it)
    {
        outGroup->addDataSet(convertJsonToDataSet(*it));
    }

    return outGroup;
}

json
convertDataSetToJson(const SpDataSet_t & inDataSet)
{
    json outJson;
    outJson["type"] = "DataSet";
    outJson["name"] = inDataSet->getName();
    outJson["dataSetType"] = isize_t(inDataSet->getType());
    outJson["fileName"] = inDataSet->getFileName();
    return outJson;
}

SpDataSet_t
convertJsonToDataSet(const json & inJson)
{
    std::string name = inJson["name"];
    DataSet::Type dataSetType = DataSet::Type(isize_t(inJson["dataSetType"]));
    std::string fileName = inJson["fileName"];
    SpDataSet_t dataSet = std::make_shared<DataSet>(name, dataSetType, fileName);
    return dataSet;
}

}
