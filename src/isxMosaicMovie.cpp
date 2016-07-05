#include "isxMovieImpl.h"
#include "isxMosaicMovie.h"
#include "isxHdf5Utils.h"
#include "isxHdf5Movie.h"
#include <iostream>
#include <sstream>
#include <queue>

namespace isx {
    class MosaicMovie::Impl : public MovieImpl
    {


    public:
        ~Impl() {};

        Impl() {};

        
        Impl(const SpH5File_t & inHdf5File, const std::string & inPath)
        {
            std::string moviePath = inPath + "/Movie";

            m_movie = std::make_unique<Hdf5Movie>(inHdf5File, moviePath);            

            // TODO sweet 2016/05/31 : the start and step should be read from
            // the file but it doesn't currently contain these, so picking some
            // dummy values
            isx::Ratio frameRate(30, 1);
            m_timingInfo = createDummyTimingInfo(m_movie->getNumFrames(), frameRate);
            m_movie->readProperties(m_timingInfo);
            m_isValid = true;
        }


        Impl(const SpH5File_t & inHdf5File, const std::string & inPath, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight, isx::Ratio inFrameRate)
            : m_isValid(false)
        {

            m_movie = std::make_unique<Hdf5Movie>(inHdf5File, inPath + "/Movie", inNumFrames, inFrameWidth, inFrameHeight);            
            // TODO sweet 2016/09/31 : the start and step should also be specified
            // but we don't currently have a mechnanism for that
            m_timingInfo = createDummyTimingInfo(inNumFrames, inFrameRate);
            m_movie->writeProperties(m_timingInfo);
            m_isValid = true;

        }

        bool
            isValid() const
        {
            return m_isValid;
        }

        isize_t
            getFrameWidth() const
        {
            return m_movie->getFrameWidth();
        }

        isize_t
            getFrameHeight() const
        {
            return m_movie->getFrameHeight();
        }

        isize_t
            getFrameSizeInBytes() const
        {
            return m_movie->getFrameSizeInBytes();
        }

        SpU16VideoFrame_t
            getFrame(isize_t inFrameNumber)
        {

            Time frameTime = m_timingInfo.convertIndexToTime(inFrameNumber);

            auto nvf = std::make_shared<U16VideoFrame_t>(
                getFrameWidth(), getFrameHeight(),
                sizeof(uint16_t) * getFrameWidth(),
                1, // numChannels
                frameTime, inFrameNumber);

            ScopedMutex locker(IoQueue::getMutex(), "getFrame");
            m_movie->getFrame(inFrameNumber, nvf);

            return nvf;
        }

        void
            serialize(std::ostream& strm) const
        {
            strm << m_movie->getPath();
        }

        std::string getName()
        {
            std::string path = m_movie->getPath();
            std::string name = path.substr(path.find_last_of("/") + 1);
            return name;
        }

        void
            writeFrame(isize_t inFrameNumber, void * inBuffer, isize_t inBufferSize)
        {
            if (!m_isValid)
            {
                ISX_THROW(isx::ExceptionFileIO, "Writing frame to invalid movie.");
            }
            ScopedMutex locker(IoQueue::getMutex(), "writeFrame");
            m_movie->writeFrame(inFrameNumber, inBuffer, inBufferSize);

        }

    private:
    

        /// A method to create a dummy TimingInfo object from the number of frames.
        ///
        isx::TimingInfo
            createDummyTimingInfo(isize_t numFrames, isx::Ratio inFrameRate)
        {
            isx::Time start = isx::Time();
            isx::Ratio step = inFrameRate.invert();
            return isx::TimingInfo(start, step, numFrames);
        }


        bool m_isValid = false;

        std::unique_ptr<Hdf5Movie> m_movie; 

    };



    MosaicMovie::MosaicMovie()
    {
        m_pImpl.reset(new Impl());
    }

    MosaicMovie::MosaicMovie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath)
    {
        m_pImpl.reset(new Impl(inHdf5FileHandle->get(), inPath));
    }

    MosaicMovie::MosaicMovie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight, isx::Ratio inFrameRate)
    {
        if (false == inHdf5FileHandle->isReadWrite())
        {
            ISX_THROW(isx::ExceptionFileIO, "File was opened with no write permission.");
        }

        m_pImpl.reset(new Impl(inHdf5FileHandle->get(), inPath, inNumFrames, inFrameWidth, inFrameHeight, inFrameRate));
    }

    MosaicMovie::~MosaicMovie()
    {
    }

    bool
        MosaicMovie::isValid() const
    {
        return m_pImpl->isValid();
    }

    isize_t
        MosaicMovie::getNumFrames() const
    {
        return m_pImpl->getNumFrames();
    }

    isize_t
        MosaicMovie::getFrameWidth() const
    {
        return m_pImpl->getFrameWidth();
    }

    isize_t
        MosaicMovie::getFrameHeight() const
    {
        return m_pImpl->getFrameHeight();
    }

    isize_t
        MosaicMovie::getFrameSizeInBytes() const
    {
        return m_pImpl->getFrameSizeInBytes();
    }

    SpU16VideoFrame_t
        MosaicMovie::getFrame(isize_t inFrameNumber)
    {
        return m_pImpl->getFrame(inFrameNumber);
    }

    SpU16VideoFrame_t
        MosaicMovie::getFrame(const Time & inTime)
    {
        return m_pImpl->getFrame(inTime);
    }

    void
        MosaicMovie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
    {
        return m_pImpl->getFrameAsync(inFrameNumber, inCallback);
    }

    void
        MosaicMovie::getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback)
    {
        return m_pImpl->getFrameAsync(inTime, inCallback);
    }

    double
        MosaicMovie::getDurationInSeconds() const
    {
        return m_pImpl->getDurationInSeconds();
    }

    const isx::TimingInfo &
        MosaicMovie::getTimingInfo() const
    {
        return m_pImpl->getTimingInfo();
    }

    void
        MosaicMovie::serialize(std::ostream& strm) const
    {
        m_pImpl->serialize(strm);
    }

    std::string
        MosaicMovie::getName()
    {
        return m_pImpl->getName();
    }

    void
        MosaicMovie::writeFrame(isize_t inFrameNumber, void * inBuffer, isize_t inBufferSize)
    {
        m_pImpl->writeFrame(inFrameNumber, inBuffer, inBufferSize);
    }

} // namespace isx

