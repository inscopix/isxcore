#include "isxTimingInfo.h"
#include <cmath>
#include <algorithm>

namespace isx
{

TimingInfo::TimingInfo()
{
}

TimingInfo::TimingInfo(const Time & start, const DurationInSeconds & step, isize_t numTimes, const std::vector<isize_t> & droppedFrames)
: m_start(start)
, m_step(step)
, m_numTimes(numTimes)
, m_droppedFrames(droppedFrames)
{
    std::sort(m_droppedFrames.begin(), m_droppedFrames.end());
    m_isValid = true;
}

Time
TimingInfo::getStart() const
{
    return m_start;
}

Time
TimingInfo::getEnd() const
{
    return m_start + getDuration();
}

DurationInSeconds
TimingInfo::getStep() const
{
    return m_step;
}

isize_t
TimingInfo::getNumTimes() const
{
    return m_numTimes;
}

DurationInSeconds
TimingInfo::getDuration() const
{
    return m_step * m_numTimes;
}

isize_t
TimingInfo::convertTimeToIndex(const Time & inTime) const
{
    if (m_numTimes == 0)
    {
        return 0;
    }

    Ratio secsFromStart = inTime - m_start;
    double index = std::floor((secsFromStart / m_step).toDouble());

    if (index <= 0)
    {
        return 0;
    }
    else if (index >= m_numTimes)
    {
        return m_numTimes - 1;
    }
    else
    {
        return isize_t(index);
    }
}

Time
TimingInfo::convertIndexToMidTime(isize_t inIndex) const
{
    if (m_numTimes == 0)
    {
        return m_start;
    }

    isx::DurationInSeconds centerOffset = m_step / 2;
    isize_t index = inIndex;
    if (index == 0)
    {
        return m_start + centerOffset;
    }
    else if (index >= m_numTimes)
    {
         index = m_numTimes - 1;
    }

    return m_start + centerOffset + (m_step * index);
}

Time
TimingInfo::convertIndexToStartTime(isize_t inIndex) const
{
    if (m_numTimes == 0)
    {
        return m_start;
    }

    isize_t index = inIndex;
    if (index == 0)
    {
        return m_start;
    }
    else if (index >= m_numTimes)
    {
         index = m_numTimes - 1;
    }

    Time ret = m_start + (m_step * index);
    return ret;
}

bool
TimingInfo::overlapsWith(const TimingInfo & inOther) const
{
    const bool thisStartsAfterOtherEnds = getStart() >= inOther.getEnd();
    const bool thisEndsBeforeOtherStarts = getEnd() <= inOther.getStart();
    return !(thisStartsAfterOtherEnds || thisEndsBeforeOtherStarts);
}

bool
TimingInfo::operator ==(const TimingInfo& other) const
{
    return (m_start == other.m_start)
        && (m_step == other.m_step)
        && (m_numTimes == other.m_numTimes)
        && (m_droppedFrames == other.m_droppedFrames);
}

void
TimingInfo::serialize(std::ostream& strm) const
{
    strm << "TimingInfo("
            << "Start=" << m_start << ", "
            << "Step=" << m_step << ", "
            << "NumTimes=" << m_numTimes
         << ")";
}


void 
TimingInfo::setValid(bool inValid)
{
    m_isValid = inValid;
}


bool 
TimingInfo::isValid() const
{
    return m_isValid;
}

const std::vector<isize_t> & 
TimingInfo::getDroppedFrames() const
{
    return m_droppedFrames;
}

isize_t
TimingInfo::getDroppedCount() const
{
    return m_droppedFrames.size();
}

bool 
TimingInfo::isDropped(isize_t inIndex) const
{
    auto & df = m_droppedFrames;
    return std::binary_search(df.begin(), df.end(), inIndex);    
}


isize_t 
TimingInfo::timeIdxToRecordedIdx(isize_t inIndex) const
{
    /// ISX_ASSERT(!isDropped(inIndex))? inIndex should not be a dropped index but I don't want to perform this 
    /// search for every requested frame (for the second time). 
    auto it = std::find_if(
        m_droppedFrames.begin(),
        m_droppedFrames.end(),
        [&inIndex](const isize_t &index)
        { return index > inIndex; });

    isize_t numDroppedBefore = it - m_droppedFrames.begin();
    isize_t recordedIdx = inIndex - numDroppedBefore;
    
    return recordedIdx;
}


TimingInfo 
TimingInfo::getDefault(isize_t inFrames, const std::vector<isize_t> & inDroppedFrames)
{
    Time start;                       // Default to Unix epoch
    DurationInSeconds step(50, 1000); // Default to 20Hz
    return TimingInfo(start, step, inFrames, inDroppedFrames);
}

} // namespace
