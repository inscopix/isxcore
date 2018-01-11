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

MovieSeries::MovieSeries(const std::vector<std::string> & inFileNames, const std::vector<DataSet::Properties> & inProperties)
    : m_ioTaskTracker(new IoTaskTracker<VideoFrame>())
{
    ISX_ASSERT(inFileNames.size() > 0);
    if (inFileNames.size() == 0)
    {
        return;
    }

    

    if (isBehavioralMovieFileExtension(inFileNames.at(0)))
    {   
        ISX_ASSERT(inFileNames.size() == inProperties.size());     
        isize_t propIndex = 0;
        for (const auto & fn: inFileNames)
        {
            if (!isBehavioralMovieFileExtension(fn))
            {
                ISX_THROW(ExceptionSeries, "Can't mix movie types");
            }
            m_movies.emplace_back(readBehavioralMovie(fn, inProperties.at(propIndex)));
            ++propIndex;
        }
    }
    else
    {
        if (!inProperties.empty())
        {
            ISX_ASSERT(inFileNames.size() == inProperties.size());
        }

        for (isize_t i(0); i < inFileNames.size(); ++i)
        {
            if (!inProperties.empty())
            {
                m_movies.emplace_back(readMovie(inFileNames.at(i), inProperties.at(i)));
            }
            else
            {
                m_movies.emplace_back(readMovie(inFileNames.at(i)));
            }
        }
    }

    std::sort(m_movies.begin(), m_movies.end(), [](SpMovie_t a, SpMovie_t b)
        {
            return a->getTimingInfo().getStart() < b->getTimingInfo().getStart();
        });

    // movies are sorted by start time now, check if they meet requirements
    std::string errorMessage;
    m_timingInfos = {m_movies.front()->getTimingInfo()};
    for (isize_t i = 1; i < m_movies.size(); ++i)
    {
        if (!checkNewMemberOfSeries({m_movies[i - 1]}, m_movies[i], errorMessage))
        {
            ISX_THROW(ExceptionSeries, errorMessage);
        }
        m_timingInfos.push_back(m_movies[i]->getTimingInfo());
    }

    m_spacingInfo = m_movies[0]->getSpacingInfo();

    m_gaplessTimingInfo = makeGaplessTimingInfo(m_timingInfos);

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
    if (inFrameNumber >= m_gaplessTimingInfo.getNumTimes())
    {
        ISX_THROW(ExceptionDataIO, "The index of the frame (", inFrameNumber,
                ") is out of range (0-", m_gaplessTimingInfo.getNumTimes(), ").");
    }

    size_t movieIndex = 0;
    size_t frameIndex = 0;
    std::tie(movieIndex, frameIndex) = getSegmentAndLocalIndex(m_timingInfos, inFrameNumber);

    auto f = m_movies[movieIndex]->getFrame(frameIndex);
    auto ret = makeVideoFrameInternal(inFrameNumber, movieIndex, frameIndex, f->getRowBytes());
    ret->moveFrameContent(f);
    return ret;
}

void
MovieSeries::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    if (inFrameNumber >= m_gaplessTimingInfo.getNumTimes())
    {
        ISX_THROW(ExceptionDataIO, "The index of the frame (", inFrameNumber,
                ") is out of range (0-", m_gaplessTimingInfo.getNumTimes(), ").");
    }

    size_t movieIndex = 0;
    size_t frameIndex = 0;
    std::tie(movieIndex, frameIndex) = getSegmentAndLocalIndex(m_timingInfos, inFrameNumber);

    std::weak_ptr<MovieSeries> weakThis = shared_from_this();
    m_movies[movieIndex]->getFrameAsync(frameIndex, [weakThis, inFrameNumber, movieIndex, frameIndex, inCallback](AsyncTaskResult<SpVideoFrame_t> inAsyncTaskResult)
        {
            AsyncTaskResult<SpVideoFrame_t> atr;
            std::shared_ptr<MovieSeries> sharedThis = weakThis.lock();
            if (!inAsyncTaskResult.getException() && inAsyncTaskResult.get() && sharedThis != nullptr)
            {
                auto ret = sharedThis->makeVideoFrameInternal(inFrameNumber, movieIndex, frameIndex, inAsyncTaskResult.get()->getRowBytes());
                atr.setValue(ret);
                try
                {
                    ret->moveFrameContent(inAsyncTaskResult.get());
                }
                catch (const ExceptionDataIO & error)
                {
                    ISX_LOG_ERROR("Exception thrown in worker thread, will be forwarded to main: ", error.what());
                    std::exception_ptr eptr = std::current_exception();
                    atr.setException(eptr);
                }
            }
            else
            {
                atr.setException(inAsyncTaskResult.getException());
            }
            inCallback(atr);
        });
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
MovieSeries::makeVideoFrameInternal(
        const isize_t inGlobalIndex,
        const isize_t inMovieIndex,
        const isize_t inLocalIndex,
        const isize_t inRowBytes) const
{
    const SpacingInfo spacingInfo = getSpacingInfo();
    const DataType dataType = getDataType();
    SpVideoFrame_t outFrame = std::make_shared<VideoFrame>(
        spacingInfo,
        inRowBytes,
        1,
        dataType,
        m_timingInfos.at(inMovieIndex).convertIndexToStartTime(inLocalIndex),
        inGlobalIndex);
    return outFrame;
}

const TimingInfo &
MovieSeries::getTimingInfo() const
{
    return m_gaplessTimingInfo;
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
