#include "isxTimingInfo.h"
#include "isxAssert.h"
#include "isxMovieFactory.h"

#include <cmath>
#include <algorithm>

namespace isx
{

const DurationInSeconds TimingInfo::s_defaultStep(50, 1000);

TimingInfo::TimingInfo()
{
}

TimingInfo::TimingInfo(
        const Time & start,
        const DurationInSeconds & step,
        isize_t numTimes,
        const std::vector<isize_t> & droppedFrames,
        const IndexRanges_t & cropped,
        const std::vector<isize_t> & blankFrames)
: m_start(start)
, m_step(step)
, m_numTimes(numTimes)
{
    // NOTE sweet : the order here is important because we want to crop dropped frames.
    m_cropped = sortAndCompactIndexRanges(cropped);
    cropSortAndSetBlankFrames(blankFrames);
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
    Ratio duration = m_step * m_numTimes;

    if (secsFromStart.toDouble() <= 0)
    {
        return 0;
    }
    else if (secsFromStart.toDouble() >= duration.toDouble())
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
        && (m_cropped == other.m_cropped)
        && (m_blankFrames == other.m_blankFrames);
}

void
TimingInfo::serialize(std::ostream& strm) const
{
    strm << "TimingInfo("
            << "Start=" << m_start << ", "
            << "Step=" << m_step << ", "
            << "NumTimes=" << m_numTimes << ", "
            << "DroppedFrames=[";
    
    for (size_t i = 0; i < m_droppedFrames.size(); i++)
    {
        strm << m_droppedFrames[i];
        if (i < (m_droppedFrames.size() - 1))
        {
            strm << ", ";
        }
    }
    
    strm << "], CroppedFrames=[";
    for (size_t i = 0; i < m_cropped.size(); i++)
    {
        strm << m_cropped[i];
        if (i < (m_cropped.size() - 1))
        {
            strm << ", ";
        }
    }

    strm << "], BlankFrames=[";
    for (size_t i = 0; i < m_blankFrames.size(); i++)
    {
        strm << m_blankFrames[i];
        if (i < (m_blankFrames.size() - 1))
        {
            strm << ", ";
        }
    }
    strm << "]";

    strm << ")";
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

const std::vector<isize_t> &
TimingInfo::getBlankFrames() const
{
    return m_blankFrames;
}

isize_t
TimingInfo::getBlankCount() const
{
    return m_blankFrames.size();
}

bool
TimingInfo::isBlank(isize_t inIndex) const
{
    auto & df = m_blankFrames;
    return std::binary_search(df.begin(), df.end(), inIndex);
}

bool
TimingInfo::isIndexValid(const isize_t inIndex) const
{
    return !(isCropped(inIndex) || isDropped(inIndex) || isBlank(inIndex));
}

isize_t
TimingInfo::getNumValidTimes() const
{
    ISX_ASSERT((int64_t(m_numTimes) - int64_t(getDroppedCount()) - int64_t(getCroppedCount()) - int64_t(getBlankCount())) >= 0);
    return m_numTimes - getDroppedCount() - getCroppedCount() - getBlankCount();
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
    isize_t numDroppedBefore = it - m_droppedFrames.begin();

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

    // Note that this internally asserts that the input index is not blank.
    it = std::find_if(
        m_blankFrames.begin(),
        m_blankFrames.end(),
        [&inIndex](const isize_t &index)
        {
            ISX_ASSERT(index != inIndex);
            return index > inIndex;
        }
    );
    isize_t numBlankBefore = it - m_blankFrames.begin();

    ISX_ASSERT(inIndex >= (numDroppedBefore + numCroppedBefore + numBlankBefore));
    isize_t recordedIdx = inIndex - numDroppedBefore - numCroppedBefore - numBlankBefore;

    return recordedIdx;
}


TimingInfo
TimingInfo::getDefault(isize_t inFrames, const std::vector<isize_t> & inDroppedFrames)
{
    Time start;                       // Default to Unix epoch
    return TimingInfo(start, s_defaultStep, inFrames, inDroppedFrames);
}

void
TimingInfo::cropSortAndSetBlankFrames(const std::vector<isize_t> & inBlankFrames)
{
    m_blankFrames.clear();
    for (const auto df : inBlankFrames)
    {
        if (!isCropped(df))
        {
            m_blankFrames.push_back(df);
        }
    }
    std::sort(m_blankFrames.begin(), m_blankFrames.end());
}

void
TimingInfo::cropSortAndSetDroppedFrames(const std::vector<isize_t> & inDroppedFrames)
{
    m_droppedFrames.clear();
    for (const auto df : inDroppedFrames)
    {
        if (!isCropped(df) && !isBlank(df))
        {
            m_droppedFrames.push_back(df);
        }
    }
    std::sort(m_droppedFrames.begin(), m_droppedFrames.end());
}

std::pair<isize_t, isize_t>
getSegmentAndLocalIndex(const TimingInfos_t & inTis, const isize_t inGlobalIndex)
{
    const isize_t segmentIndex = getSegmentIndex(inTis, inGlobalIndex);
    ISX_ASSERT(segmentIndex < inTis.size());
    isize_t localIndex = inGlobalIndex;
    for (isize_t s = 0; s < segmentIndex; ++s)
    {
        const isize_t numTimes = inTis.at(s).getNumTimes();
        if (numTimes >= localIndex)
        {
            localIndex = 0;
            break;
        }
        ISX_ASSERT(localIndex >= numTimes);
        localIndex -= numTimes;
    }
    return std::pair<isize_t, isize_t>(segmentIndex, localIndex);
}

std::pair<isize_t, isize_t>
getSegmentAndLocalIndex(const TimingInfos_t & inTis, const Time & inTime)
{
    const isize_t segmentIndex = getSegmentIndex(inTis, inTime);
    ISX_ASSERT(segmentIndex < inTis.size());
    const isize_t localIndex = inTis.at(segmentIndex).convertTimeToIndex(inTime);
    return std::pair<isize_t, isize_t>(segmentIndex, localIndex);
}

isize_t
getSegmentIndex(const TimingInfos_t & inTis, const isize_t inGlobalIndex)
{
    isize_t segmentIndex = 0;
    const isize_t numSegments = inTis.size();
    ISX_ASSERT(numSegments > 0);
    isize_t totalNumTimes = 0;
    for (; segmentIndex < numSegments - 1; ++segmentIndex)
    {
        const isize_t numTimes = inTis.at(segmentIndex).getNumTimes();
        totalNumTimes += numTimes;
        if (inGlobalIndex < totalNumTimes)
        {
            break;
        }
    }
    return segmentIndex;
}

isize_t
getSegmentIndex(const TimingInfos_t & inTis, const Time & inTime)
{
    isize_t segmentIndex = 0;
    const isize_t numSegments = inTis.size();
    ISX_ASSERT(numSegments > 0);
    for (; segmentIndex < numSegments - 1; ++segmentIndex)
    {
        if (inTime < inTis.at(segmentIndex).getEnd())
        {
            break;
        }
    }
    return segmentIndex;
}

isize_t
getGlobalIndex(const TimingInfos_t & inTis, const std::pair<isize_t, isize_t> & inSegmentLocal)
{
    isize_t globalIndex = inSegmentLocal.second;
    for (isize_t s = 0; s < inSegmentLocal.first; ++s)
    {
        globalIndex += inTis.at(s).getNumTimes();
    }
    return globalIndex;
}

isize_t
getGlobalIndex(const TimingInfos_t & inTis, const Time & inTime)
{
    return getGlobalIndex(inTis, getSegmentAndLocalIndex(inTis, inTime));
}

isize_t
getTotalNumTimes(const TimingInfos_t & inTis)
{
    isize_t numTimes = 0;
    for (const auto & ti : inTis)
    {
        numTimes += ti.getNumTimes();
    }
    return numTimes;
}

DurationInSeconds
getTotalDuration(const TimingInfos_t & inTis)
{
    DurationInSeconds totalDuration;
    for (const auto & ti : inTis)
    {
        const DurationInSeconds simplifiedDuration = DurationInSeconds::fromMicroseconds(
            int64_t((ti.getStep() * ti.getNumTimes() * int64_t(1e6)).toDouble())
        );
        totalDuration += simplifiedDuration;
    }
    return totalDuration;
}

DurationInSeconds
getGaplessDurationSinceStart(const TimingInfos_t & inTis, const isize_t inSegmentIndex, const isize_t inLocalIndex)
{
    ISX_ASSERT(inSegmentIndex < inTis.size());
    ISX_ASSERT(inLocalIndex <= inTis.at(inSegmentIndex).getNumTimes());
    DurationInSeconds d;
    for (isize_t s = 0; s < inSegmentIndex; ++s)
    {
        const DurationInSeconds simplifiedDuration = DurationInSeconds::fromMicroseconds(
            int64_t((inTis.at(s).getStep() * inTis.at(s).getNumTimes() * int64_t(1e6)).toDouble())
        );
        d += simplifiedDuration;
    }
    const DurationInSeconds simplifiedStep = DurationInSeconds::fromMicroseconds(
        int64_t((inTis.at(inSegmentIndex).getStep() * int64_t(1e6)).toDouble())
    );
    d += simplifiedStep * inLocalIndex;
    return d;
}

std::vector<IndexRanges_t>
convertGlobalRangesToLocalRanges(
        const TimingInfos_t & inLocalTis,
        const IndexRanges_t & inGlobalRanges)
{
    const size_t numMovies = inLocalTis.size();
    std::vector<IndexRanges_t> outLocalRanges(numMovies);
    for (const auto & r : inGlobalRanges)
    {
        const std::pair<isize_t, isize_t> firstPair = getSegmentAndLocalIndex(inLocalTis, r.m_first);
        const std::pair<isize_t, isize_t> lastPair = getSegmentAndLocalIndex(inLocalTis, r.m_last);

        // Unpack the pairs into more readable indices
        isize_t firstSegment = firstPair.first;
        isize_t firstSample = firstPair.second;
        isize_t lastSegment = lastPair.first;
        isize_t lastSample = lastPair.second;

        // First index is after end of last segment, or the first sample
        // comes after the end of the last segment, so we can just skip it.
        if (firstSegment >= numMovies)
        {
            continue;
        }

        const isize_t firstSegmentNumTimes = inLocalTis[firstSegment].getNumTimes();
        if (firstSegment == (numMovies - 1) && firstSample >= firstSegmentNumTimes)
        {
            continue;
        }

        // If the last segment is past the end of all movies then roll it
        // back to the end of the last one.
        if (lastSegment >= numMovies)
        {
            lastSegment = numMovies - 1;
            lastSample = inLocalTis[lastSegment].getNumTimes() - 1;
        }

        // The simple case is when the first/last indices are in the same segment.
        if (firstSegment == lastSegment)
        {
            outLocalRanges[firstSegment].push_back(IndexRange(firstSample, lastSample));
            continue;
        }

        // Otherwise, we must keep adding ranges until we hit the adjusted last segment/sample
        for (isize_t s = firstSegment; s <= lastSegment; ++s)
        {
            // Starts here, but doesn't end here, so add the rest of this segment and continue.
            if (s == firstSegment)
            {
                outLocalRanges[s].push_back(IndexRange(firstSample, inLocalTis[s].getNumTimes() - 1));
            }
            // Doesn't start here, but does end here so add from start of this segment and finish.
            else if (s == lastSegment)
            {
                outLocalRanges[s].push_back(IndexRange(0, lastSample));
                break;
            }
            // Doesn't start here and hasn't ended yet, so add everything and continue.
            else
            {
                outLocalRanges[s].push_back(IndexRange(0, inLocalTis[s].getNumTimes() - 1));
            }
        }
    }
    return outLocalRanges;
}

std::vector<IndexRanges_t>
convertGlobalRangesToLocalRanges(
        const std::vector<SpMovie_t> inMovies,
        const IndexRanges_t & inGlobalRanges)
{
    std::vector<std::string> movieFileNames;
    TimingInfos_t localTis;
    for (const auto & m : inMovies)
    {
        movieFileNames.push_back(m->getFileName());
        localTis.push_back(m->getTimingInfo());
    }
    return convertGlobalRangesToLocalRanges(localTis, inGlobalRanges);
}

} // namespace isx
