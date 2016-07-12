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
        m_isValid = true;

    }

    Impl(const SpH5File_t & inHdf5File, const std::string & inPath)
    {
        std::unique_ptr<Hdf5Movie> p( new Hdf5Movie(inHdf5File, inPath) );
        m_movies.push_back(std::move(p));
        m_cumulativeFrames.push_back(m_movies[0]->getNumFrames());
        
        // TODO sweet 2016/06/20 : the spacing information should be read from
        // the file, but just use dummy values for now
        m_spacingInfo = createDummySpacingInfo(m_movies[0]->getFrameWidth(), m_movies[0]->getFrameHeight());

        // TODO michele : see above
        std::vector<SpH5File_t> vecFile;
        vecFile.push_back(inHdf5File);
        m_timingInfo = readTimingInfo(vecFile);
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

    isx::TimingInfo
    readTimingInfo(std::vector<SpH5File_t> inHdf5Files)
    {
        H5::DataSet timingInfoDataSet;
        hsize_t totalNumFrames = 0;
        double startTime = 0;
        double temp = 0;

        for (isize_t f(0); f < inHdf5Files.size(); ++f)
        {
            timingInfoDataSet = inHdf5Files[f]->openDataSet("/timeStamp");

            std::vector<hsize_t> timingInfoDims;
            std::vector<hsize_t> timingInfoMaxDims;
            isx::internal::getHdf5SpaceDims(timingInfoDataSet.getSpace(), timingInfoDims, timingInfoMaxDims);

            hsize_t numFrames = timingInfoDims[0];
            double *buffer = new double[numFrames];

            timingInfoDataSet.read(buffer, timingInfoDataSet.getDataType());

            // get start time
            if (f == 0)
            {
                startTime = buffer[0];
            }

            // get isx::Ratio object (in ms)
            for (int i = 0; i < numFrames - 1; i++)
            {
                temp += buffer[i + 1] - buffer[i];
            }
           
            totalNumFrames += numFrames;
        }

        temp *= 1000.0 / double(totalNumFrames);

        isx::Ratio step = isx::Ratio(int64_t(temp), 1000);
        isx::Time start = isx::Time(int64_t(startTime));

        return isx::TimingInfo(start, step, totalNumFrames);
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

const isx::SpacingInfo &
NVistaMovie::getSpacingInfo() const
{
    return m_pImpl->getSpacingInfo();
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

