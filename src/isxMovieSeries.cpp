#include "isxMovieSeries.h"
#include "isxMovieFactory.h"
#include "isxException.h"
#include "isxDispatchQueue.h"
#include "isxSeries.h"
#include "isxIoTaskTracker.h"
#include "isxSeriesUtils.h"

#include <algorithm>
#include <functional>
#include <cmath>
#include <cstring>

namespace isx
{
MovieSeries::MovieSeries()
{}

MovieSeries::MovieSeries(const std::vector<std::string> & inFileNames)
    : m_ioTaskTracker(new IoTaskTracker<VideoFrame>())
{
    ISX_ASSERT(inFileNames.size() > 0);
    if (inFileNames.size() == 0)
    {
        return;
    }
    
    for (const auto & fn: inFileNames)
    {
        m_movies.emplace_back(readMovie(fn));
    }
    std::sort(m_movies.begin(), m_movies.end(), [](SpMovie_t a, SpMovie_t b)
        {
            return a->getTimingInfo().getStart() < b->getTimingInfo().getStart();
        });

    // movies are sorted by start time now, check if they meet requirements
    std::string errorMessage;
    for (isize_t i = 1; i < m_movies.size(); ++i)
    {
        if (!checkNewMemberOfSeries({m_movies[i - 1]}, m_movies[i], errorMessage))
        {
            ISX_THROW(ExceptionSeries, errorMessage);
        }
    }

    // individual movie files are compatible, initialize
    m_spacingInfo = m_movies[0]->getSpacingInfo();
    const auto step  = m_movies[0]->getTimingInfo().getStep();
    const auto start = Time(m_movies[0]->getTimingInfo().getStart().getSecsSinceEpoch().expandWithDenomOf(step));
    const auto end = Time(m_movies.back()->getTimingInfo().getEnd().getSecsSinceEpoch());
    const auto duration = end - start;
    const auto numTimesRatio = duration / step;
    const auto numTimes = isize_t(std::floor(numTimesRatio.toDouble()));

    m_globalTimingInfo = TimingInfo(start, step, numTimes);

    // each individual movie's TimingInfo
    for (isize_t i = 0; i < m_movies.size(); ++i)
    {
        const auto startInGlobal = Time(m_movies[i]->getTimingInfo().getStart().getSecsSinceEpoch().expandWithDenomOf(step));
        const auto numTimes = m_movies[i]->getTimingInfo().getNumTimes();
        m_timingInfos.push_back(TimingInfo(startInGlobal, step, numTimes));
    }

    m_valid = true;
}

const
std::vector<SpMovie_t> &
MovieSeries::getMovies()
const
{
    return m_movies;
}

bool
MovieSeries::isValid()
const
{
    return m_valid;
}

   
SpVideoFrame_t
MovieSeries::getFrame(isize_t inFrameNumber)
{
    size_t movieIndex = 0;
    size_t frameIndex = 0;
    std::tie(movieIndex, frameIndex) = getSegmentIndexAndSampleIndexFromGlobalSampleIndex(m_globalTimingInfo, m_timingInfos, inFrameNumber);
    auto ret = makeVideoFrameInternal(inFrameNumber);
    if (movieIndex == m_movies.size()) 
    {
        auto lastFrame = m_globalTimingInfo.convertTimeToIndex(m_globalTimingInfo.getEnd());
        ISX_THROW(isx::ExceptionDataIO,
                  "The index of the frame (", inFrameNumber, ") is out of range (0-", lastFrame, ").");
    }
    else if (frameIndex >= m_movies[movieIndex]->getTimingInfo().getNumTimes()) 
    {
        // in between individual movies
        // --> return placeholder frame
        ret->setFrameType(VideoFrame::Type::INGAP);
    }
    else
    {
        auto f = m_movies[movieIndex]->getFrame(frameIndex);
        ret->moveFrameContent(f);
    }
    return ret;
}

void
MovieSeries::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    size_t movieIndex = 0;
    size_t frameIndex = 0;
    std::tie(movieIndex, frameIndex) = getSegmentIndexAndSampleIndexFromGlobalSampleIndex(m_globalTimingInfo, m_timingInfos, inFrameNumber);
    auto ret = makeVideoFrameInternal(inFrameNumber);
    if (movieIndex == m_movies.size()) 
    {
        auto lastFrame = m_globalTimingInfo.convertTimeToIndex(m_globalTimingInfo.getEnd());
        ISX_THROW(isx::ExceptionDataIO,
                  "The index of the frame (", inFrameNumber, ") is out of range (0-", lastFrame, ").");
    }
    else if (frameIndex >= m_movies[movieIndex]->getTimingInfo().getNumTimes()) 
    {
        // in between individual movies
        // --> return placeholder frame
        ret->setFrameType(VideoFrame::Type::INGAP);
        
        GetFrameCB_t getFrameCB = [ret, inFrameNumber]()
        {
            return ret;
        };
        m_ioTaskTracker->schedule(getFrameCB, inCallback);
    }
    else
    {
        m_movies[movieIndex]->getFrameAsync(frameIndex, [ret, inCallback](AsyncTaskResult<SpVideoFrame_t> inAsyncTaskResult)
            {
                AsyncTaskResult<SpVideoFrame_t> atr;
                if (!inAsyncTaskResult.getException() && inAsyncTaskResult.get())
                {
                    atr.setValue(ret);
                    ret->moveFrameContent(inAsyncTaskResult.get());
                }
                else
                {
                    atr.setException(inAsyncTaskResult.getException());
                }
                inCallback(atr);
            });
    }
}

void
MovieSeries::cancelPendingReads()
{
    m_ioTaskTracker->cancelPendingTasks();
    for (auto & m: m_movies)
    {
        m->cancelPendingReads();
    }
}
    
SpVideoFrame_t
MovieSeries::makeVideoFrameInternal(isize_t inIndex) const
{
    const SpacingInfo spacingInfo = getSpacingInfo();
    const DataType dataType = getDataType();
    const isize_t pixelSizeInBytes = getDataTypeSizeInBytes(dataType);
    const isize_t rowSizeInBytes = pixelSizeInBytes * spacingInfo.getNumColumns();
    SpVideoFrame_t outFrame = std::make_shared<VideoFrame>(
        spacingInfo,
        rowSizeInBytes,
        1,
        dataType,
        getTimingInfo().convertIndexToStartTime(inIndex),
        inIndex);
    return outFrame;
}

const TimingInfo &
MovieSeries::getTimingInfo() const
{
    return m_globalTimingInfo;
}

const TimingInfos_t &
MovieSeries::getTimingInfosForSeries() const
{
    return m_timingInfos;
}

const SpacingInfo &
MovieSeries::getSpacingInfo() const
{
    return m_spacingInfo;
}

DataType
MovieSeries::getDataType() const
{
    return m_movies[0]->getDataType();
}

std::string
MovieSeries::getFileName() const
{
    return "**MovieSeries";
}

void
MovieSeries::serialize(std::ostream & strm) const
{
    strm << getFileName();
}

} // namespace isx
