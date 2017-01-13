#include "isxGaplessMovieSeries.h"
#include "isxSeries.h"
#include "isxMovieFactory.h"
#include "isxException.h"

#include <algorithm>
#include <functional>

namespace isx
{
GaplessMovieSeries::GaplessMovieSeries()
{}

GaplessMovieSeries::GaplessMovieSeries(const std::vector<std::string> & inFileNames)
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

    isize_t totalNumTimes = m_movies[0]->getTimingInfo().getNumTimes();

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

        totalNumTimes += tic.getNumTimes();
    }
    
    // individual movie files are compatible, initialize
    m_spacingInfo = m_movies[0]->getSpacingInfo();

    // this create a TimingInfo object with 0 frame duration, but one
    // that still tracks number of frames. Operations that don't care
    // about time (a lot of algos) will be fine to just treat this movie
    // series like a regular movie.
    Time start = m_movies[0]->getTimingInfo().getStart();
    m_timingInfo = TimingInfo(start, DurationInSeconds(), totalNumTimes);
    ISX_ASSERT(m_timingInfo.getStart() == m_timingInfo.getEnd());

    m_valid = true;
}

const
std::vector<SpMovie_t> &
GaplessMovieSeries::getMovies()
const
{
    return m_movies;
}

bool
GaplessMovieSeries::isValid()
const
{
    return m_valid;
}

std::pair<isize_t, isize_t>
GaplessMovieSeries::getMovieIndexAndFrameIndexFromGlobalFrameIndex(isize_t inFrameIndex) const
{
    isize_t fn = inFrameIndex;
    for (isize_t i = 0; i < m_movies.size(); ++i)
    {
        isize_t nt = m_movies[i]->getTimingInfo().getNumTimes();
        if (fn <= nt)
        {
            return std::make_pair(i, fn);
        }
        fn -= nt;
    }
    return std::make_pair(m_movies.size(), 0);
}
   
SpVideoFrame_t
GaplessMovieSeries::getFrame(isize_t inFrameNumber)
{
    size_t movieIndex = 0;
    size_t frameIndex = 0;
    std::tie(movieIndex, frameIndex) = getMovieIndexAndFrameIndexFromGlobalFrameIndex(inFrameNumber);
    if (movieIndex >= m_movies.size())
    {
        ISX_ASSERT(false, "GaplessMovieSeries::getFrame reuqested frame out of range.");
        return nullptr;
    }
    return m_movies[movieIndex]->getFrame(frameIndex);
}

void
GaplessMovieSeries::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    size_t movieIndex = 0;
    size_t frameIndex = 0;
    std::tie(movieIndex, frameIndex) = getMovieIndexAndFrameIndexFromGlobalFrameIndex(inFrameNumber);
    if (movieIndex >= m_movies.size())
    {
        ISX_ASSERT(false, "GaplessMovieSeries::getFrameAsync reuqested frame out of range.");
        return;
    }
    m_movies[movieIndex]->getFrameAsync(frameIndex, inCallback);
}

void
GaplessMovieSeries::cancelPendingReads()
{
    for (auto & m: m_movies)
    {
        m->cancelPendingReads();
    }
}
    
void
GaplessMovieSeries::writeFrame(const SpVideoFrame_t & inVideoFrame)
{
    ISX_ASSERT(false, "GaplessMovieSeries::writeFrame not implemented");
}
    
SpVideoFrame_t
GaplessMovieSeries::makeVideoFrame(isize_t inIndex)
{
    ISX_ASSERT(false, "GaplessMovieSeries::makeVideoFrame not implemented");
    return nullptr;
}

const TimingInfo &
GaplessMovieSeries::getTimingInfo() const
{
    return m_timingInfo;
}

const SpacingInfo &
GaplessMovieSeries::getSpacingInfo() const
{
    return m_spacingInfo;
}

DataType
GaplessMovieSeries::getDataType() const
{
    return m_movies[0]->getDataType();
}

std::string
GaplessMovieSeries::getFileName() const
{
    ISX_ASSERT(false, "GaplessMovieSeries::getFileName needs to be defined");
    
    return "**GaplessMovieSeries";
}

void
GaplessMovieSeries::serialize(std::ostream & strm) const
{
    strm << getFileName();
}

} // namespace isx
