#include "isxTimingInfo.h"
#include "isxAssert.h"

#include <cmath>
#include <algorithm>

namespace isx
{

TimingInfo::TimingInfo()
{
}

TimingInfo::TimingInfo(
        const Time & start,
        const DurationInSeconds & step,
        isize_t numTimes,
        const std::vector<isize_t> & droppedFrames,
        const IndexRanges_t & cropped)
: m_start(start)
, m_step(step)
, m_numTimes(numTimes)
{
    // NOTE sweet : the order here is important because we want to crop dropped frames.
    m_cropped = sortAndCompactIndexRanges(cropped);
    cropSortAndSetDroppedFrames(droppedFrames);
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
   
Time
TimingInfo::getLastStartTime() const
{
    return convertIndexToStartTime(getNumTimes() - 1);
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
        && (m_droppedFrames == other.m_droppedFrames)
        && (m_cropped == other.m_cropped);
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

void 
TimingInfo::setDroppedFrames(const std::vector<isize_t> & inDroppedFrames)
{
    m_droppedFrames = inDroppedFrames;
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

const IndexRanges_t &
TimingInfo::getCropped() const
{
    return m_cropped;
}

isize_t
TimingInfo::getCroppedCount() const
{
    isize_t count = 0;
    for (const auto & r : m_cropped)
    {
        count += r.getSize();
    }
    return count;
}

bool
TimingInfo::isCropped(isize_t inIndex) const
{
    for (const auto & r : m_cropped)
    {
        if (r.contains(inIndex))
        {
            return true;
        }
    }
    return false;
}

bool
TimingInfo::isIndexValid(const isize_t inIndex) const
{
    return !(isCropped(inIndex) || isDropped(inIndex));
}

isize_t
TimingInfo::getNumValidTimes() const
{
    ISX_ASSERT((int64_t(m_numTimes) - int64_t(getDroppedCount()) - int64_t(getCroppedCount())) >= 0);
    return m_numTimes - getDroppedCount() - getCroppedCount();
}

isize_t 
TimingInfo::timeIdxToRecordedIdx(isize_t inIndex) const
{
    // Note that this internally asserts that the input index is not dropped.
    auto it = std::find_if(
        m_droppedFrames.begin(),
        m_droppedFrames.end(),
        [&inIndex](const isize_t &index)
        {
            ISX_ASSERT(index != inIndex);
            return index > inIndex;
        }
    );

    // This count depends on the cropped frames being sorted in ascending order
    // and also depends on inIndex not being a cropped frame.
    // This also asserts that the input index is not cropped.
    isize_t numCroppedBefore = 0;
    for (const auto & r : m_cropped)
    {
        ISX_ASSERT(!r.contains(inIndex));
        if (r.m_first > inIndex)
        {
            break;
        }
        numCroppedBefore += r.getSize();
    }

    isize_t numDroppedBefore = it - m_droppedFrames.begin();
    ISX_ASSERT(inIndex >= (numDroppedBefore + numCroppedBefore));
    isize_t recordedIdx = inIndex - numDroppedBefore - numCroppedBefore;

    return recordedIdx;
}


TimingInfo 
TimingInfo::getDefault(isize_t inFrames, const std::vector<isize_t> & inDroppedFrames)
{
    Time start;                       // Default to Unix epoch
    DurationInSeconds step(50, 1000); // Default to 20Hz
    return TimingInfo(start, step, inFrames, inDroppedFrames);
}

void
TimingInfo::cropSortAndSetDroppedFrames(const std::vector<isize_t> & inDroppedFrames)
{
    m_droppedFrames.clear();
    for (const auto df : inDroppedFrames)
    {
        if (!isCropped(df))
        {
            m_droppedFrames.push_back(df);
        }
    }
    std::sort(m_droppedFrames.begin(), m_droppedFrames.end());
}

std::pair<isize_t, isize_t>
getSegmentIndexAndSampleIndexFromGlobalSampleIndex(const TimingInfo inGlobalTimingInfo, const TimingInfos_t & inTimingInfos, isize_t inGlobalSampleIndex)
{
    isize_t fn = inGlobalSampleIndex;
    for (isize_t i = 0; i < inTimingInfos.size(); ++i)
    {
        const isize_t bi = inGlobalTimingInfo.convertTimeToIndex(inTimingInfos[i].getStart());
        const isize_t ei = bi + inTimingInfos[i].getNumTimes();
        if (bi <= fn && fn < ei)
        {
            return std::make_pair(i, fn - bi);
        }
        else if (i < inTimingInfos.size() - 1)
        {
            // not the last individual segment, segment [i+1] is valid
            isize_t bn = inGlobalTimingInfo.convertTimeToIndex(inTimingInfos[i+1].getStart());
            if (ei <= fn && fn < bn)
            {
                // return frame index out of range to indicate the
                // requested index is inbetween individual segments
                // so we can return a placeholder frame
                return std::make_pair(i, inTimingInfos[i].getNumTimes());
            }
        }
    }
    // the index beyond the end of the last individual segment
    // return an out-of-range index for the segment
    return std::make_pair(inTimingInfos.size(), 0); 
}

} // namespace isx
