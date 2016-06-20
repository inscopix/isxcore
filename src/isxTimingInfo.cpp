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
    double index = std::floor((secsFromStart / m_step).toDouble());

    if (index < 0)
    {
        return 0;
    }
    else if (index >= m_numTimes)
    {
        return m_numTimes - 1;
    }
    else
    {
        return static_cast<uint32_t>(index);
    }
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
