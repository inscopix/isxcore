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
convertGroupToJson(const Group * inGroup)
{
    json outJson;
    outJson["type"] = "Group";
    outJson["name"] = inGroup->getName();

    outJson["groups"] = json::array();
    std::vector<Group *> groups = inGroup->getGroups();
    std::vector<Group *>::const_iterator groupIt;
    for (groupIt = groups.begin(); groupIt != groups.end(); ++groupIt)
    {
        json group = convertGroupToJson(*groupIt);
        outJson["groups"].push_back(group);
    }

    outJson["dataSets"] = json::array();
    std::vector<DataSet *> dataSets = inGroup->getDataSets();
    std::vector<DataSet *>::const_iterator dataSetIt;
    for (dataSetIt = dataSets.begin(); dataSetIt != dataSets.end(); ++dataSetIt)
    {
        json dataSet = convertDataSetToJson(*dataSetIt);
        outJson["dataSets"].push_back(dataSet);
    }

    return outJson;
}

std::unique_ptr<Group>
createProjectTreeFromJson(const json & inJson)
{
    std::string name = inJson["name"];
    std::unique_ptr<Group> root(new Group(name));

    json dataSets = inJson["dataSets"];
    for (json::iterator it = dataSets.begin(); it != dataSets.end(); ++it)
    {
        createAndAddDataSetFromJson(root.get(), *it);
    }
    json groups = inJson["groups"];
    for (json::iterator it = groups.begin(); it != groups.end(); ++it)
    {
        createAndAddGroupTreeFromJson(root.get(), *it);
    }

    return root;
}

json
convertDataSetToJson(const DataSet * inDataSet)
{
    json outJson;
    outJson["type"] = "DataSet";
    outJson["name"] = inDataSet->getName();
    outJson["dataSetType"] = isize_t(inDataSet->getType());
    outJson["fileName"] = inDataSet->getFileName();
    return outJson;
}

void
createAndAddDataSetFromJson(Group * inGroup, const json & inJson)
{
    std::string name = inJson["name"];
    DataSet::Type dataSetType = DataSet::Type(isize_t(inJson["dataSetType"]));
    std::string fileName = inJson["fileName"];
    inGroup->createAndAddDataSet(name, dataSetType, fileName);
}

void
createAndAddGroupTreeFromJson(Group * inGroup, const json & inJson)
{
    Group * group = inGroup->createAndAddGroup(inJson["name"]);
    json dataSets = inJson["dataSets"];
    for (json::iterator it = dataSets.begin(); it != dataSets.end(); ++it)
    {
        createAndAddDataSetFromJson(group, *it);
    }

    json groups = inJson["groups"];
    for (json::iterator it = groups.begin(); it != groups.end(); ++it)
    {
        createAndAddGroupTreeFromJson(group, *it);
    }
}

} // namespace isx
