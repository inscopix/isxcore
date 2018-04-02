#include "isxNVistaTiffMovie.h"
#include "isxMutex.h"
#include "isxIoQueue.h"
#include "isxConditionVariable.h"
#include "isxIoTaskTracker.h"
#include "isxTiffMovie.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <mutex>
#include <memory>
#include <cmath>
#include <cstring>


namespace isx
{

NVistaTiffMovie::NVistaTiffMovie()
    : m_valid(false)
{
}

NVistaTiffMovie::NVistaTiffMovie(
    const std::string & inFileName,
    const std::vector<std::string> &inTiffFileNames,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo, 
    const std::vector<isize_t> & inDroppedFrames,
    const std::map<std::string, Variant> & inAdditionalProperties,
    const std::vector<isize_t> & inNumFrames)
    : m_fileName(inFileName)
    , m_ioTaskTracker(new IoTaskTracker<VideoFrame>())
{
    m_fileName = inFileName;

    ISX_ASSERT(inTiffFileNames.size());
    m_tiffFileNames = inTiffFileNames;

    isize_t numFramesAccum = 0;

    const bool useNumFrames = inNumFrames.size() == inTiffFileNames.size();
    ISX_ASSERT(inNumFrames.empty() ? true : useNumFrames);

    for (isize_t f(0); f < inTiffFileNames.size(); ++f)
    {
        std::unique_ptr<TiffMovie> p;
        if (useNumFrames)
        {
            p.reset(new TiffMovie(inTiffFileNames[f], inNumFrames[f]));
        }
        else
        {
            p.reset(new TiffMovie(inTiffFileNames[f]));
        }
        m_movies.push_back(std::move(p));

        if (f > 1)
        {
            if (m_movies[0]->getFrameWidth() != m_movies[f]->getFrameWidth()
                || m_movies[0]->getFrameHeight() != m_movies[f]->getFrameHeight())
            {
                ISX_THROW(isx::ExceptionUserInput, "All input files must have the same dimensions.");
            }
            if (m_movies[0]->getDataType() != m_movies[f]->getDataType())
            {
                ISX_THROW(isx::ExceptionUserInput, "All input files must have the same data type.");
            }
        }

        if (!m_movies.empty())
        {
            m_dataType = m_movies[0]->getDataType();
        }

        numFramesAccum += m_movies[f]->getNumFrames();
        m_cumulativeFrames.push_back(numFramesAccum);
    }

    if (inTimingInfo.getNumTimes() != 0)
    {
        // Timing info comes from XML
        m_timingInfos = TimingInfos_t{inTimingInfo};
    }
    else
    {
        /// Timing info comes from TIFF - (set by user)
        Time start = inTimingInfo.getStart();
        DurationInSeconds step = inTimingInfo.getStep(); 
        isize_t numFrames = numFramesAccum + inDroppedFrames.size();
        m_timingInfos = TimingInfos_t{TimingInfo(start, step, numFrames, inDroppedFrames)};
    }

    if (inSpacingInfo.isValid())
    {
        // TODO sweet : Even if the spacing info is valid, we don't trust
        // the width/height because the acquisition software does not store
        // the actual width/height after downsampling.
        // Therefore, we try to get it from the HDF5 file and use the given
        // width/height as a fallback only.
        if (m_movies.size() > 0)
        {
            const SizeInPixels_t numPixels(m_movies[0]->getFrameWidth(), m_movies[0]->getFrameHeight());
            m_spacingInfo = SpacingInfo(numPixels, inSpacingInfo.getPixelSize(), inSpacingInfo.getTopLeft());
        }
        else
        {
            m_spacingInfo = inSpacingInfo;
        }
    }
    else
    {
        const SizeInPixels_t numPixels(m_movies[0]->getFrameWidth(), m_movies[0]->getFrameHeight());
        m_spacingInfo = SpacingInfo(numPixels);
    }

    m_additionalProperties = inAdditionalProperties;

    m_valid = true;
}

NVistaTiffMovie::~NVistaTiffMovie()
{
}

const std::map<std::string, Variant> & 
NVistaTiffMovie::getAdditionalProperties() const
{
    return m_additionalProperties;
}

bool
NVistaTiffMovie::isValid() const
{
    return m_valid;
}

SpVideoFrame_t
NVistaTiffMovie::getFrame(isize_t inFrameNumber)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getFrame");
    AsyncTaskResult<SpVideoFrame_t> asyncTaskResult;
    getFrameAsync(inFrameNumber,
        [&asyncTaskResult, &cv, &mutex](AsyncTaskResult<SpVideoFrame_t> inAsyncTaskResult)
        {
            mutex.lock("getFrame async");
            asyncTaskResult = inAsyncTaskResult;
            mutex.unlock();
            cv.notifyOne();
        }
    );
    cv.wait(mutex);
    mutex.unlock();

    return asyncTaskResult.get();   //throws if asyncTaskResult contains an exception
}

void
NVistaTiffMovie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    std::weak_ptr<NVistaTiffMovie> weakThis = shared_from_this();
    GetFrameCB_t getFrameCB = [weakThis, this, inFrameNumber]()
        {
            auto sharedThis = weakThis.lock();
            if (sharedThis)
            {
                return getFrameInternal(inFrameNumber);
            }
            return SpVideoFrame_t();
        };

    m_ioTaskTracker->schedule(getFrameCB, inCallback);
}

void
NVistaTiffMovie::cancelPendingReads()
{
    m_ioTaskTracker->cancelPendingTasks();
}
    
const isx::TimingInfo &
NVistaTiffMovie::getTimingInfo() const
{
    return m_timingInfos[0];
}

const isx::TimingInfos_t &
NVistaTiffMovie::getTimingInfosForSeries() const
{
    return m_timingInfos;
}

const isx::SpacingInfo &
NVistaTiffMovie::getSpacingInfo() const
{
    return m_spacingInfo;
}

DataType
NVistaTiffMovie::getDataType() const
{
    return m_dataType;
}

std::string
NVistaTiffMovie::getFileName() const
{
    return m_fileName;
}

void
NVistaTiffMovie::serialize(std::ostream& strm) const
{
    for (isize_t m(0); m < m_tiffFileNames.size(); ++m)
    {
        if(m > 0)
        {
            strm << "\n";
        }
        strm << m_tiffFileNames[m];
    }
}

SpVideoFrame_t
NVistaTiffMovie::getFrameInternal(isize_t inFrameNumber)
{
    TimingInfo & ti = m_timingInfos[0];
    SpacingInfo si = getSpacingInfo();
    auto outFrame = std::make_shared<VideoFrame>(
        si,
        isx::getDataTypeSizeInBytes(m_dataType) * si.getNumColumns(),
        1, // numChannels
        m_dataType,
        getTimingInfo().convertIndexToStartTime(inFrameNumber),
        inFrameNumber);
    
    if(ti.isDropped(inFrameNumber))
    {
        std::memset(outFrame->getPixels(), 0, outFrame->getImageSizeInBytes());
        outFrame->setFrameType(VideoFrame::Type::DROPPED);
        return outFrame;
    }

    // The frame was not dropped, shift frame numbers and proceed to read
    isize_t newFrameNumber = ti.timeIdxToRecordedIdx(inFrameNumber);
    isize_t idx = getMovieIndex(newFrameNumber);
    if (idx > 0)
    {
        newFrameNumber = newFrameNumber - m_cumulativeFrames[idx - 1];
    }

    m_movies[idx]->getFrame(newFrameNumber, outFrame);

    return outFrame;
}


isize_t
NVistaTiffMovie::getMovieIndex(isize_t inFrameNumber)
{
    isize_t idx = 0;
    while ((inFrameNumber >= m_cumulativeFrames[idx]) && (idx < m_movies.size() - 1))
    {
        ++idx;
    }
    return idx;
}

} // namespace isx
