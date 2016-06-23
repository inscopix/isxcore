#include "isxMovie.h"
#include "isxHdf5FileHandle.h"
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
    typedef std::shared_ptr<Movie::Impl> SpImpl_t;
    typedef std::weak_ptr<Movie::Impl> WpImpl_t;
public:
    ~Impl(){};

    Impl(){};

    Impl(const SpH5File_t & inHdf5File, const std::string & inPath)
    : m_H5File(inHdf5File)    
    , m_path(inPath)
    {
        try
        {
            // Turn off the auto-printing when failure occurs so that we can
            // handle the errors appropriately
            H5::Exception::dontPrint();  
            m_dataSet = m_H5File->openDataSet(m_path); 

            m_dataType = m_dataSet.getDataType();
            m_dataSpace = m_dataSet.getSpace();

            m_ndims = m_dataSpace.getSimpleExtentNdims();
            m_dims.resize(m_ndims);
            m_maxdims.resize(m_ndims);
            m_dataSpace.getSimpleExtentDims(&m_dims[0], &m_maxdims[0]);

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
        isx::Time start = isx::Time();
        isx::Ratio frameRate(30, 1);
        // TODO sweet 2016/06/06 : on Windows the type of m_dims is a uint64_t
        // so this needs some more thought
        m_timingInfo = createDummyTimingInfo(m_dims[0], frameRate);


    }
    
    
    Impl(const SpH5File_t & inHdf5File, const std::string & inPath, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight, isx::Ratio inFrameRate)
    : m_isValid(false) 
    , m_H5File(inHdf5File)
    , m_path(inPath) 
    {
        ISX_ASSERT(inNumFrames > 0);
        ISX_ASSERT(inFrameWidth > 0);
        ISX_ASSERT(inFrameHeight > 0);
 
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

        // TODO sweet 2016/09/31 : the start and step should also be specified
        // but we don't currently have a mechnanism for that
        m_timingInfo = createDummyTimingInfo(m_dims[0], inFrameRate);

        /* Create the dataspace */
        m_dataSpace = H5::DataSpace(m_ndims, m_dims.data(), m_maxdims.data()); 

        /* Create a new dataset within the file */
        m_dataType = H5::PredType::STD_U16LE;
        try
        {
            createDataSet(m_path, m_dataType, m_dataSpace);
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
        Time frameTime = m_timingInfo.convertIndexToTime(inFrameNumber);
        
        auto nvf = std::make_shared<U16VideoFrame_t>(
            getFrameWidth(), getFrameHeight(),
            sizeof(uint16_t) * getFrameWidth(),
            1, // numChannels
            frameTime, inFrameNumber);
        try {
            
            H5::DataSpace fileSpace(m_dataSpace);            
            hsize_t fileStart[3] = {inFrameNumber, 0, 0};
            hsize_t fileCount[3] = {1, m_dims[1], m_dims[2]};            
            fileSpace.selectHyperslab(H5S_SELECT_SET, fileCount, fileStart);
            
            H5::DataSpace bufferSpace(3, fileCount);
            hsize_t bufferStart[3] = { 0, 0, 0 };
            bufferSpace.selectHyperslab(H5S_SELECT_SET, fileCount, bufferStart);
            
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
    getFrameAsync(size_t inFrameNumber, MovieGetFrameCB_t inCallback)
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
        hsize_t frameNumber = hsize_t(m_timingInfo.convertTimeToIndex(inTime));
        return getFrameAsync(frameNumber, inCallback);
    }

    void
    processFrameQueue()
    {
        isx::ScopedMutex locker(m_frameRequestQueueMutex, "getFrameAsync");
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
        // Make sure the movie is valid
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
               
        // Define file space.
        H5::DataSpace fileSpace(m_dataSpace);
        hsize_t fileOffset[3] = { inFrameNumber, 0, 0 };
        hsize_t dims[3] = { 1, m_dims[1], m_dims[2] };
        fileSpace.selectHyperslab(H5S_SELECT_SET, dims, fileOffset);

        // Define memory space.
        H5::DataSpace memSpace = H5::DataSpace(3, dims, NULL);
        hsize_t memOffset[3] = { 0, 0, 0 };
        memSpace.selectHyperslab(H5S_SELECT_SET, dims, memOffset);

        // Write data to the dataset.
        try
        {
           m_dataSet.write(inBuffer, m_dataType, memSpace, fileSpace);
        }
   
        // Catch failure caused by the DataSet operations
        catch (H5::DataSetIException error)
        {
           ISX_THROW(isx::ExceptionDataIO,
                "Failed to write frame to movie.\n", error.getDetailMsg());
        }
    }

private:
    class FrameRequest
    {
    public:
        FrameRequest(size_t inFrameNumber, MovieGetFrameCB_t inCallback)
        : m_frameNumber(inFrameNumber)
        , m_callback(inCallback){}

        size_t              m_frameNumber;
        MovieGetFrameCB_t   m_callback;
    };

    /// A method to create a dummy TimingInfo object from the number of frames.
    ///
    isx::TimingInfo
    createDummyTimingInfo(isize_t numFrames, isx::Ratio inFrameRate)
    {
        isx::Time start = isx::Time();
        isx::Ratio step = inFrameRate.invert();
        return isx::TimingInfo(start, step, numFrames);
    }

    void 
    createDataSet (const std::string &name, const H5::DataType &data_type, const H5::DataSpace &data_space)
    {
        // Parse name and create the hierarchy tree (if not in the file already). 
        // Every level other than the last one is created as a group. The last level is the dataset
        std::vector<std::string> tree = splitPath(name); 
     
        std::string currentObjName("/");
        H5::Group currentGroup = m_H5File->openGroup(currentObjName);
        hsize_t nObjInGroup = currentGroup.getNumObjs();
        
        
        
        unsigned int nCreateFromIdx = 0;
        
        while((nObjInGroup > 0) && (nCreateFromIdx < tree.size()))
        {
            std::string targetObjName = currentObjName + "/" + tree[nCreateFromIdx];
            bool bTargetFound = false;
            
            for(hsize_t obj(0); obj < nObjInGroup; ++obj)
            {
                std::string objName = currentGroup.getObjnameByIdx(obj);
                if(objName == tree[nCreateFromIdx])
                {
                    bTargetFound = true;
                    break;
                }
            }

            if(bTargetFound)
            {
                nCreateFromIdx += 1;
                
                if(nCreateFromIdx < tree.size())
                {
                    currentGroup = m_H5File->openGroup(targetObjName);
                    currentObjName = targetObjName;
                    nObjInGroup = currentGroup.getNumObjs(); 
                }           
            }
            else
            {
                break;
            }
        }
        
           
        for ( ; nCreateFromIdx < tree.size(); ++nCreateFromIdx)
        {
            if(nCreateFromIdx == (tree.size() - 1))
            {
                m_dataSet = m_H5File->createDataSet(name, data_type, data_space);           
                return;
            }
            
            std::string targetObjName = currentObjName + "/" + tree[nCreateFromIdx]; 
            m_H5File->createGroup(targetObjName); 
            currentObjName = targetObjName;
        }
        
        // If we get here, the dataset exists in the file and we don't need to create it
        m_dataSet = m_H5File->openDataSet(name);
        H5::DataType type = m_dataSet.getDataType();
        H5::DataSpace space = m_dataSet.getSpace();
        int nDims = space.getSimpleExtentNdims();;
        std::vector<hsize_t> dims(nDims);
        std::vector<hsize_t> maxdims(nDims);
        space.getSimpleExtentDims(&dims[0], &maxdims[0]);
            
        // Check that the size of the file dataset is the same as the one the 
        // user is trying to write out
        if(nDims != m_ndims)
        {
            ISX_THROW(isx::ExceptionDataIO, "Dataset dimension mismatch");
        }
            
        for (int i(0); i < nDims; i++)
        {
            if(dims[i] != m_dims[i])
            {
                ISX_THROW(isx::ExceptionDataIO, "Dataset size mismatch");
            }
                
            if(maxdims[i] != m_maxdims[i])
            {
                ISX_THROW(isx::ExceptionDataIO, "Dataset size mismatch");
            }
        }
            
        // Dataset is valid if we get here
        m_dataType = type;
        m_dataSpace = space;
            
        if (m_dataType == H5::PredType::STD_U16LE)
        {
            m_frameSizeInBytes = m_dims[1] * m_dims[2] * 2;
        }    
    }

    std::vector<std::string> 
    splitPath(const std::string &s)
    {
        using namespace std;
        char delim = '/';
        stringstream ss(s);
        string item;
        vector<string> tokens;
        while (getline(ss, item, delim)) 
        {
            if(!item.empty())
            {
                tokens.push_back(item);
            }
        }
        return tokens;
    }

    bool m_isValid = false;
    SpH5File_t m_H5File;
    std::string m_path;
    
    H5::DataSet m_dataSet;
    H5::DataSpace m_dataSpace;
    H5::DataType m_dataType;
    
    int m_ndims;
    std::vector<hsize_t> m_dims;
    std::vector<hsize_t> m_maxdims;
    size_t m_frameSizeInBytes;

    isx::TimingInfo m_timingInfo;
    std::queue<FrameRequest>    m_frameRequestQueue;
    isx::Mutex                  m_frameRequestQueueMutex;

};


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
Movie::getFrameAsync(size_t inFrameNumber, MovieGetFrameCB_t inCallback)
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

