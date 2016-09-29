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
    outJson["groupType"] = isize_t(inGroup->getType());
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
        createDataSetFromJson(root.get(), *it);
    }
    json groups = inJson["groups"];
    for (json::iterator it = groups.begin(); it != groups.end(); ++it)
    {
        createGroupTreeFromJson(root.get(), *it);
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
    // TODO sweet : when a dataset in a project is serialized, we should
    // store the relative path to the project file name, so that if that
    // sub-tree gets moved, then the paths are still accurate.
    // For any file names that are above the project file, we should store
    // absolute paths.
    outJson["fileName"] = inDataSet->getFileName();
    outJson["properties"] = inDataSet->getProperties();
    return outJson;
}

void
createDataSetFromJson(Group * inGroup, const json & inJson)
{
    std::string name = inJson["name"];
    DataSet::Type dataSetType = DataSet::Type(isize_t(inJson["dataSetType"]));
    std::string fileName = inJson["fileName"];
    DataSet::Properties properties = inJson["properties"];
    inGroup->createDataSet(name, dataSetType, fileName, properties);
}

void
createGroupTreeFromJson(Group * inGroup, const json & inJson)
{
    const Group::Type groupType = Group::Type(isize_t(inJson["groupType"]));
    Group * group = inGroup->createGroup(inJson["name"], groupType);
    json dataSets = inJson["dataSets"];
    for (json::iterator it = dataSets.begin(); it != dataSets.end(); ++it)
    {
        createDataSetFromJson(group, *it);
    }

    json groups = inJson["groups"];
    for (json::iterator it = groups.begin(); it != groups.end(); ++it)
    {
        createGroupTreeFromJson(group, *it);
    }
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
