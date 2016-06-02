#include "isxTimingInfo.h"
#include <cmath>

namespace isx
{

TimingInfo::TimingInfo()
: m_start(isx::Time())
, m_step(isx::Ratio(50, 1000))
, m_numTimes(100)
{
}

TimingInfo::TimingInfo(const isx::Time& start, const isx::Ratio& step, uint64_t numTimes)
: m_start(start)
, m_step(step)
, m_numTimes(numTimes)
{
}

isx::Time
TimingInfo::getStart() const
{
    return m_start;
}

isx::Time
TimingInfo::getEnd() const
{
    return m_start.addSecs(getDuration());
}

isx::Ratio
TimingInfo::getStep() const
{
    return m_step;
}

uint64_t
TimingInfo::getNumTimes() const
{
    return m_numTimes;
}

isx::Ratio
TimingInfo::getDuration() const
{
    return m_step * m_numTimes;
}

uint64_t
TimingInfo::convertTimeToIndex(const isx::Time& inTime) const
{
    isx::Ratio secsFromStart = inTime.secsFrom(m_start);
    isx::Ratio indexRatio = (secsFromStart / m_step) - isx::Ratio(1, 2);
    long index = std::lround(indexRatio.toDouble());

    if (index < 0)
    {
        index = 0;
    }
    else if (index >= m_numTimes)
    {
        index = m_numTimes - 1;
    }

    return uint64_t(index);
}

} // namespace
