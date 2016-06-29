#include "isxMovie.h"
#include "isxHdf5FileHandle.h"
#include "isxHdf5Utils.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxIoQueue.h"
#include "H5Cpp.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <queue>

namespace isx {
class Movie::Impl : public std::enable_shared_from_this<Movie::Impl>
{
    

public:
    ~Impl(){};

    Impl(){};

    Impl(const SpH5File_t & inHdf5File, const std::string & inPath)
    : m_H5File(inHdf5File)    
    , m_path(inPath)
    {
        std::string moviePath = m_path;
        std::string propertyPath;

        if (isInProjectFile())
        {
            moviePath += "/Movie";
            propertyPath = m_path + "/Properties";
        }

        try
        {
            // Turn off the auto-printing when failure occurs so that we can
            // handle the errors appropriately
            H5::Exception::dontPrint();  
            m_dataSet = m_H5File->openDataSet(moviePath);

            m_dataType = m_dataSet.getDataType();
            m_dataSpace = m_dataSet.getSpace();

            isx::internal::getHdf5SpaceDims(m_dataSpace, m_dims, m_maxdims);
            m_ndims = m_dims.size();

            if (m_dataType == H5::PredType::STD_U16LE)
            {
                m_frameSizeInBytes = m_dims[1] * m_dims[2] * 2;
                m_isValid = true;
            }
            else
            {
                ISX_THROW(isx::ExceptionDataIO,
                    "Unsupported data type ", m_dataType.getTag());
            }
            
            
        }  // end of try block
        
        catch (const H5::FileIException& error)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failure caused by H5File operations.\n", error.getDetailMsg());
        }
        
        catch (const H5::DataSetIException& error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failure caused by DataSet operations.\n", error.getDetailMsg());
        }

        catch (...)
        {
            ISX_ASSERT(false, "Unhandled exception.");
        }
        
        // TODO sweet 2016/05/31 : the start and step should be read from
        // the file but it doesn't currently contain these, so picking some
        // dummy values
        isx::Ratio frameRate(30, 1);
        m_timingInfo = createDummyTimingInfo(m_dims[0], frameRate);
        readProperties(propertyPath);
    }
    
    
    Impl(const SpH5File_t & inHdf5File, const std::string & inPath, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight, isx::Ratio inFrameRate)
    : m_isValid(false) 
    , m_H5File(inHdf5File)
    , m_path(inPath) 
    {
        ISX_ASSERT(inNumFrames > 0);
        ISX_ASSERT(inFrameWidth > 0);
        ISX_ASSERT(inFrameHeight > 0);

        std::string moviePath = m_path + "/Movie";
 
        /* Set rank, dimensions and max dimensions */
        m_ndims = 3;
        m_dims.resize(m_ndims);
        m_maxdims.resize(m_ndims);

        m_dims[0] = inNumFrames;
        m_maxdims[0] = inNumFrames;

        m_dims[1] = inFrameHeight;
        m_maxdims[1] = inFrameHeight;

        m_dims[2] = inFrameWidth;
        m_maxdims[2] = inFrameWidth;

        /* Create the dataspace */
        m_dataSpace = H5::DataSpace(static_cast<int>(m_ndims), m_dims.data(), m_maxdims.data());

        /* Create a new dataset within the file */
        m_dataType = H5::PredType::STD_U16LE;
        try
        {
            m_dataSet = isx::internal::createHdf5DataSet(m_H5File, moviePath, m_dataType, m_dataSpace);
            m_isValid = true;
            m_frameSizeInBytes = m_dims[1] * m_dims[2] * 2;
        }
        catch (const H5::DataSetIException& error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failure caused by H5 DataSet operations.\n", error.getDetailMsg());
        }
        catch (const H5::FileIException& error)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failure caused by H5 File operations.\n", error.getDetailMsg());
        }
        catch (const H5::GroupIException& error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failure caused by H5 Group operations.\n", error.getDetailMsg());
        }

        // TODO sweet 2016/09/31 : the start and step should also be specified
        // but we don't currently have a mechnanism for that
        m_timingInfo = createDummyTimingInfo(m_dims[0], inFrameRate);
        writeProperties();

    }

    bool
    isValid() const
    {
        return m_isValid;
    }

    isize_t 
    getNumFrames() const
    {
        return m_timingInfo.getNumTimes();
    }

    isize_t
    getFrameWidth() const
    {
        return m_dims[2];
    }

    isize_t
    getFrameHeight() const
    {
        return m_dims[1];
    }

    isize_t 
    getFrameSizeInBytes() const
    {
        return m_frameSizeInBytes;
    }

    SpU16VideoFrame_t
    getFrame(isize_t inFrameNumber)
    {
        ScopedMutex locker(IoQueue::getMutex(), "getFrame");
        Time frameTime = m_timingInfo.convertIndexToTime(inFrameNumber);
        
        auto nvf = std::make_shared<U16VideoFrame_t>(
            getFrameWidth(), getFrameHeight(),
            sizeof(uint16_t) * getFrameWidth(),
            1, // numChannels
            frameTime, inFrameNumber);
        try {
            isx::internal::HSizeVector_t size = {1, m_dims[1], m_dims[2]};
            isx::internal::HSizeVector_t offset = {inFrameNumber, 0, 0};
            H5::DataSpace fileSpace = isx::internal::createHdf5SubSpace(
                    m_dataSpace, offset, size);
            H5::DataSpace bufferSpace = isx::internal::createHdf5BufferSpace(
                    size);

            m_dataSet.read(nvf->getPixels(), m_dataType, bufferSpace, fileSpace);
        }
        catch (const H5::DataSetIException& error)
        {
            ISX_LOG_ERROR("Exception in ", error.getFuncName(), ":\n", error.getDetailMsg());
            m_isValid = false;
        }

        return nvf;
    }

    SpU16VideoFrame_t
    getFrame(const Time & inTime)
    {
        isize_t frameNumber = m_timingInfo.convertTimeToIndex(inTime);
        return getFrame(frameNumber);
    }

    void
    getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
    {
        {
            isx::ScopedMutex locker(m_frameRequestQueueMutex, "getFrameAsync");
            m_frameRequestQueue.push(FrameRequest(inFrameNumber, inCallback));
        }
        processFrameQueue();
    }

    void
    getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback)
    {
        isize_t frameNumber = m_timingInfo.convertTimeToIndex(inTime);
        return getFrameAsync(frameNumber, inCallback);
    }

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

    void
    purgeFrameQueue()
    {
        isx::ScopedMutex locker(m_frameRequestQueueMutex, "getFrameAsync");
        while (!m_frameRequestQueue.empty())
        {
            m_frameRequestQueue.pop();
        }
    }

    double 
    getDurationInSeconds() const
    {
        return m_timingInfo.getDuration().toDouble();
    }

    const isx::TimingInfo &
    getTimingInfo() const
    {
        return m_timingInfo;
    }
    
    void
    serialize(std::ostream& strm) const
    {
        strm << m_path;
    }
    
    std::string getName()
    {
        return m_path.substr(m_path.find_last_of("/")+1);
    }

    void
    writeFrame(isize_t inFrameNumber, void * inBuffer, isize_t inBufferSize)
    {
        ScopedMutex locker(IoQueue::getMutex(), "writeFrame");
        if (!m_isValid)
        {
            ISX_THROW(isx::ExceptionFileIO, "Writing frame to invalid movie.");
        }
             
        // Check that buffer size matches dataspace definition
        if (inBufferSize != m_frameSizeInBytes)
        {
            ISX_THROW(isx::ExceptionUserInput,
                "The buffer size (", inBufferSize, " B) does not match the frame size (",
                m_frameSizeInBytes, " B).");
        }
            
        // Check that frame number is within range
        if (inFrameNumber > m_maxdims[0])
        {
            ISX_THROW(isx::ExceptionUserInput,
                "Frame number (", inFrameNumber, ") exceeds the total number of frames (",
                m_maxdims[0], ") in the movie.");
        }

        isx::internal::HSizeVector_t size = {1, m_dims[1], m_dims[2]};
        isx::internal::HSizeVector_t offset = {inFrameNumber, 0, 0};
        H5::DataSpace fileSpace = isx::internal::createHdf5SubSpace(
            m_dataSpace, offset, size);
        H5::DataSpace bufferSpace = isx::internal::createHdf5BufferSpace(
            size);

        // Write data to the dataset.
        try
        {
           m_dataSet.write(inBuffer, m_dataType, bufferSpace, fileSpace);
        }
   
        // Catch failure caused by the DataSet operations
        catch (const H5::DataSetIException &error)
        {
           ISX_THROW(isx::ExceptionDataIO,
                "Failed to write frame to movie.\n", error.getDetailMsg());
        }
    }

private:
    class FrameRequest
    {
    public:
        FrameRequest(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
        : m_frameNumber(inFrameNumber)
        , m_callback(inCallback){}

        isize_t             m_frameNumber;
        MovieGetFrameCB_t   m_callback;
    };
    
    H5::CompType 
    getTimingInfoType()
    {
        H5::CompType timingInfoType(sizeof(sTimingInfo_t));
        timingInfoType.insertMember(sTimingInfoTimeSecsNum, HOFFSET(sTimingInfo_t, timeSecsNum), H5::PredType::NATIVE_INT64);
        timingInfoType.insertMember(sTimingInfoTimeSecsDen, HOFFSET(sTimingInfo_t, timeSecsDen), H5::PredType::NATIVE_INT64);
        timingInfoType.insertMember(sTimingInfoTimeOffset, HOFFSET(sTimingInfo_t, timeOffset), H5::PredType::NATIVE_INT32);
        timingInfoType.insertMember(sTimingInfoStepNum, HOFFSET(sTimingInfo_t, stepNum), H5::PredType::NATIVE_INT64);
        timingInfoType.insertMember(sTimingInfoStepDen, HOFFSET(sTimingInfo_t, stepDen), H5::PredType::NATIVE_INT64);
        timingInfoType.insertMember(sTimingInfoNumTimes, HOFFSET(sTimingInfo_t, numTimes), H5::PredType::NATIVE_HSIZE);
        
        return timingInfoType;
    }

    void 
    readProperties(const std::string & property_path)
    {
        if (property_path.empty())
        {
            return;
        }

        H5::DataSet dataset;

        try
        {
            dataset = m_H5File->openDataSet(property_path + "/TimingInfo");
        }
        catch (const H5::FileIException& error)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failure to read movie properties caused by H5 File operations.\n", error.getDetailMsg());
        }
        catch (const H5::GroupIException& error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failure to read movie properties caused by H5 Group operations.\n", error.getDetailMsg());
        }
        
        sTimingInfo_t t;
        dataset.read(&t, getTimingInfoType());

        Ratio secSinceEpoch(t.timeSecsNum, t.timeSecsDen);
        Time start(secSinceEpoch, t.timeOffset);
        Ratio step(t.stepNum, t.stepDen);
        isize_t numTimes = t.numTimes;
        m_timingInfo = TimingInfo(start, step, numTimes);
    }

    void
    writeProperties()
    {
        /*
        * Initialize the data
        */
        Time time = m_timingInfo.getStart();
        sTimingInfo_t t;
        t.timeSecsNum = time.secsFrom(time).getNum();
        t.timeSecsDen = time.secsFrom(time).getDen();
        t.timeOffset = time.getUtcOffset();
        t.stepNum = m_timingInfo.getStep().getNum();
        t.stepDen = m_timingInfo.getStep().getDen();
        t.numTimes = m_timingInfo.getNumTimes();
        
        /*
        * Create the data space.
        */
        hsize_t dim[] = { 1 };   /* Dataspace dimensions */
        H5::DataSpace space(1, dim);
        
        try
        {
            /*
            * Create the dataset.
            */
            std::string grName = m_path + "/Properties";
            H5::Group grProperties = m_H5File->createGroup(grName);
            std::string dataset_name = "TimingInfo";
            H5::DataSet dataset = H5::DataSet(grProperties.createDataSet(dataset_name, getTimingInfoType(), space));
            /*
            * Write data to the dataset;
            */
            dataset.write(&t, getTimingInfoType());
        }
        
        catch (const H5::DataSetIException &error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failed to write movie properties.\n", error.getDetailMsg());
        }
        catch (const H5::FileIException& error)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failure to write movie properties caused by H5 File operations.\n", error.getDetailMsg());
        }
        catch (const H5::GroupIException& error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failure to write movie properties caused by H5 Group operations.\n", error.getDetailMsg());
        }

    }

    /// A method to create a dummy TimingInfo object from the number of frames.
    ///
    isx::TimingInfo
    createDummyTimingInfo(isize_t numFrames, isx::Ratio inFrameRate)
    {
        isx::Time start = isx::Time();
        isx::Ratio step = inFrameRate.invert();
        return isx::TimingInfo(start, step, numFrames);
    }

    bool
    isInProjectFile()
    {
        std::vector<std::string> tokens = isx::internal::splitPath(m_path);
        bool res = false;

        if (tokens.size() && tokens[0] == "MosaicProject")
        {
            res = true;
        }
        return res;
    }

    bool m_isValid = false;
    SpH5File_t m_H5File;
    std::string m_path;
    
    H5::DataSet m_dataSet;
    H5::DataSpace m_dataSpace;
    H5::DataType m_dataType;
    
    hsize_t m_ndims;
    std::vector<hsize_t> m_dims;
    std::vector<hsize_t> m_maxdims;
    size_t m_frameSizeInBytes;

    isx::TimingInfo m_timingInfo;
    std::queue<FrameRequest>    m_frameRequestQueue;
    isx::Mutex                  m_frameRequestQueueMutex;
    
    typedef std::shared_ptr<Movie::Impl> SpImpl_t;
    typedef std::weak_ptr<Movie::Impl> WpImpl_t;

    typedef struct {
        int64_t timeSecsNum;
        int64_t timeSecsDen;
        int32_t timeOffset;
        int64_t stepNum;
        int64_t stepDen;
        isize_t numTimes;
    } sTimingInfo_t;


    static const std::string sTimingInfoTimeSecsNum;
    static const std::string sTimingInfoTimeSecsDen;
    static const std::string sTimingInfoTimeOffset;
    static const std::string sTimingInfoStepNum;
    static const std::string sTimingInfoStepDen;
    static const std::string sTimingInfoNumTimes;

};

const std::string Movie::Impl::sTimingInfoTimeSecsNum = "TimeSecsNum";
const std::string Movie::Impl::sTimingInfoTimeSecsDen = "TimeSecsDen";
const std::string Movie::Impl::sTimingInfoTimeOffset = "TimeOffset";
const std::string Movie::Impl::sTimingInfoStepNum = "StepNum";
const std::string Movie::Impl::sTimingInfoStepDen = "StepDen";
const std::string Movie::Impl::sTimingInfoNumTimes = "NumTimes";

Movie::Movie()
{
    m_pImpl.reset(new Impl());
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

Movie::~Movie()
{
}

bool
Movie::isValid() const
{
    return m_pImpl->isValid();
}

isize_t 
Movie::getNumFrames() const
{
    return m_pImpl->getNumFrames();
}

isize_t
Movie::getFrameWidth() const
{
    return m_pImpl->getFrameWidth();
}

isize_t
Movie::getFrameHeight() const
{
    return m_pImpl->getFrameHeight();
}

isize_t 
Movie::getFrameSizeInBytes() const
{
    return m_pImpl->getFrameSizeInBytes();
}

SpU16VideoFrame_t
Movie::getFrame(isize_t inFrameNumber)
{
    return m_pImpl->getFrame(inFrameNumber);
}

SpU16VideoFrame_t
Movie::getFrame(const Time & inTime)
{
    return m_pImpl->getFrame(inTime);
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

double 
Movie::getDurationInSeconds() const
{
    return m_pImpl->getDurationInSeconds();
}

const isx::TimingInfo &
Movie::getTimingInfo() const
{
    return m_pImpl->getTimingInfo();
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

void 
Movie::writeFrame(isize_t inFrameNumber, void * inBuffer, isize_t inBufferSize)
{
    m_pImpl->writeFrame(inFrameNumber, inBuffer, inBufferSize);
}

} // namespace isx

