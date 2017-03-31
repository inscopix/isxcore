#include "isxMovieSeries.h"
#include "isxMovieFactory.h"
#include "isxException.h"
#include "isxDispatchQueue.h"
#include "isxSeries.h"
#include "isxIoTaskTracker.h"

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
    const auto & refSi = m_movies[0]->getSpacingInfo();
    const DataType refDataType = m_movies[0]->getDataType();
    std::string errorMessage;
    for (isize_t i = 1; i < m_movies.size(); ++i)
    {
        const auto & m = m_movies[i];
        if (!Series::checkDataType(refDataType, m->getDataType(), errorMessage))
        {
            ISX_THROW(ExceptionSeries, errorMessage);
        }
        if (!Series::checkSpacingInfo(refSi, m->getSpacingInfo(), errorMessage))
        {
            ISX_THROW(ExceptionSeries, errorMessage);
        }

        const auto & tip = m_movies[i-1]->getTimingInfo();
        const auto & tic = m_movies[i]->getTimingInfo();
        if (!Series::checkTimingInfo(tip, tic, errorMessage))
        {
            ISX_THROW(ExceptionSeries, errorMessage);
        }
    }

    // individual movie files are compatible, initialize
    m_spacingInfo = m_movies[0]->getSpacingInfo();
    const auto start = m_movies[0]->getTimingInfo().getStart();
    const auto end   = m_movies.back()->getTimingInfo().getEnd();
    const auto step  = m_movies[0]->getTimingInfo().getStep();

    // calculate each individual movie's first frame in global (MovieSeries') frame indexes
    for (const auto & m: m_movies)
    {
        const auto secsFromStart = m->getTimingInfo().getStart() - start;
        const auto ffIndexD = std::ceil((secsFromStart / step).toDouble());
        const auto ffIndex = isize_t(ffIndexD);
        m_moviesFirstFrameInGlobal.push_back(ffIndex);
    }

    m_globalTimingInfo = TimingInfo(start, step,
        m_moviesFirstFrameInGlobal.back() + m_movies.back()->getTimingInfo().getNumTimes());
    
    // each individual movie's TimingInfo
    for (isize_t i = 0; i < m_movies.size(); ++i)
    {
        const auto startInGlobal = m_globalTimingInfo.convertIndexToStartTime(m_moviesFirstFrameInGlobal[i]);
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
        ISX_THROW(isx::ExceptionDataIO,
                  "The index of the frame (", inFrameNumber, ") is out of range (0-",
                  m_moviesFirstFrameInGlobal.back() + m_movies.back()->getTimingInfo().getNumTimes(), ").");
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
        ret->moveCompatibleImage(f->getImage());
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
        ISX_THROW(isx::ExceptionDataIO,
                  "The index of the frame (", inFrameNumber, ") is out of range (0-",
                  m_moviesFirstFrameInGlobal.back() + m_movies.back()->getTimingInfo().getNumTimes(), ").");
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
                    ret->moveCompatibleImage(inAsyncTaskResult.get()->getImage());
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
