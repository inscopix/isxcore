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
convertIndexRangesToJson(const IndexRanges_t & inRanges)
{
    return json::parse(isx::convertIndexRangesToString(inRanges));
}

IndexRanges_t
convertJsonToIndexRanges(const json & j)
{
    IndexRanges_t outRanges;
    for (const std::string & str : j)
    {
        outRanges.push_back(IndexRange(str));
    }
    return outRanges;
}

json
convertTimingInfoToJson(const TimingInfo & inTimingInfo)
{
    json j;
    j["numTimes"] = inTimingInfo.getNumTimes();
    j["period"] = convertRatioToJson(inTimingInfo.getStep());
    j["start"] = convertTimeToJson(inTimingInfo.getStart());
    std::vector<isize_t> droppedFrames = inTimingInfo.getDroppedFrames();
    j["dropped"] = droppedFrames;
    j["cropped"] = convertIndexRangesToJson(inTimingInfo.getCropped());
    std::vector<isize_t> blankFrames = inTimingInfo.getBlankFrames();
    j["blank"] = blankFrames;
    return j;
}

TimingInfo
convertJsonToTimingInfo(const json & j)
{
    isize_t numTimes = j.at("numTimes");
    DurationInSeconds step = convertJsonToRatio(j.at("period"));
    Time start = convertJsonToTime(j.at("start"));
    std::vector<isize_t> droppedFrames;
    if (j.count("dropped") > 0)
    {
        droppedFrames = j.at("dropped").get<std::vector<isize_t>>();
    }
    IndexRanges_t cropped;
    if (j.count("cropped") > 0)
    {
        cropped = convertJsonToIndexRanges(j.at("cropped"));
    }
    std::vector<isize_t> blankFrames;
    if (j.count("blank") > 0)
    {
        blankFrames = j.at("blank").get<std::vector<isize_t>>();
    }
    return TimingInfo(start, step, numTimes, droppedFrames, cropped, blankFrames);
}

json
convertSizeInPixelsToJson(const SizeInPixels_t & inSizeInPixels)
{
    json j;
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

json 
convertPointInPixelsToJson(const PointInPixels_t & inPointInPixels)
{
    json j;
    j["x"] = inPointInPixels.getX();
    j["y"] = inPointInPixels.getY();
    return j;
}

PointInPixels_t convertJsonToPointInPixels(const json & j)
{
    int64_t x = j.at("x");
    int64_t y = j.at("y");
    return PointInPixels_t(x, y);
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
    j["x"] = convertRatioToJson(inSizeInMicrons.getX());
    j["y"] = convertRatioToJson(inSizeInMicrons.getY());
    return j;
}

json
convertPointInMicronsToJson(const PointInMicrons_t & inPointInMicrons)
{
    json j;
    j["x"] = convertRatioToJson(inPointInMicrons.getX());
    j["y"] = convertRatioToJson(inPointInMicrons.getY());
    return j;
}

json
convertSpacingInfoToJson(const SpacingInfo & inSpacingInfo)
{
    json j;
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
        if (it.key() == DataSet::PROP_MOVIE_START_TIME)
        {
            v = Variant(convertJsonToTime(it.value()));
        }
        else if (it.key() == DataSet::PROP_BEHAV_NUM_FRAMES
              || it.key() == DataSet::PROP_BEHAV_GOP_SIZE)
        {
            v = Variant(it.value().get<int64_t>());
        }
        else
        {
            v = Variant(it.value().get<float>());
        }
        properties[it.key()] = v;
    }
    return properties;
}

json
getProducerAsJson()
{
    json producer;
    producer["name"] = "isxcore";
    producer["version"] = CoreVersionVector();
    return producer;
}

json
readJson(std::istream & inStream)
{
    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error while reading JSON.");
    }

    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error while seeking to JSON.");
    }

    json jsonObject;
    try
    {
        std::string jsonStr;
        std::getline(inStream, jsonStr, '\0');
        jsonObject = json::parse(jsonStr);
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

   
    inStream << std::setw(4) << inJsonObject;

    inStream << '\0';

    if (!inStream.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Failed to write JSON.");
    }
}

/////*** Cell Data ***///////

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
convertCellStatusesToJson(const CellStatuses_t & inCellStatuses)
{
    std::vector<int> intCellStatuses(inCellStatuses.size());
    isize_t i = 0;
    for (auto & s : inCellStatuses)
    {
        intCellStatuses[i] = (int)s;
        ++i;
    }
    return intCellStatuses;
}

CellStatuses_t
convertJsonToCellStatuses(const json & inJson)
{
    std::vector<int> intCellStatuses = inJson.get<std::vector<int>>();
    CellStatuses_t cellStatuses(intCellStatuses.size());
    isize_t i = 0;
    for (auto & s : intCellStatuses)
    {
        cellStatuses[i] = (CellSet::CellStatus)s;
        ++i;
    }
    return cellStatuses;
}

json
convertCellColorsToJson(const CellColors_t & inCellColors)
{
    std::vector<Rgba> uintCellColors(inCellColors.size());
    isize_t i = 0;
    for (auto & c : inCellColors)
    {
        uintCellColors[i] = c.m_rgba;
        ++i;
    }
    return uintCellColors;
}

CellColors_t
convertJsonToCellColors(const json & inJson)
{
    std::vector<Rgba> intCellColors = inJson.get<std::vector<Rgba>>();
    CellColors_t cellColors(intCellColors.size());

    isize_t i = 0;
    for (auto & c : intCellColors)
    {
        cellColors[i] = Color(c);
        ++i;
    }
    return cellColors;
}

json
convertCellActivitiesToJson(const CellActivities_t & inCellActivities)
{
    return inCellActivities;
}

CellActivities_t
convertJsonToCellActivities(const json & inJson)
{
    return inJson.get<CellActivities_t>();
}

/////*** Vessel Data ***///////

json
convertVesselNamesToJson(const VesselNames_t & inVesselNames)
{
    return inVesselNames;
}

VesselNames_t
convertJsonToVesselNames(const json & inJson)
{
    return inJson.get<VesselNames_t>();
}

json
convertVesselStatusesToJson(const VesselStatuses_t & inVesselStatuses)
{
    std::vector<int> intVesselStatuses(inVesselStatuses.size());
    isize_t i = 0;
    for (auto & s : inVesselStatuses)
    {
        intVesselStatuses[i] = (int)s;
        ++i;
    }
    return intVesselStatuses;
}

VesselStatuses_t
convertJsonToVesselStatuses(const json & inJson)
{
    std::vector<int> intVesselStatuses = inJson.get<std::vector<int>>();
    VesselStatuses_t vesselStatuses(intVesselStatuses.size());
    isize_t i = 0;
    for (auto & s : intVesselStatuses)
    {
        vesselStatuses[i] = (VesselSet::VesselStatus)s;
        ++i;
    }
    return vesselStatuses;
}

json
convertVesselColorsToJson(const VesselColors_t & inVesselColors)
{
    std::vector<Rgba> uintVesselColors(inVesselColors.size());
    isize_t i = 0;
    for (auto & c : inVesselColors)
    {
        uintVesselColors[i] = c.m_rgba;
        ++i;
    }
    return uintVesselColors;
}

VesselColors_t
convertJsonToVesselColors(const json & inJson)
{
    std::vector<Rgba> intVesselColors = inJson.get<std::vector<Rgba>>();
    VesselColors_t vesselColors(intVesselColors.size());

    isize_t i = 0;
    for (auto & c : intVesselColors)
    {
        vesselColors[i] = Color(c);
        ++i;
    }
    return vesselColors;
}

json
convertVesselActivitiesToJson(const VesselActivities_t & inVesselActivities)
{
    return inVesselActivities;
}

VesselActivities_t
convertJsonToVesselActivities(const json & inJson)
{
    return inJson.get<VesselActivities_t>();
}

/////*** Cell Metrics ***///////

json
convertImageMetricsToJson(const ImageMetrics & inMetrics) 
{
    json outJ = json{
        inMetrics.m_numComponents, 
        convertPointInPixelsToJson(inMetrics.m_overallCenterInPixels), 
        convertPointInPixelsToJson(inMetrics.m_largestComponentCenterInPixels),
        inMetrics.m_overallAreaInPixels, 
        inMetrics.m_largestComponentAreaInPixels,
        inMetrics.m_overallMaxContourWidthInPixels, 
        inMetrics.m_largestComponentMaxContourWidthInPixels};

    return outJ;
}

void
convertJsonToImageMetrics(const json & inJ, ImageMetrics & outMetrics) 
{
    outMetrics.m_numComponents                     = inJ.at(0).get<isize_t>();
    outMetrics.m_overallCenterInPixels             = convertJsonToPointInPixels(inJ.at(1));
    outMetrics.m_largestComponentCenterInPixels    = convertJsonToPointInPixels(inJ.at(2));
    outMetrics.m_overallAreaInPixels               = inJ.at(3).get<float>();
    outMetrics.m_largestComponentAreaInPixels      = inJ.at(4).get<float>();
    outMetrics.m_overallMaxContourWidthInPixels    = inJ.at(5).get<float>();
    outMetrics.m_largestComponentMaxContourWidthInPixels = inJ.at(6).get<float>();
}

json
convertCellMetricsToJson(const CellMetrics_t & inMetrics)
{
    json j;
    for (auto & cm : inMetrics)
    {
        if (cm)
        {
            json jcm = convertImageMetricsToJson(*cm);
            j.push_back(jcm);
        }
        else
        {
            j.push_back(json::object());
        }
        
    }
    return j;
}

CellMetrics_t
convertJsonToCellMetrics(const json & inJson)
{
    CellMetrics_t outMetrics;

    for (auto & j : inJson)
    {
        SpImageMetrics_t im;
        if (!j.empty())
        {
            im = std::make_shared<ImageMetrics>();
            convertJsonToImageMetrics(j, *im);
        }        
        outMetrics.push_back(im);
    }

    return outMetrics;
}


json
convertTraceMetricsToJson(const TraceMetrics & inMetrics)
{
    json outJ = json{
        inMetrics.m_snr,        
        inMetrics.m_mad, 
        inMetrics.m_eventRate,
        inMetrics.m_eventAmpMedian, 
        inMetrics.m_eventAmpSD,
        inMetrics.m_riseMedian, 
        inMetrics.m_riseSD,
        inMetrics.m_decayMedian, 
        inMetrics.m_decaySD};

    return outJ;
}

void
convertJsonToTraceMetrics(const json & inJ, TraceMetrics & outMetrics)
{
    outMetrics.m_snr            = inJ.at(0).get<float>();
    outMetrics.m_mad            = inJ.at(1).get<float>();
    outMetrics.m_eventRate      = inJ.at(2).get<float>();
    outMetrics.m_eventAmpMedian = inJ.at(3).get<float>();
    outMetrics.m_eventAmpSD     = inJ.at(4).get<float>();
    outMetrics.m_riseMedian     = inJ.at(5).get<float>();
    outMetrics.m_riseSD         = inJ.at(6).get<float>();
    outMetrics.m_decayMedian    = inJ.at(7).get<float>();
    outMetrics.m_decaySD        = inJ.at(8).get<float>();
}

json
convertEventMetricsToJson(const EventMetrics_t & inMetrics)
{
    json j;
    for (auto & em : inMetrics)
    {
        if (em)
        {
            json jem = convertTraceMetricsToJson(*em);
            j.push_back(jem);
        }
        else
        {
            j.push_back(json::object());
        }
        
    }
    return j;
}

EventMetrics_t
convertJsonToEventMetrics(const json & inJson)
{
    EventMetrics_t outMetrics;

    for (auto & j : inJson)
    {
        SpTraceMetrics_t tm;
        if (!j.empty())
        {
            tm = std::make_shared<TraceMetrics>();
            convertJsonToTraceMetrics(j, *tm);
        }        
        outMetrics.push_back(tm);
    }

    return outMetrics;
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

void verifyJsonKey(
    const json & inJson,
    const std::string key)
{
    if (inJson.find(key) == inJson.end())
    {
        ISX_THROW(isx::ExceptionDataIO, "Error while parsing JSON object: No key (", key, ") in JSON object.");
    }
}
} // namespace isx
