#include "isxSeriesUtils.h"
#include "isxGpio.h"
#include "isxException.h"
#include "isxSeries.h"

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

void
checkGpioSeriesMembers(const std::vector<SpGpio_t> & inGpios)
{
    const SpGpio_t & refGpio = inGpios.front();

    const isize_t refNumChannels = refGpio->numberOfChannels();
    const bool refIsAnalog = refGpio->isAnalog();
    const std::vector<std::string> refChannelList = refGpio->getChannelList();

    std::string errorMessage;
    for (isize_t i = 1; i < inGpios.size(); ++i)
    {
        const auto & g = inGpios.at(i);

        if (g->isAnalog() != refIsAnalog)
        {
            ISX_THROW(ExceptionSeries, "GPIO series member with mismatching analog/digital data: ", g->getFileName());
        }

        if (g->numberOfChannels() != refNumChannels)
        {
            ISX_THROW(ExceptionSeries, "GPIO series member with mismatching number of logical channels: ", g->getFileName());
        }

        if (g->getChannelList() != refChannelList)
        {
            ISX_THROW(ExceptionSeries, "GPIO series member with mismatching channel names: ", g->getFileName());
        }

        const auto & tip = inGpios.at(i-1)->getTimingInfo();
        const auto & tic = inGpios.at(i)->getTimingInfo();
        if (!Series::checkTimingInfo(tip, tic, errorMessage))
        {
            ISX_THROW(ExceptionSeries, errorMessage);
        }
    }
}

} // namespace isx
