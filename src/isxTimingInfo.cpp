#include "isxTimingInfo.h"
#include <cmath>

namespace isx
{

TimingInfo::TimingInfo()
: m_start(Time())
, m_step(Ratio(50, 1000))
, m_numTimes(100)
{
}

TimingInfo::TimingInfo(const Time& start, const Ratio& step, isize_t numTimes)
: m_start(start)
, m_step(step)
, m_numTimes(numTimes)
{
}

Time
TimingInfo::getStart() const
{
    return m_start;
}

Time
TimingInfo::getEnd() const
{
    return m_start.addSecs(getDuration());
}

Ratio
TimingInfo::getStep() const
{
    return m_step;
}

isize_t
TimingInfo::getNumTimes() const
{
    return m_numTimes;
}

Ratio
TimingInfo::getDuration() const
{
    return m_step * m_numTimes;
}

isize_t
TimingInfo::convertTimeToIndex(const Time& inTime) const
{
    Ratio secsFromStart = inTime.secsFrom(m_start);
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
        return static_cast<isize_t>(index);
    }
}

Time
TimingInfo::convertIndexToTime(isize_t inIndex) const
{
    isize_t index = inIndex;

    if (index == 0)
    {
        return m_start;
    }
    else if (index >= m_numTimes)
    {
         index = m_numTimes - 1;
    }

    Time ret = m_start.addSecs(Ratio(index, 1) * m_step);
    return ret;
}

bool
TimingInfo::operator ==(const isx::TimingInfo& other) const
{
    return (m_start == other.m_start)
        && (m_step == other.m_step)
        && (m_numTimes == other.m_numTimes);
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

} // namespace
