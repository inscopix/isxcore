#include "isxIndexRange.h"
#include "isxAssert.h"

#include <algorithm>
#include <sstream>
#include "json.hpp"

namespace isx
{

IndexRange::IndexRange()
    : m_first(0)
    , m_last(0)
{
}

IndexRange::IndexRange(const isize_t inIndex)
    : m_first(inIndex)
    , m_last(inIndex)
{
}

IndexRange::IndexRange(const isize_t inFirst, const isize_t inLast)
    : m_first(inFirst)
    , m_last(inLast)
{
    ISX_ASSERT(inFirst <= inLast);
}

IndexRange::IndexRange(const std::string & inStr)
{
    std::istringstream ss(inStr);
    ss >> m_first;
    ss.ignore(2, s_delimiter);
    if (ss.eof())
    {
        m_last = m_first;
    }
    else
    {
        ISX_ASSERT(!(ss.bad() || ss.fail()));
        ss >> std::skipws >> m_last;
    }
}

std::string
IndexRange::toString() const
{
    std::ostringstream ss;
    ss << m_first;
    if (m_first != m_last)
    {
        ss << " " << s_delimiter << " " << m_last;
    }
    return ss.str();
}

size_t
IndexRange::getSize() const
{
    return m_last - m_first + 1;
}

bool
IndexRange::contains(const isize_t inIndex) const
{
    return (inIndex >= m_first) && (inIndex <= m_last);
}

bool
IndexRange::operator ==(const IndexRange & inOther) const
{
    return (m_first == inOther.m_first) && (m_last == inOther.m_last);
}

bool
IndexRange::operator <(const IndexRange & inOther) const
{
    if (m_first == inOther.m_first)
    {
        return m_last < inOther.m_last;
    }
    else
    {
        return m_first < inOther.m_first;
    }
}

std::ostream &
operator <<(std::ostream & inOutStream, const IndexRange & inRange)
{
    inOutStream << inRange.toString();
    return inOutStream;
}

std::string
convertIndexRangesToString(const IndexRanges_t & inRanges)
{
    using json = nlohmann::json;
    auto j = json::array();
    const size_t numRanges = inRanges.size();
    for (size_t r = 0; r < numRanges; ++r)
    {
        j.push_back(inRanges[r].toString());
    }
    return j.dump(4);
}

IndexRanges_t
sortAndCompactIndexRanges(const IndexRanges_t & inRanges)
{
    if (inRanges.empty())
    {
        return IndexRanges_t();
    }

    IndexRanges_t sortedRanges = inRanges;
    std::sort(sortedRanges.begin(), sortedRanges.end());
    IndexRange curRange = sortedRanges[0];
    IndexRanges_t compactRanges;
    for (size_t r = 1; r < sortedRanges.size(); ++r)
    {
        const IndexRange range = sortedRanges[r];

        // Compact adjacent ranges (test range against curRange)
        // If ranges are adjacent, 
        //     update the end of curRange
        //     then test next range in sortedRanges against curRange
        // Store curRange in resulting ranges only if it is not adjacent to the one currently tested.
        if (curRange.m_last + 1 >= range.m_first)
        {
            // need to use max in case range is entirely within the bounds of curRange
            curRange.m_last = std::max(curRange.m_last, range.m_last);
        }
        else
        {
            compactRanges.push_back(curRange);
            curRange = range;
        }
    }
    compactRanges.push_back(curRange);
    return compactRanges;
}

} // namespace isx
