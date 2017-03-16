#include "isxSeriesIdentifier.h"
#include "isxAssert.h"

namespace isx
{

std::atomic<isize_t>            SeriesIdentifier::s_nextAvailableId;
SeriesIdentifier::SeriesMap     SeriesIdentifier::s_seriesMap;

    
SeriesIdentifier::SeriesIdentifier(Series * inSeries)
    : m_id(std::to_string(s_nextAvailableId++))
{
    auto it = s_seriesMap.find(m_id);
    ISX_ASSERT(it == s_seriesMap.end());
    s_seriesMap[m_id] = inSeries;
}

SeriesIdentifier::~SeriesIdentifier()
{
    auto it = s_seriesMap.find(m_id);
    ISX_ASSERT(it != s_seriesMap.end());
    s_seriesMap.erase(it);
}

SeriesIdentifier::IdentifierType
SeriesIdentifier::getId() const
{
    return m_id;
}

Series *
SeriesIdentifier::getSeries(IdentifierType inId)
{
    Series * ret = nullptr;
    auto it = s_seriesMap.find(inId);
    if (it != s_seriesMap.end())
    {
        ret = s_seriesMap.at(inId);
    }
    return ret;
}

}; // namespace
