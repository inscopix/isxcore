#include "isxSeriesUtils.h"
#include "isxGpio.h"
#include "isxException.h"
#include "isxSeries.h"

#include "isxSpacingInfo.h"
#include "isxMovie.h"
#include "isxGpio.h"
#include "isxCellSet.h"

#include <cmath>

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
        outMessage = "Unable to add a data set of type " + DataSet::getTypeString(inNew) + " to a series of type " + DataSet::getTypeString(inRef) + ".";;
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
        outMessage = "Unable to add data of type " + isx::getDataTypeString(inNew) + " to a series with data of type " + isx::getDataTypeString(inRef) +".";
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
    // Note aschildan 6/15/2017:
    // nvista movies can have a huge fluctuation between movies that are (requested) to be recorded with the same
    // framerate.  Michele mentioned one typical case with a large difference where a user requested 20Hz and the
    // recording was made at 20.11Hz. I'm using that as the upper limit to still allow creation of series. I also
    // use a warning epsilon that is lower and we'll log differences larger than the warning epsilon.
    // This will not be an issue when we change the player to work with the framerates of the individual segments
    // of a series, and basically play them as if they were stand-alone data sets.

    // 20Hz vs 20.11Hz is a difference of 0.00027349577325s in frame duration
    const double epsilon = 0.00028; // 0.28ms

    // For the warning epsilon allow up to 0.005ms of difference between frame durations of individual movies.
    // This means over 200000 frames (~2.7 hours at 20Hz) the error can be up to 1s.
    // It is needed for behavioral
    const double warningEpsilon = 0.000005; // 0.005ms
    const double newStep = inNew.getStep().toDouble();
    const double refStep = inRef.getStep().toDouble();

    if (std::abs(newStep - refStep) > epsilon)
    {
        outMessage = "Unable to add a data set with a different frame rate than the rest of the series.";
        return false;
    }
    if (std::abs(newStep - refStep) > warningEpsilon)
    {
        ISX_LOG_WARNING("Creating series from data sets with large difference in framerates.");
    }
    if (inNew.overlapsWith(inRef))
    {
        outMessage = "Unable to insert data that temporally overlaps with other parts of the series. Data sets in a series must all be non-overlapping.";
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
        outMessage = "The new data set has a different processing history than the rest of the series' components. Only unprocessed data sets can be moved into a series.";
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
        outMessage = "The new data set has different spacing information than the rest of the series. Spacing information must be equal among series' components.";
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
makeGaplessTimingInfo(const TimingInfos_t & inTis)
{
    ISX_ASSERT(!inTis.empty());
    const TimingInfo & first = inTis.front();
    return TimingInfo(first.getStart(), first.getStep(), getTotalNumTimes(inTis));
}

} // namespace isx
