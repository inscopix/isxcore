#include "isxSeries.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxSpacingInfo.h"
#include "isxTimingInfo.h"
#include "isxMovieFactory.h"
#include "isxPathUtils.h"

#include "json.hpp"

namespace isx
{

using json = nlohmann::json;

Series::Series()
    : m_valid(false)
    , m_modified(false)
    , m_parent(nullptr)
{
}

Series::Series(const std::string & inName)
    : m_valid(true)
    , m_modified(false)
    , m_parent(nullptr)
    , m_name(inName)
{
}

isize_t 
Series::getNumDataSets() const
{
    return m_dataSets.size();
}

std::vector<DataSet *>
Series::getDataSets() const
{
    std::vector<DataSet *> outDataSets;
    for (const auto & dataSet : m_dataSets)
    {
        outDataSets.push_back(dataSet.get());
    }
    return outDataSets;
}

void
Series::insertDataSet(std::shared_ptr<DataSet> & inDataSet)
{
    const std::string name = inDataSet->getName();
    if (isChild(name))
    {
        ISX_THROW(ExceptionDataIO, "There is already a data set with the name: ", name);
    }

    std::string outMessage;
    if (!checkDataSet(inDataSet.get(), outMessage))
    {
        ISX_THROW(ExceptionSeries, outMessage);
    }

    const Time start = inDataSet->getTimingInfo().getStart();
    size_t index = 0;
    for (const auto & dataSet : m_dataSets)
    {
        if (start < dataSet->getTimingInfo().getStart())
        {
            break;
        }
        ++index;
    }

    // If the child still has a parent, this should remove it
    ProjectItem * parent = inDataSet->getParent();
    if (parent != nullptr && parent->isChild(name))
    {
        parent->removeChild(name);
    }

    inDataSet->setParent(this);
    m_dataSets.insert(m_dataSets.begin() + index, inDataSet);
    m_modified = true;
}

std::shared_ptr<DataSet>
Series::removeDataSet(const std::string & inName)
{
    for (auto it = m_dataSets.begin(); it != m_dataSets.end(); ++it)
    {
        if ((*it)->getName() == inName)
        {

            std::shared_ptr<DataSet> item = *it;
            item->setParent(nullptr);
            m_dataSets.erase(it);
            m_modified = true;
            return item;
        }
    }
    ISX_THROW(ExceptionDataIO, "Could not find data set with the name: ", inName);
}

bool 
Series::hasHistory() const
{
    return (m_previous != nullptr);
}

isize_t 
Series::getNumHistoricalItems() const
{
    isize_t num = 0;
    if(m_previous)
    {
        num += 1;
        num += m_previous->getNumHistoricalItems();
    }
    return num;
}

const HistoricalDetails 
Series::getHistory() const
{
    if(m_dataSets.size())
    {
         return m_dataSets[0]->getHistory();
    }

    return HistoricalDetails();
}

void 
Series::setPrevious(const std::shared_ptr<Series> & inSeries)
{
    if (inSeries)
    {
        inSeries->setHistorical();
        inSeries->setParent(this);
    }
    m_previous = inSeries;
}

ProjectItem *
Series::getPrevious() const
{
    return m_previous.get();
}

void 
Series::setHistorical()
{
    m_historical = true;
}

bool 
Series::isHistorical() const
{
    return m_historical;
}

std::string 
Series::getHistoricalDetails() const
{
    std::string str;
    if(m_dataSets.size())
    {
        str = m_dataSets[0]->getHistoricalDetails();
    }
    return str;
}

std::vector<std::shared_ptr<Series>> 
Series::getHistoricalSeries()
{
    std::vector<std::shared_ptr<Series>> outSeries;
    if(m_previous)
    {
        outSeries.push_back(m_previous);        
        std::vector<std::shared_ptr<Series>> ancestors = m_previous->getHistoricalSeries();
        if(ancestors.empty() == false)
        {
            outSeries.insert(outSeries.end(), ancestors.begin(), ancestors.end());    
        }        
    }
    return outSeries;    
}

bool
Series::checkDataSet(DataSet * inDataSet, std::string & outMessage)
{
    try
    {
        if (!checkDataSetType(inDataSet->getType(), outMessage))
        {
            return false;
        }

        if (m_dataSets.empty())
        {
            return true;
        }

        DataSet * refDs = m_dataSets.at(0).get();

        if (!checkDataType(refDs->getDataType(), inDataSet->getDataType(), outMessage))
        {
            return false;
        }

        if (!checkSpacingInfo(refDs->getSpacingInfo(), inDataSet->getSpacingInfo(), outMessage))
        {
            return false;
        }

        const TimingInfo & timingInfo = inDataSet->getTimingInfo();
        for (size_t i = 0; i < m_dataSets.size(); ++i)
        {
            if (i > 0)
            {
                refDs = m_dataSets.at(i).get();
            }
            if (!checkTimingInfo(refDs->getTimingInfo(), timingInfo, outMessage))
            {
                return false;
            }
        }

        if(!checkHistory(refDs->getHistory(), inDataSet->getHistory(), outMessage))
        {
            return false;
        }
    } catch (isx::Exception & inException)
    {
        outMessage = inException.what();
        return false;
    }

    return true;
}

bool
Series::checkDataSetType(
        const DataSet::Type inNew,
        std::string & outMessage)
{
    if (inNew != isx::DataSet::Type::MOVIE)
    {
        outMessage = "A series can only contain nVista movies.";
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
        outMessage = "The data type is different than that of the reference.";
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

std::string
Series::getName() const
{
    return m_name;
}

void
Series::setName(const std::string & inName)
{
    m_name = inName;
    m_modified = true;
}

ProjectItem * 
Series::getMostRecent() const 
{
    ProjectItem * descendant = getParent();
    if(descendant)
    {
        while(descendant->isHistorical())
        {
            descendant = descendant->getParent();
            if(!descendant)
            {
                break;
            }
        }
    }
    return descendant;
}

ProjectItem *
Series::getParent() const
{
    return m_parent;
}

void
Series::setParent(ProjectItem * inParent)
{
    m_parent = inParent;
}

std::vector<ProjectItem *>
Series::getChildren() const
{
    std::vector<ProjectItem *> outChildren;
    for (const auto & child : m_dataSets)
    {
        outChildren.push_back(child.get());
    }
    return outChildren;
}

size_t
Series::getNumChildren() const
{
    return m_dataSets.size();
}

void
Series::insertChild(std::shared_ptr<ProjectItem> inItem, const int inIndex)
{
    if (inItem->getItemType() != isx::ProjectItem::Type::DATASET)
    {
        ISX_THROW(ExceptionDataIO, "Only data sets can be inserted into series.");
    }

    auto dataSet = std::static_pointer_cast<DataSet>(inItem);
    insertDataSet(dataSet);
}

std::shared_ptr<ProjectItem>
Series::removeChild(const std::string & inName)
{
    return removeDataSet(inName);
}

bool
Series::isModified() const
{
    return m_modified || areChildrenModified();
}

void
Series::setUnmodified()
{
    m_modified = false;
    setChildrenUnmodified();
}

std::string
Series::toJsonString(const bool inPretty, const std::string & inPathToOmit) const
{
    json jsonObj;
    jsonObj["itemType"] = size_t(getItemType());
    jsonObj["name"] = m_name;
    jsonObj["dataSets"] = json::array();
    for (const auto & dataSet : m_dataSets)
    {
        jsonObj["dataSets"].push_back(json::parse(dataSet->toJsonString(inPretty, inPathToOmit)));
    }

    jsonObj["previous"] = json::object();
    if (m_previous)
    {
        jsonObj["previous"] = json::parse(m_previous->toJsonString(inPretty, inPathToOmit));
    }

    if (inPretty)
    {
        return jsonObj.dump(4);
    }
    return jsonObj.dump();
}

std::shared_ptr<Series>
Series::fromJsonString(const std::string & inString, const std::string & inAbsolutePathToPrepend)
{
    if (inString == json::object().dump())
    {
        return std::shared_ptr<Series>();
    }

    const json jsonObj = json::parse(inString);
    const ProjectItem::Type itemType = ProjectItem::Type(size_t(jsonObj.at("itemType")));
    ISX_ASSERT(itemType == ProjectItem::Type::SERIES);
    const std::string name = jsonObj.at("name");
    auto outSeries = std::make_shared<Series>(name);
    for (const auto & jsonDataSet : jsonObj.at("dataSets"))
    {
        std::shared_ptr<DataSet> dataSet = DataSet::fromJsonString(jsonDataSet.dump(), inAbsolutePathToPrepend);
        outSeries->insertDataSet(dataSet);
    }
    if (jsonObj.find("previous") != jsonObj.end())
    {
        outSeries->setPrevious(Series::fromJsonString(jsonObj.at("previous").dump(), inAbsolutePathToPrepend));
    }
    
    return outSeries;
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
        && (m_dataSets.size() == otherSeries->m_dataSets.size());
    for (size_t i = 0; equal && i < m_dataSets.size(); ++i)
    {
        equal &= *m_dataSets.at(i) == *otherSeries->m_dataSets.at(i);
    }
    return equal;
}

} // namespace isx
