#include "isxSeriesUtils.h"

namespace isx
{

TimingInfo
makeGlobalTimingInfo(const TimingInfos_t & inTis)
{
    const Time start = inTis.front().getStart();
    const DurationInSeconds step = inTis.front().getStep();
    const Time end = inTis.back().getEnd();
    const isize_t totalNumTimes = isize_t(DurationInSeconds(end - start).toDouble() / step.toDouble());
    return TimingInfo(start, step, totalNumTimes);
}

TimingInfo
makeGaplessTimingInfo(const TimingInfos_t & inTis)
{
    isize_t totalNumTimes = 0;
    for (const auto & ti : inTis)
    {
        totalNumTimes += ti.getNumTimes();
    }
    const TimingInfo first = inTis.front();
    return TimingInfo(first.getStart(), first.getStep(), totalNumTimes);
}

} // namespace isx
