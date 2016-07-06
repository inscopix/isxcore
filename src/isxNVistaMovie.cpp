#include "isxHdf5Movie.h"
#include "isxMovieImpl.h"
#include "isxNVistaMovie.h"
#include <iostream>
#include <vector>
#include <sstream>


namespace isx {
class NVistaMovie::Impl : public MovieImpl
{
    

public:
    ~Impl(){};

    Impl(){};
    
    Impl(const std::vector<SpH5File_t> & inHdf5Files, const std::vector<std::string> & inPaths)
    {
        ISX_ASSERT(inHdf5Files.size());
        ISX_ASSERT(inHdf5Files.size() == inPaths.size());

        isize_t w, h;
        isize_t numFramesAccum = 0;

        for (isize_t f(0); f < inHdf5Files.size(); ++f)
        {
            m_movies.push_back(std::make_unique<Hdf5Movie>(inHdf5Files[f], inPaths[f]));
            
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

        isx::Ratio frameRate(30, 1);
        m_timingInfo = createDummyTimingInfo(numFramesAccum, frameRate);
        m_isValid = true;

    }

    Impl(const SpH5File_t & inHdf5File, const std::string & inPath)
    {

        m_movies.push_back(std::make_unique<Hdf5Movie>(inHdf5File, inPath));
        m_cumulativeFrames.push_back(m_movies[0]->getNumFrames());
        
        // TODO sweet 2016/05/31 : the start and step should be read from
        // the file but it doesn't currently contain these, so picking some
        // dummy values
        isx::Ratio frameRate(30, 1);
        m_timingInfo = createDummyTimingInfo(m_movies[0]->getNumFrames(), frameRate);
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
        return m_movies[0]->getFrameWidth();
    }

    isize_t
    getFrameHeight() const
    {
        return m_movies[0]->getFrameHeight();
    }

    isize_t 
    getFrameSizeInBytes() const
    {
        return m_movies[0]->getFrameSizeInBytes();
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


    void
    serialize(std::ostream& strm) const
    {
        for (isize_t m(0); m < m_movies.size(); ++m)
        {
            strm << m_movies[m]->getPath() << std::endl;
        }
    }
    
    std::string getName()
    {
        std::string path = m_movies[0]->getPath();
        std::string name = path.substr(path.find_last_of("/") + 1);
        return name;
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

    isize_t getMovieIndex(isize_t inFrameNumber)
    {
        isize_t idx = 0;
        while ((inFrameNumber >= m_cumulativeFrames[idx]) && (idx < m_movies.size() - 1))
        {
            ++idx;
        }

        return idx;
    }
    

    bool m_isValid = false;


    std::vector<std::unique_ptr<Hdf5Movie>> m_movies;
    std::vector<isize_t> m_cumulativeFrames;


};


NVistaMovie::NVistaMovie()
{
    m_pImpl.reset(new Impl());
}

NVistaMovie::NVistaMovie(const std::vector<SpHdf5FileHandle_t> & inHdf5FileHandles, const std::vector<std::string> & inPaths)
{
    std::vector<SpH5File_t> files;
    for (isize_t i(0); i < inHdf5FileHandles.size(); ++i)
    {
        files.push_back(inHdf5FileHandles[i]->get());
    }
    m_pImpl.reset(new Impl(files, inPaths));
}


NVistaMovie::NVistaMovie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath)
{    
    m_pImpl.reset(new Impl(inHdf5FileHandle->get(), inPath));
}


NVistaMovie::~NVistaMovie()
{
}

bool
NVistaMovie::isValid() const
{
    return m_pImpl->isValid();
}

isize_t 
NVistaMovie::getNumFrames() const
{
    return m_pImpl->getNumFrames();
}

isize_t
NVistaMovie::getFrameWidth() const
{
    return m_pImpl->getFrameWidth();
}

isize_t
NVistaMovie::getFrameHeight() const
{
    return m_pImpl->getFrameHeight();
}

isize_t 
NVistaMovie::getFrameSizeInBytes() const
{
    return m_pImpl->getFrameSizeInBytes();
}

SpU16VideoFrame_t
NVistaMovie::getFrame(isize_t inFrameNumber)
{
    return m_pImpl->getFrame(inFrameNumber);
}

SpU16VideoFrame_t
NVistaMovie::getFrame(const Time & inTime)
{
    return m_pImpl->getFrameByTime(inTime);
}

void
NVistaMovie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    return m_pImpl->getFrameAsync(inFrameNumber, inCallback);
}

void
NVistaMovie::getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback)
{
    return m_pImpl->getFrameAsync(inTime, inCallback);
}

double 
NVistaMovie::getDurationInSeconds() const
{
    return m_pImpl->getDurationInSeconds();
}

const isx::TimingInfo &
NVistaMovie::getTimingInfo() const
{
    return m_pImpl->getTimingInfo();
}

void 
NVistaMovie::serialize(std::ostream& strm) const
{
    m_pImpl->serialize(strm);
}

std::string 
NVistaMovie::getName()
{
    return m_pImpl->getName();
}

} // namespace isx

