#include "isxLogicalTrace.h"
#include "isxLog.h"
#include "isxAssert.h"

namespace
{

bool
contains(const isx::TimingInfo & inTi, const isx::Time & inTime)
{
    return (inTime >= inTi.getStart()) && (inTime < inTi.getEnd());
}

} // namespace

namespace isx
{

void
getCoordinatesFromLogicalTrace(
        const SpLogicalTrace_t & inTrace,
        const TimingInfos_t & inTis,
        const bool inCoordsForSquareWave,
        std::vector<std::vector<double>> & outX,
        std::vector<std::vector<double>> & outY)
{
    ISX_ASSERT(inTrace);
    ISX_ASSERT(inTis.size() > 0);

    std::vector<double> durationOfPrevSegments(inTis.size());
    durationOfPrevSegments[0] = 0.0;
    for (isize_t i(1); i < inTis.size(); ++i)
    {
        durationOfPrevSegments[i] = durationOfPrevSegments[i-1] + inTis[i-1].getDuration().toDouble();
    }

    const std::map<Time, float> & values = inTrace->getValues();

    outX = std::vector<std::vector<double>>(inTis.size());
    outY = std::vector<std::vector<double>>(inTis.size());
    isize_t segmentIdx = 0;
    if (inCoordsForSquareWave)
    {
        outX.at(segmentIdx).push_back(durationOfPrevSegments[segmentIdx]);
        outY.at(segmentIdx).push_back(0.0);
    }
    double startTimeForSegment = inTis.at(segmentIdx).getStart().getSecsSinceEpoch().toDouble();

    for (auto & pair : values)
    {
        const Time & time = pair.first;
        if (!contains(inTis.at(segmentIdx), time))
        {
            while (!contains(inTis.at(segmentIdx), time))
            {
                ++segmentIdx;
                if (segmentIdx >= inTis.size())
                {
                    return;
                }
                startTimeForSegment = inTis.at(segmentIdx).getStart().getSecsSinceEpoch().toDouble();
            }

            // GPIO requires at least two points to have a graph plotted
            // Make sure each segment starts with zero
            if (inCoordsForSquareWave)
            {
                outX.at(segmentIdx).push_back(durationOfPrevSegments[segmentIdx]);
                outY.at(segmentIdx).push_back(0.0);
            }
        }
        outX.at(segmentIdx).push_back(time.getSecsSinceEpoch().toDouble() - startTimeForSegment + durationOfPrevSegments[segmentIdx]);
        outY.at(segmentIdx).push_back(double(pair.second));
    }

    // This should ensure that we assume the same value until the end of each segment.
    // TODO : The x-coordinates for the separators and plot points can be slightly different
    // due to slightly different arithmetic. In the future, it would be nice if these were shared.
    if (inCoordsForSquareWave)
    {
        for (size_t i = 0; i < outX.size(); ++i)
        {
            if (!(outX.at(i).empty()) && !(outY.at(i).empty()))
            {
                const double lastTime = inTis.at(i).getLastStartTime().getSecsSinceEpoch().toDouble();
                const double startTime = inTis.at(i).getStart().getSecsSinceEpoch().toDouble();
                outX.at(i).push_back(lastTime - startTime + durationOfPrevSegments.at(i));
                outY.at(i).push_back(outY.at(i).back());
            }
        }
    }
}

} // namespace isx
