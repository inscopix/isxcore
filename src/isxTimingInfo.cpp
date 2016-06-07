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

TimingInfo::TimingInfo(const isx::Time& start, const isx::Ratio& step, uint32_t numTimes)
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

uint32_t
TimingInfo::getNumTimes() const
{
    return m_numTimes;
}

isx::Ratio
TimingInfo::getDuration() const
{
    return m_step * m_numTimes;
}

uint32_t
TimingInfo::convertTimeToIndex(const isx::Time& inTime) const
{
    isx::Ratio secsFromStart = inTime.secsFrom(m_start);
    isx::Ratio indexRatio = (secsFromStart / m_step) - isx::Ratio(1, 2);
    long long indexLongLong = std::llround(indexRatio.toDouble());

    uint32_t index = 0;
    if (indexLongLong < 0)
    {
        index = 0;
    }
    else if (indexLongLong >= m_numTimes)
    {
        index = m_numTimes - 1;
    }
    else
    {
        index = static_cast<uint32_t>(indexLongLong);
    }
    return index;
}

} // namespace
