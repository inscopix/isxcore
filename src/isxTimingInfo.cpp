#include "isxTimingInfo.h"

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

} // namespace
