#include "isxSeries.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxSpacingInfo.h"
#include "isxTimingInfo.h"
#include "isxMovieFactory.h"
#include "isxPathUtils.h"
#include "isxJsonUtils.h"
#include "isxSeriesIdentifier.h"

#include "json.hpp"

#include <string>
#include <queue>

namespace isx
{

using json = nlohmann::json;

Series::Series()
    : m_valid(false)
    , m_modified(false)
    , m_container(nullptr)
    , m_identifier(new SeriesIdentifier(this))
{
}

Series::Series(const std::string & inName)
    : m_valid(true)
    , m_modified(false)
    , m_container(nullptr)
    , m_name(inName)
    , m_identifier(new SeriesIdentifier(this))
{
}

Series::Series(const std::string & inName,
    const DataSet::Type inType,
    const std::string & inFileName,
    const HistoricalDetails & inHistory,
    const DataSet::Properties & inProperties,
    const bool inImported)
    : m_valid(true)
    , m_modified(false)
    , m_dataSet(std::make_shared<DataSet>(inName, inType, inFileName, inHistory, inProperties, inImported))
    , m_container(nullptr)
    , m_name(inName)
    , m_identifier(new SeriesIdentifier(this))
{
}

Series::Series(const SpDataSet_t & inDataSet)
    : m_valid(true)
    , m_modified(false)
    , m_dataSet(inDataSet)
    , m_container(nullptr)
    , m_name(inDataSet->getName())
    , m_identifier(new SeriesIdentifier(this))
{
}

Series::~Series() = default;

isize_t 
Series::getNumDataSets() const
{
    if (isUnitary())
    {
        return 1;
    }
    return m_unitarySeries.size();
}

std::vector<DataSet *>
Series::getDataSets() const
{
    std::vector<DataSet *> ret;
    if (isUnitary())
    {
        ret.push_back(m_dataSet.get());
    }
    else
    {
        for (const auto & ps : m_unitarySeries)
        {
            ret.push_back(ps->getDataSet(0));
        }
    }

    return ret;
}

DataSet *
Series::getDataSet(isize_t inIndex) const
{
    if (isUnitary())
    {
        if (inIndex > 0)
        {
            ISX_THROW(ExceptionDataIO, "Unitary Series does not have dataset with index: ", inIndex);
        }
        return m_dataSet.get();
    }
    
    return m_unitarySeries.at(inIndex)->getDataSet(0);
}
    
SpSeries_t
Series::getUnitarySeries(isize_t inIndex) const
{
    ISX_ASSERT(!isUnitary());
    return m_unitarySeries.at(inIndex);
}

DataSet::Type
Series::getType() const
{
    return getDataSet(0)->getType();
}

void
Series::insertUnitarySeries(const SpSeries_t & inUnitarySeries)
{
    if (isUnitary())
    {
        ISX_THROW(ExceptionSeries, "Can't add DataSets to a unitary Series!");
    }

    if (!inUnitarySeries->isUnitary())
    {
        ISX_THROW(ExceptionSeries, "Only unitary Sereis can be inserted!");
    }

    if (hasUnitarySeries(inUnitarySeries.get()))
    {
        ISX_THROW(ExceptionDataIO, "There is already a data set with the name: ", inUnitarySeries->getName());
    }
    
    if (inUnitarySeries->getContainer() != nullptr)
    {
        ISX_THROW(ExceptionDataIO, "Series is already in another container!");
    }

    auto ds = inUnitarySeries->getDataSet(0);

    std::string message;
    if (!checkDataSet(ds, message))
    {
        ISX_THROW(ExceptionSeries, message);
    }

    const Time start = ds->getTimingInfo().getStart();
    size_t index = 0;
    for (const auto & mds : getDataSets())
    {
        if (start < mds->getTimingInfo().getStart())
        {
            break;
        }
        ++index;
    }

    inUnitarySeries->setContainer(this);
    m_unitarySeries.insert(m_unitarySeries.begin() + index, inUnitarySeries);
    m_modified = true;
}

SpSeries_t
Series::removeDataSet(const DataSet * inDataSet)
{
    auto it = std::find_if(
        m_unitarySeries.begin(),
        m_unitarySeries.end(),
        [inDataSet](const SpSeries_t & s)
        {
            return s->getDataSet(0) == inDataSet;
        });

    if (it == m_unitarySeries.end())
    {
        ISX_THROW(ExceptionDataIO, "Could not find item with the name: ", inDataSet->getName());
    }
    auto item = *it;
    item->setContainer(nullptr);
    m_unitarySeries.erase(it);
    m_modified = true;
    return item;
}

SpSeries_t
Series::removeDataSet(const Series * inUnitarySeries)
{
    auto it = std::find_if(
        m_unitarySeries.begin(),
        m_unitarySeries.end(),
        [inUnitarySeries](const SpSeries_t & s)
        {
            return s.get() == inUnitarySeries;
        });

    if (it == m_unitarySeries.end())
    {
        ISX_THROW(ExceptionDataIO, "Could not find item with the name: ", inUnitarySeries->getName());
    }
    auto item = *it;
    item->setContainer(nullptr);
    m_unitarySeries.erase(it);
    m_modified = true;
    return item;
}

isize_t
Series::getMemberIndex() const
{
    isize_t index = 0;
    ISX_ASSERT(m_container, "Orphaned child does not have a owning group.");
    
    if (m_container == nullptr)
    {
        return index;
    }
    if (m_container->getItemType() == ProjectItem::Type::GROUP)
    {
        auto g = static_cast<Group *>(m_container);
        for (const auto & m : g->getGroupMembers())
        {
            ++index;
            if (m == this)
            {
                return index;
            }
        }
        ISX_ASSERT(false, "Non-orphaned child cannot be found in parent.");
     }
    return index;
}

HistoricalDetails
Series::getHistoricalDetails() const
{
    if (isUnitary())
    {
        return m_dataSet->getHistoricalDetails();
    }
    else if (m_unitarySeries.size())
    {
         return m_unitarySeries.at(0)->getHistoricalDetails();
    }

    return HistoricalDetails();
}

std::string
Series::getHistory() const
{
    std::string str;
    if (isUnitary())
    {
        str = m_dataSet->getHistory();
    }
    else if (m_unitarySeries.size())
    {
        str = m_unitarySeries.at(0)->getHistory();
    }
    return str;
}

isize_t
Series::getNumChildren() const
{
    return m_children.size();
}

bool
Series::addChild(SpSeries_t inSeries)
{
    ISX_ASSERT(inSeries);
    if (inSeries)
    {
        ISX_ASSERT(inSeries->getParent() == nullptr);
        inSeries->setParent(this);
        m_children.push_back(inSeries);
        m_modified = true;
        return true;
    }
    return false;
}

bool
Series::addChildWithCompatibilityCheck(SpSeries_t inSeries, std::string & outErrorMessage)
{
    if (inSeries != nullptr)
    {
        const DataSet::Type thisType = getType();
        const DataSet::Type childType = inSeries->getType();

        switch (thisType)
        {
            case DataSet::Type::MOVIE:
            {
                if (childType == DataSet::Type::MOVIE)
                {
                    if (!checkSeriesIsTemporallyContained(inSeries))
                    {
                        outErrorMessage = "A movie can only derive movies that are within its time span.";
                        return false;
                    }
                }
                else if (childType == DataSet::Type::CELLSET)
                {
                    if (!checkSeriesHasSameNumPixels(inSeries))
                    {
                        outErrorMessage = "A movie can only derive cellsets with the same number of pixels.";
                        return false;
                    }

                    if (!checkSeriesIsTemporallyContained(inSeries))
                    {
                        outErrorMessage = "A movie can only derive cellsets that are within its time span.";
                        return false;
                    }
                }
                else
                {
                    outErrorMessage = "A movie can only derive movies and cellsets.";
                    return false;
                }
                break;
            }
            case DataSet::Type::CELLSET:
            {
                if (childType == DataSet::Type::CELLSET)
                {
                    if (!checkSeriesIsTemporallyContained(inSeries))
                    {
                        outErrorMessage = "A cellset can only derive cellsets that are within its time span.";
                        return false;
                    }
                }
                else
                {
                    outErrorMessage = "A cellset can only derive other cellsets.";
                    return false;
                }
                break;
            }
            case DataSet::Type::BEHAVIOR:
            case DataSet::Type::GPIO:
            default:
            {
                if (!isASuitableParent(outErrorMessage))
                {
                    return false;
                }
            }
        }
    }
    return addChild(inSeries);
}

bool
Series::isASuitableParent(std::string & outErrorMessage) const
{
    // An empty series is always suitable.
    // TODO sweet : instead of having this check to stop getType
    // failing, we might want to add a NONE type to DataSet::Type.
    if (!isUnitary() && m_unitarySeries.empty())
    {
        return true;
    }
    const DataSet::Type type = getType();
    if (type == DataSet::Type::BEHAVIOR || type == DataSet::Type::GPIO)
    {
        outErrorMessage = "Only movies and cellsets can derive other datasets.";
        return false;
    }
    return true;
}

bool
Series::checkSeriesIsTemporallyContained(const SpSeries_t inSeries) const
{
    const std::vector<DataSet *> & thisDss = getDataSets();
    ISX_ASSERT(thisDss.size() > 0);
    const Time thisStart = thisDss.front()->getTimingInfo().getStart();
    const Time thisEnd = thisDss.back()->getTimingInfo().getEnd();

    const std::vector<DataSet *> & childDss = inSeries->getDataSets();
    ISX_ASSERT(childDss.size() > 0);
    const Time childStart = childDss.front()->getTimingInfo().getStart();
    const Time childEnd = childDss.back()->getTimingInfo().getEnd();

    return childStart >= thisStart && childEnd <= thisEnd;
}

bool
Series::checkSeriesHasSameNumPixels(const SpSeries_t inSeries) const
{
    // First dataset of a series should have representative spacing info
    const std::vector<DataSet *> & thisDss = getDataSets();
    ISX_ASSERT(thisDss.size() > 0);
    const SizeInPixels_t thisNumPixels = thisDss.front()->getSpacingInfo().getNumPixels();

    const std::vector<DataSet *> & childDss = inSeries->getDataSets();
    ISX_ASSERT(childDss.size() > 0);
    const SizeInPixels_t childNumPixels = childDss.front()->getSpacingInfo().getNumPixels();

    return thisNumPixels == childNumPixels;
}

Series *
Series::getChild(isize_t inIndex) const
{
    return m_children.at(inIndex).get();
}

Series *
Series::getChild(const std::string & inName) const
{
    auto it = std::find_if(m_children.begin(), m_children.end(), [&inName](const SpSeries_t & s)
        {
           return s->getName() == inName;
        });

    if (it != m_children.end())
    {
        return it->get();
    }

    return nullptr;
}

std::vector<Series *>
Series::getChildren(const bool inRecurse) const
{
    std::vector<Series *> ret;
    for (const auto & c : m_children)
    {
        ret.push_back(c.get());
        if (inRecurse)
        {
            const auto descendants = c->getChildren(true);
            ret.insert(ret.end(), descendants.begin(), descendants.end());
        }
    }
    return ret;
}

void
Series::setParent(Series * inSeries)
{
    ISX_ASSERT(m_parent == nullptr);
    m_parent = inSeries;
}

Series *
Series::getParent() const
{
    return m_parent;
}

SpSeries_t
Series::removeChild(isize_t inIndex)
{
    SpSeries_t ret = m_children.at(inIndex);
    m_children.erase(m_children.begin() + inIndex);
    return ret;
}

bool
Series::checkDataSet(DataSet * inDataSet, std::string & outMessage)
{
    try
    {
        auto dss = getDataSets();
        if (dss.empty())
        {
            return true;
        }

        DataSet * refDs = dss.at(0);

        if (!checkDataSetType(refDs->getType(), inDataSet->getType(), outMessage))
        {
            return false;
        }

        if (!checkDataType(refDs->getDataType(), inDataSet->getDataType(), outMessage))
        {
            return false;
        }

        if (!checkSpacingInfo(refDs->getSpacingInfo(), inDataSet->getSpacingInfo(), outMessage))
        {
            return false;
        }

        const TimingInfo & timingInfo = inDataSet->getTimingInfo();
        for (size_t i = 0; i < dss.size(); ++i)
        {
            if (i > 0)
            {
                refDs = dss.at(i);
            }
            if (!checkTimingInfo(refDs->getTimingInfo(), timingInfo, outMessage))
            {
                return false;
            }
        }

        if(!checkHistory(refDs->getHistoricalDetails(), inDataSet->getHistoricalDetails(), outMessage))
        {
            return false;
        }
    } catch (Exception & inException)
    {
        outMessage = inException.what();
        return false;
    }

    return true;
}

bool
Series::checkDataSetType(
        const DataSet::Type inRef,
        const DataSet::Type inNew,
        std::string & outMessage)
{
    if (inRef != inNew)
    {
        outMessage = "The DataSet type is different from that of the reference.";
        return false;
    }
    return true;
}

bool
Series::checkDataType(
        const DataType inRef,
        const DataType inNew,
        std::string & outMessage)
{
    if (inRef != inNew)
    {
        outMessage = "The data type is different from that of the reference.";
        return false;
    }
    return true;
}

bool
Series::checkTimingInfo(
        const TimingInfo & inRef,
        const TimingInfo & inNew,
        std::string & outMessage)
{
    if (inNew.getStep() != inRef.getStep())
    {
        outMessage = "The timing info has a different frame rate than the reference.";
        return false;
    }
    if (inNew.overlapsWith(inRef))
    {
        outMessage = "The timing info temporally overlaps with the reference.";
        return false;
    }
    return true;
}

bool 
Series::checkHistory(
        const HistoricalDetails & inRef,
        const HistoricalDetails & inNew,
        std::string & outMessage)
{
    if (inRef != inNew)
    {
        outMessage = "The history details are different than those of the reference.";
        return false;
    }
    return true;
}

bool
Series::checkSpacingInfo(
        const SpacingInfo & inRef,
        const SpacingInfo & inNew,
        std::string & outMessage)
{
    if (!(inRef == inNew))
    {
        outMessage = "The spacing info is different than that of the reference.";
        return false;
    }
    return true;
}

ProjectItem::Type
Series::getItemType() const
{
    return ProjectItem::Type::SERIES;
}

bool
Series::isValid() const
{
    return m_valid;
}

const std::string &
Series::getName() const
{
    return m_name;
}

void
Series::setName(const std::string & inName)
{
    m_name = inName;
    if (m_dataSet != nullptr)
    {
        m_dataSet->setName(inName);
    }
    m_modified = true;
}

bool
Series::isModified() const
{
    if (m_modified || (m_dataSet && m_dataSet->isModified()))
    {
        return true;
    }
    for (const auto & i: m_unitarySeries)
    {
        if (i->isModified())
        {
            return true;
        }
    }
    return false;
}

void
Series::setUnmodified()
{
    m_modified = false;
    if (m_dataSet)
    {
        m_dataSet->setUnmodified();
    }
    for (const auto & i : m_unitarySeries)
    {
        i->setUnmodified();
    }
}

std::string
Series::getUniqueIdentifier() const
{
    return m_identifier->getId();
}

std::string
Series::toJsonString(const bool inPretty, const std::string & inPathToOmit) const
{
    json jsonObj;
    jsonObj["itemType"] = size_t(getItemType());
    jsonObj["name"] = m_name;
    jsonObj["dataSets"] = json::array();
    jsonObj["isUnitary"] = isUnitary();
    if (isUnitary())
    {
        jsonObj["dataSets"].push_back(json::parse(m_dataSet->toJsonString(inPretty, inPathToOmit)));
    }
    else
    {
        for (const auto & dataSet : getDataSets())
        {
            jsonObj["dataSets"].push_back(json::parse(dataSet->toJsonString(inPretty, inPathToOmit)));
        }
    }

    jsonObj["children"] = json::array();
    for (const auto & c : m_children)
    {
        jsonObj["children"].push_back(json::parse(c->toJsonString(inPretty, inPathToOmit)));
    }

    if (inPretty)
    {
        return jsonObj.dump(4);
    }
    return jsonObj.dump();
}

SpSeries_t
Series::fromJsonString(const std::string & inString, const std::string & inAbsolutePathToPrepend)
{
    SpSeries_t ret;
    if (inString == json::object().dump())
    {
        return SpSeries_t();
    }

    const json jsonObj = json::parse(inString);
    const ProjectItem::Type itemType = ProjectItem::Type(size_t(jsonObj.at("itemType")));
    ISX_ASSERT(itemType == ProjectItem::Type::SERIES);
    const std::string name = jsonObj.at("name");
    bool isSeriesUnitary = jsonObj.at("isUnitary");
    if (isSeriesUnitary)
    {
        auto jds = jsonObj.at("dataSets")[0];
        auto dataSet = DataSet::fromJsonString(jds.dump(), inAbsolutePathToPrepend);
        ret = std::make_shared<Series>(dataSet);
        ret->setName(name);
    }
    else
    {
        ret = std::make_shared<Series>(name);
        for (const auto & jsonDataSet : jsonObj.at("dataSets"))
        {
            SpDataSet_t dataSet = DataSet::fromJsonString(jsonDataSet.dump(), inAbsolutePathToPrepend);
            auto tmpUnitarySeries = std::make_shared<Series>(dataSet);
            ret->insertUnitarySeries(tmpUnitarySeries);
        }
    }
    if (jsonObj.find("children") != jsonObj.end())
    {
        auto children = jsonObj.find("children");
        for (const auto & c : *children)
        {
            ret->addChild(Series::fromJsonString(c.dump(), inAbsolutePathToPrepend));
        }
    }

    return ret;
}

bool
Series::operator ==(const ProjectItem & other) const
{
    if (other.getItemType() != ProjectItem::Type::SERIES)
    {
        return false;
    }
    auto otherSeries = static_cast<const Series *>(&other);

    bool equal = (m_name == otherSeries->m_name)
        && (m_unitarySeries.size() == otherSeries->m_unitarySeries.size());
    for (size_t i = 0; i < m_unitarySeries.size(); ++i)
    {
        if (!(*(m_unitarySeries.at(i)->getDataSet(0)) == *(otherSeries->m_unitarySeries.at(i)->getDataSet(0))))
        {
            equal = false;
            break;
        }
    }
    return equal;
}

bool
Series::hasUnitarySeries(const Series * inUnitarySeries) const
{
    return std::any_of(
        m_unitarySeries.begin(),
        m_unitarySeries.end(),
        [inUnitarySeries](const SpSeries_t & s)
        {
            return s->getName() == inUnitarySeries->getName();
        });
}

bool
Series::isUnitary() const
{
    if (m_dataSet)
    {
        ISX_ASSERT(m_unitarySeries.size() == 0);
        return true;
    }
    return false;
}

void
Series::setContainer(ProjectItem * inContainer)
{
    ISX_ASSERT(m_parent == nullptr);
    m_container = inContainer;
}

ProjectItem *
Series::getContainer() const
{
    return m_container;
}

WpSeries_t
Series::getWeakRef()
{
    return shared_from_this();
}

void
Series::deleteFiles() const
{
    for (const auto & ds : getDataSets())
    {
        ds->deleteFile();
    }
}

bool
Series::isProcessed() const
{
    if (isAMemberOfASeries())
    {
        auto cs = static_cast<const isx::Series *>(m_container);
        return cs->m_children.size() > 0;
    }
    else
    {
        return m_children.size() > 0;
    }
    return false;
}

bool
Series::isAMemberOfASeries() const
{
    return isUnitary() && m_container != nullptr && m_container->getItemType() == isx::ProjectItem::Type::SERIES;
}

std::ostream &
operator<<(::std::ostream & inStream, const Series & inSeries)
{
    inStream << inSeries.toJsonString(true);
    return inStream;
}

} // namespace isx
