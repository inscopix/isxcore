#include "isxSeriesUtils.h"
#include "isxGpio.h"
#include "isxException.h"
#include "isxSeries.h"

#include "isxSpacingInfo.h"
#include "isxMovie.h"
#include "isxGpio.h"
#include "isxCellSet.h"
#include "isxEvents.h"

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
    if (inRef != inNew)
    {
        if (!(inRef.getPixelSize() == inNew.getPixelSize()))
        {
            outMessage = "The new data set has a different pixel size than the rest of the series. If this data was recorded using an nVista 2.0 or nVoke 1.0 please use IDPS version 1.3.0 for this step.\n";
            outMessage += "If you are adding a TIFF file, note that IDPS does not currently support combining downsampled TIFF files to a series.";
            return false;
        }

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
    const std::vector<std::string> & newChannels = inNew->getChannelList();
    
    for (const auto & e : inExisting)
    {
        if (e->getChannelList() != newChannels)
        {
            outMessage = "GPIO series member with mismatching channels.";
            return false;
        }

        for (auto & n : newChannels)
        {
            const bool newIsAnalog = inNew->isAnalog(n);
            const TimingInfo & newTi = inNew->getTimingInfo(n);

            if (e->isAnalog(n) != newIsAnalog)
            {
                outMessage = "GPIO series member with mismatching data types.";
                return false;
            }        

            if (!checkSeriesTimingInfo(e->getTimingInfo(n), newTi, outMessage))
            {
                return false;
            }
        }
        
    }
    return true;
}

bool
checkNewMemberOfSeries(
        const std::vector<SpEvents_t> & inExisting,
        const SpEvents_t & inNew,
        std::string & outMessage)
{
    const std::vector<std::string> & newNames = inNew->getCellNamesList();
    const TimingInfo & newTi = inNew->getTimingInfo();
    for (const auto & e : inExisting)
    {
        if (e->getCellNamesList() != newNames)
        {
            outMessage = "Events series member with mismatching channels.";
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

std::string
makeSeriesFilePathString(const std::string & inType, const std::vector<std::string> & inFilePaths)
{
    std::stringstream stream;
    stream << inType << "(";
    for (size_t i = 0; i < inFilePaths.size(); ++i)
    {
        stream << inFilePaths.at(i);
        if (i < (inFilePaths.size() - 1))
        {
            stream << ", ";
        }
    }
    stream << ")";
    return stream.str();
}

} // namespace isx
