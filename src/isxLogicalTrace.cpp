#include "isxLogicalTrace.h"
#include "isxLog.h"
#include "isxAssert.h"

#include <cmath>

namespace
{

double
toMicrosecondPrecision(const double inSeconds)
{
    return std::round(inSeconds * 1e6) / 1e6;
}

void
addXCoordinate(std::vector<double> & inCoords, const double inX)
{
    inCoords.push_back(toMicrosecondPrecision(inX));
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
    double startTimeForSegment = inTis.at(segmentIdx).getStart().getSecsSinceEpoch().toDouble();

    for (auto & pair : values)
    {
        const Time & time = pair.first;

        // The logic here is a little delicate, but we want to detect a change in the
        // segment without doing silly things as was the case in MOS-1602.
        // Therefore, we keep trying new segments until we get one that ends before
        // the current time, but only add that time if it occurs after the start time.
        // This means we might miss some coordinates around the boundaries, but should
        // not make any really bad mistakes.
        bool newSegment = false;
        while (time > inTis.at(segmentIdx).getEnd())
        {
            newSegment = true;
            ++segmentIdx;
            if (segmentIdx >= inTis.size())
            {
                return;
            }
        }

        if (newSegment)
        {
            startTimeForSegment = inTis.at(segmentIdx).getStart().getSecsSinceEpoch().toDouble();
        }

        if (time >= inTis.at(segmentIdx).getStart())
        {
            addXCoordinate(outX.at(segmentIdx), time.getSecsSinceEpoch().toDouble() - startTimeForSegment + durationOfPrevSegments[segmentIdx]);
            outY.at(segmentIdx).push_back(double(pair.second));
        }
    }

    // This should ensure that we assume the same value until the end of each segment,
    // which means we will definitely have an least 2 points for non-empty channels,
    // which is necessary for a line to be drawn.
    if (inCoordsForSquareWave)
    {
        for (size_t i = 0; i < outX.size(); ++i)
        {
            if (!(outX.at(i).empty()) && !(outY.at(i).empty()))
            {
                const double lastTime = inTis.at(i).getLastStartTime().getSecsSinceEpoch().toDouble();
                const double startTime = inTis.at(i).getStart().getSecsSinceEpoch().toDouble();
                addXCoordinate(outX.at(i), lastTime - startTime + durationOfPrevSegments.at(i));
                outY.at(i).push_back(outY.at(i).back());
            }
        }
    }
}

} // namespace isx
