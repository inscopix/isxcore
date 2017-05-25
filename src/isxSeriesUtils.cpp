#include "isxSeriesUtils.h"
#include "isxGpio.h"
#include "isxException.h"
#include "isxSeries.h"

#include "isxSpacingInfo.h"
#include "isxMovie.h"
#include "isxGpio.h"
#include "isxCellSet.h"

namespace isx
{

bool
checkSeriesDataSetType(
        const DataSet::Type inRef,
        const DataSet::Type inNew,
        std::string & outMessage)
{
    if (inRef != inNew)
    {
        outMessage = "The DataSet type is different from that of the reference.";
        return false;
    }
    return true;
}

bool
checkSeriesDataType(
        const DataType inRef,
        const DataType inNew,
        std::string & outMessage)
{
    if (inRef != inNew)
    {
        outMessage = "The data type is different from that of the reference.";
        return false;
    }
    return true;
}

bool
checkSeriesTimingInfo(
        const TimingInfo & inRef,
        const TimingInfo & inNew,
        std::string & outMessage)
{
    if (inNew.getStep() != inRef.getStep())
    {
        outMessage = "The timing info has a different frame rate than the reference.";
        return false;
    }
    if (inNew.overlapsWith(inRef))
    {
        outMessage = "The timing info temporally overlaps with the reference.";
        return false;
    }
    return true;
}

bool
checkSeriesHistory(
        const HistoricalDetails & inRef,
        const HistoricalDetails & inNew,
        std::string & outMessage)
{
    if (inRef != inNew)
    {
        outMessage = "The history details are different than those of the reference.";
        return false;
    }
    return true;
}

bool
checkSeriesSpacingInfo(
        const SpacingInfo & inRef,
        const SpacingInfo & inNew,
        std::string & outMessage)
{
    if (!(inRef == inNew))
    {
        outMessage = "The spacing info is different than that of the reference.";
        return false;
    }
    return true;
}

bool
checkNewMemberOfSeries(
        const std::vector<SpMovie_t> & inExisting,
        const SpMovie_t & inNew,
        std::string & outMessage)
{
    const DataType newDataType = inNew->getDataType();
    const SpacingInfo & newSi = inNew->getSpacingInfo();
    const TimingInfo & newTi = inNew->getTimingInfo();
    for (const auto & e : inExisting)
    {
        if (!checkSeriesDataType(e->getDataType(), newDataType, outMessage) ||
            !checkSeriesSpacingInfo(e->getSpacingInfo(), newSi, outMessage) ||
            !checkSeriesTimingInfo(e->getTimingInfo(), newTi, outMessage))
        {
            return false;
        }
    }
    return true;
}

bool
checkNewMemberOfSeries(
        const std::vector<SpCellSet_t> & inExisting,
        const SpCellSet_t & inNew,
        std::string & outMessage)
{
    const isize_t newNumCells = inNew->getNumCells();
    const SpacingInfo & newSi = inNew->getSpacingInfo();
    const TimingInfo & newTi = inNew->getTimingInfo();
    for (const auto & e : inExisting)
    {
        if (e->getNumCells() != newNumCells)
        {
            outMessage = "CellSet series member with mismatching number of cells.";
            return false;
        }

        if (!checkSeriesSpacingInfo(e->getSpacingInfo(), newSi, outMessage) ||
            !checkSeriesTimingInfo(e->getTimingInfo(), newTi, outMessage))
        {
            return false;
        }
    }
    return true;
}

bool
checkNewMemberOfSeries(
        const std::vector<SpGpio_t> & inExisting,
        const SpGpio_t & inNew,
        std::string & outMessage)
{
    const bool newIsAnalog = inNew->isAnalog();
    const std::vector<std::string> & newChannels = inNew->getChannelList();
    const TimingInfo & newTi = inNew->getTimingInfo();
    for (const auto & e : inExisting)
    {
        if (e->isAnalog() != newIsAnalog)
        {
            outMessage = "GPIO series member with mismatching analog/digital data.";
            return false;
        }

        if (e->getChannelList() != newChannels)
        {
            outMessage = "GPIO series member with mismatching channels.";
            return false;
        }

        if (!checkSeriesTimingInfo(e->getTimingInfo(), newTi, outMessage))
        {
            return false;
        }
    }
    return true;
}

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
