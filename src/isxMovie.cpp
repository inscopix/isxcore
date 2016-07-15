#include "isxMovie.h"
#include "isxHdf5Movie.h"
#include "isxHdf5Utils.h"
#include "isxMutex.h"
#include "isxIoQueue.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <queue>
#include <mutex>
#include <memory>
#include <cmath>

namespace isx {
class Movie::Impl : public std::enable_shared_from_this<Movie::Impl>
{
    

public:
    ~Impl(){};

    Impl(){};
    
    Impl(const std::vector<SpH5File_t> & inHdf5Files, const std::vector<std::string> & inPaths)
    {
        initialize(inHdf5Files, inPaths);
    }

    Impl(const SpH5File_t & inHdf5File, const std::string & inPath)
    {
        // Figure out if the input is a recording from nVista or a Mosaic Project
//        if (isx::internal::hasDatasetAtPath(inHdf5File, inPath, "Properties"))
        if (inPath != "/images")
        {
            // mosaic movie
            std::string moviePath = inPath + "/Movie";

            std::unique_ptr<Hdf5Movie> p(new Hdf5Movie(inHdf5File, moviePath));

            // TODO sweet 2016/05/31 : the start and step should be read from
            // the file but it doesn't currently contain these, so picking some
            // dummy values
            p->readProperties(m_timingInfo);

            // TODO sweet 2016/06/20 : the spacing information should be read from
            // the file, but just use dummy values for now
            m_spacingInfo = createDummySpacingInfo(p->getFrameWidth(), p->getFrameHeight());
            m_movies.push_back(std::move(p));

            m_isValid = true;
            
        }
        else
        {
            // nvista recording
            std::vector<SpH5File_t> hdf5Files(1, inHdf5File);
            std::vector<std::string> paths(1, inPath);
            initialize(hdf5Files, paths);
        }
    }

    void
    initialize(const std::vector<SpH5File_t> & inHdf5Files, const std::vector<std::string> & inPaths)
    {
        ISX_ASSERT(inHdf5Files.size());
        ISX_ASSERT(inHdf5Files.size() == inPaths.size());

        isize_t w, h;
        isize_t numFramesAccum = 0;

        for (isize_t f(0); f < inHdf5Files.size(); ++f)
        {
            std::unique_ptr<Hdf5Movie> p( new Hdf5Movie(inHdf5Files[f], inPaths[f]) );
            m_movies.push_back(std::move(p));
            
            if (f > 1)
            {
                ISX_ASSERT(w == m_movies[f]->getFrameWidth());
                ISX_ASSERT(h == m_movies[f]->getFrameHeight());
            }
            else
            {
                w = m_movies[f]->getFrameWidth();
                h = m_movies[f]->getFrameHeight();
            }

            numFramesAccum += m_movies[f]->getNumFrames();
            m_cumulativeFrames.push_back(numFramesAccum);
        }

        // TODO michele 2016/07/08 : time since epoch comes from host machine and frame rate 
        // is calculated, so these values are not what we really want. we should want to 
        // pull these from the xml eventually
        m_timingInfo = readTimingInfo(inHdf5Files);
        m_spacingInfo = createDummySpacingInfo(m_movies[0]->getFrameWidth(), m_movies[0]->getFrameHeight());
        m_isValid = true;
    }

    Impl(const SpH5File_t & inHdf5File, const std::string & inPath, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight, isx::Ratio inFrameRate)
        : m_isValid(false)
    {

        std::unique_ptr<Hdf5Movie> p(new Hdf5Movie(inHdf5File, inPath + "/Movie", inNumFrames, inFrameWidth, inFrameHeight));

        // TODO sweet 2016/09/31 : the start and step should also be specified
        // but we don't currently have a mechnanism for that
        m_timingInfo = createDummyTimingInfo(inNumFrames, inFrameRate);
        p->writeProperties(m_timingInfo);

        // TODO sweet 2016/06/20 : the spacing information should be read from
        // the file, but just use dummy values for now
        m_spacingInfo = createDummySpacingInfo(inFrameWidth, inFrameHeight);
        m_movies.push_back(std::move(p));
        m_cumulativeFrames.push_back(inNumFrames);
        m_isValid = true;
    }

    Impl(const SpH5File_t & inHdf5File, const std::string & inPath, const TimingInfo & inTimingInfo, const SpacingInfo & inSpacingInfo)
        : m_isValid(false)
    {
        m_timingInfo = inTimingInfo;
        m_spacingInfo = inSpacingInfo;

        isize_t numFrames = inTimingInfo.getNumTimes();
        SizeInPixels_t numPixels = inSpacingInfo.getNumPixels();

        std::unique_ptr<Hdf5Movie> p(new Hdf5Movie(inHdf5File, inPath + "/Movie", numFrames, numPixels.getWidth(), numPixels.getHeight()));
        p->writeProperties(m_timingInfo);
        m_movies.push_back(std::move(p));
        m_cumulativeFrames.push_back(numFrames);
        m_isValid = true;
    }

    bool
    isValid() const
    {
        return m_isValid;
    }

    SpU16VideoFrame_t
    getFrame(isize_t inFrameNumber)
    {
        
        Time frameTime = m_timingInfo.convertIndexToTime(inFrameNumber);
        SpacingInfo si = getSpacingInfo();
        auto nvf = std::make_shared<U16VideoFrame_t>(
            si.getNumColumns(), si.getNumRows(),
            sizeof(uint16_t) * si.getNumColumns(),
            1, // numChannels
            frameTime, inFrameNumber);

        isize_t newFrameNumber = inFrameNumber;
        isize_t idx = getMovieIndex(inFrameNumber);
        if (idx > 0)
        {
            newFrameNumber = inFrameNumber - m_cumulativeFrames[idx - 1];
        }        

        ScopedMutex locker(IoQueue::getMutex(), "getFrame");
        m_movies[idx]->getFrame(newFrameNumber, nvf);
        return nvf;
    }

    SpU16VideoFrame_t
    getFrameByTime(const Time & inTime)
    {
        isize_t frameNumber = m_timingInfo.convertTimeToIndex(inTime);
        return getFrame(frameNumber);
    }

    /// Get a frame asynchronously
    /// \param inFrameNumber    frame number
    /// \param inCallback   callback to execute when finished
    void
    getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
    {
        {
            isx::ScopedMutex locker(m_frameRequestQueueMutex, "getFrameAsync");
            m_frameRequestQueue.push(FrameRequest(inFrameNumber, inCallback));
        }
        processFrameQueue();
    }

    /// Get frame asynchronously by time
    /// \param inTime    time point
    /// \param inCallback   callback to execute when finished
    void
    getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback)
    {
        isize_t frameNumber = m_timingInfo.convertTimeToIndex(inTime);
        return getFrameAsync(frameNumber, inCallback);
    }

    /// \return timing information for the movie
    ///
    const isx::TimingInfo &
    getTimingInfo() const
    {
        return m_timingInfo;
    }

    /// \return timing information for the movie
    ///
    const SpacingInfo &
    getSpacingInfo() const
    {
        return m_spacingInfo;
    }

    void
    writeFrame(const SpU16VideoFrame_t & inVideoFrame)
    {
        if (!m_isValid)
        {
            ISX_THROW(isx::ExceptionFileIO, "Writing frame to invalid movie.");
        }
        ScopedMutex locker(IoQueue::getMutex(), "writeFrame");
        m_movies[0]->writeFrame(inVideoFrame);
    }
    
    void
    serialize(std::ostream& strm) const
    {
        for (isize_t m(0); m < m_movies.size(); ++m)
        {
            if(m > 0)
            {
                strm << "\n";
            }
            strm << m_movies[m]->getPath();
        }
    }
    
    std::string getName()
    {
        std::string path = m_movies[0]->getPath();
        std::string name = path.substr(path.find_last_of("/") + 1);
        return name;
    }


private:
    /// a class representing a frame request
    ///
    class FrameRequest
    {
    public:
        /// constructor
        /// \param inFrameNumber    frame number requested
        /// \param inCallback       callback to execute when finished
        FrameRequest(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
            : m_frameNumber(inFrameNumber)
            , m_callback(inCallback) {}

        isize_t             m_frameNumber;  //!< frame index
        MovieGetFrameCB_t   m_callback;     //!< callback to execute
    };

    /// process next frame request
    ///
    void
    processFrameQueue()
    {
        isx::ScopedMutex locker(m_frameRequestQueueMutex, "processFrameQueue");
        if (!m_frameRequestQueue.empty())
        {
            FrameRequest fr = m_frameRequestQueue.front();
            m_frameRequestQueue.pop();
            WpImpl_t weakThis = shared_from_this();
            IoQueue::instance()->dispatch([weakThis, this, fr]()
            {
                SpImpl_t sharedThis = weakThis.lock();
                if (!sharedThis)
                {
                    fr.m_callback(nullptr);
                }
                else
                {
                    fr.m_callback(getFrame(fr.m_frameNumber));
                    processFrameQueue();
                }
            });
        }
    }

    /// Purge queue
    ///
    void
    purgeFrameQueue()
    {
        isx::ScopedMutex locker(m_frameRequestQueueMutex, "getFrameAsync");
        while (!m_frameRequestQueue.empty())
        {
            m_frameRequestQueue.pop();
        }
    }

    isx::TimingInfo
    readTimingInfo(std::vector<SpH5File_t> inHdf5Files)
    {
        H5::DataSet timingInfoDataSet;
        hsize_t totalNumFrames = 0;
        int64_t startTime = 0;
        double totalDurationInSecs = 0;

        for (isize_t f(0); f < inHdf5Files.size(); ++f)
        {
            timingInfoDataSet = inHdf5Files[f]->openDataSet("/timeStamp");

            std::vector<hsize_t> timingInfoDims;
            std::vector<hsize_t> timingInfoMaxDims;
            isx::internal::getHdf5SpaceDims(timingInfoDataSet.getSpace(), timingInfoDims, timingInfoMaxDims);

            hsize_t numFrames = timingInfoDims[0];
            std::vector<double> buffer(numFrames);

            timingInfoDataSet.read(buffer.data(), timingInfoDataSet.getDataType());

            // get start time
            if (f == 0)
            {
                startTime = int64_t(buffer[0]);
            }

            totalDurationInSecs += buffer[numFrames - 1] - buffer[0];
            totalNumFrames += numFrames;
        }

        totalDurationInSecs *= 1000.0 / double(totalNumFrames);

        isx::DurationInSeconds step = isx::DurationInSeconds(isize_t(std::round(totalDurationInSecs)), 1000);
        isx::Time start = isx::Time(startTime);

        return isx::TimingInfo(start, step, totalNumFrames);
    }

    /// A method to create a dummy TimingInfo object from the number of frames.
    ///
    TimingInfo
    createDummyTimingInfo(isize_t numFrames, Ratio inFrameRate)
    {
        isx::Time start = isx::Time();
        isx::Ratio step = inFrameRate.getInverse();
        return isx::TimingInfo(start, step, numFrames);
    }

    /// A method to create a dummy spacing information from the number of rows and columns.
    ///
    SpacingInfo
    createDummySpacingInfo(isize_t width, isize_t height)
    {
        SizeInPixels_t numPixels(width, height);
        SizeInMicrons_t pixelSize(Ratio(22, 10), Ratio(22, 10));
        PointInMicrons_t topLeft(0, 0);
        return SpacingInfo(numPixels, pixelSize, topLeft);
    }

    isize_t getMovieIndex(isize_t inFrameNumber)
    {
        isize_t idx = 0;
        while ((inFrameNumber >= m_cumulativeFrames[idx]) && (idx < m_movies.size() - 1))
        {
            ++idx;
        }

        return idx;
    }

    bool                        m_isValid = false;
    isx::TimingInfo             m_timingInfo;
    isx::SpacingInfo            m_spacingInfo;

    std::queue<FrameRequest>    m_frameRequestQueue;
    isx::Mutex                  m_frameRequestQueueMutex;

    std::vector<std::unique_ptr<Hdf5Movie>> m_movies;
    std::vector<isize_t> m_cumulativeFrames;

    typedef std::shared_ptr<Movie::Impl> SpImpl_t;
    typedef std::weak_ptr<Movie::Impl> WpImpl_t;
};


Movie::Movie()
{
    m_pImpl.reset(new Impl());
}

Movie::Movie(const std::vector<SpHdf5FileHandle_t> & inHdf5FileHandles, const std::vector<std::string> & inPaths)
{
    std::vector<SpH5File_t> files;
    for (isize_t i(0); i < inHdf5FileHandles.size(); ++i)
    {
        files.push_back(inHdf5FileHandles[i]->get());
    }
    m_pImpl.reset(new Impl(files, inPaths));
}


Movie::Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath)
{    
    m_pImpl.reset(new Impl(inHdf5FileHandle->get(), inPath));
}

Movie::Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight, isx::Ratio inFrameRate)
{
    if (false == inHdf5FileHandle->isReadWrite())
    {
        ISX_THROW(isx::ExceptionFileIO, "File was opened with no write permission.");
    }

    m_pImpl.reset(new Impl(inHdf5FileHandle->get(), inPath, inNumFrames, inFrameWidth, inFrameHeight, inFrameRate));
}

Movie::Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath, const TimingInfo & inTimingInfo, const SpacingInfo & inSpacingInfo)
{
    if (false == inHdf5FileHandle->isReadWrite())
    {
        ISX_THROW(isx::ExceptionFileIO, "File was opened with no write permission.");
    }

    m_pImpl.reset(new Impl(inHdf5FileHandle->get(), inPath, inTimingInfo, inSpacingInfo));
}

Movie::~Movie()
{
}

bool
Movie::isValid() const
{
    return m_pImpl->isValid();
}

SpU16VideoFrame_t
Movie::getFrame(isize_t inFrameNumber)
{
    return m_pImpl->getFrame(inFrameNumber);
}

SpU16VideoFrame_t
Movie::getFrame(const Time & inTime)
{
    return m_pImpl->getFrameByTime(inTime);
}

void
Movie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    return m_pImpl->getFrameAsync(inFrameNumber, inCallback);
}

void
Movie::getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback)
{
    return m_pImpl->getFrameAsync(inTime, inCallback);
}

const isx::TimingInfo &
Movie::getTimingInfo() const
{
    return m_pImpl->getTimingInfo();
}

const isx::SpacingInfo &
Movie::getSpacingInfo() const
{
    return m_pImpl->getSpacingInfo();
}

void
Movie::writeFrame(const SpU16VideoFrame_t & inVideoFrame)
{
    m_pImpl->writeFrame(inVideoFrame);
}

void 
Movie::serialize(std::ostream& strm) const
{
    m_pImpl->serialize(strm);
}

std::string 
Movie::getName()
{
    return m_pImpl->getName();
}

} // namespace isx

