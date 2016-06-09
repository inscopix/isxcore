#include "isxMovie.h"
#include "isxHdf5FileHandle.h"
#include "isxException.h"
#include "H5Cpp.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <cassert>

namespace isx {
class Movie::Impl
{
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
                ISX_THROW_EXCEPTION_DATAIO("Unsupported data type");                
            }
            
            
        }  // end of try block
        
        // catch failure caused by the H5File operations
        catch(H5::FileIException error)
        {
            ISX_THROW_EXCEPTION_FILEIO("Failure caused by the H5File operations");
        }
        
        // catch failure caused by the DataSet operations
        catch(H5::DataSetIException error)
        {
            ISX_THROW_EXCEPTION_DATAIO("Failure caused by the DataSet operations");
        }
        
        catch(...)
        {
            ISX_LOG_ERROR("Unhandled exception.");
        }

        // TODO sweet 2016/05/31 : the start and step should be read from
        // the file but it doesn't currently contain these, so picking some
        // dummy values
        isx::Time start = isx::Time();
        isx::Ratio step(1, 30);
        // TODO sweet 2016/06/06 : on Windows the type of m_dims is a uint64_t
        // so this needs some more thought
        m_timingInfo = createDummyTimingInfo(static_cast<uint32_t>(m_dims[0]));
    }
    
    
    Impl(const SpH5File_t & inHdf5File, const std::string & inPath, size_t inNumFrames, size_t inFrameWidth, size_t inFrameHeight)
    : m_isValid(false) 
    , m_H5File(inHdf5File)
    , m_path(inPath)   
    {
        assert((inNumFrames > 0) && (inFrameWidth > 0) && (inFrameHeight > 0));
 
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
        m_timingInfo = createDummyTimingInfo(static_cast<uint32_t>(m_dims[0]));

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
        catch (H5::DataSetIException error)
        {
            ISX_THROW_EXCEPTION_DATAIO("Failure caused by the DataSet operations");
        }
        catch(H5::FileIException error)
        {
            ISX_THROW_EXCEPTION_FILEIO("Failure caused by the H5File operations"); 
        }
        catch(H5::GroupIException error)
        {
            ISX_THROW_EXCEPTION_DATAIO("Failure caused by the Group operations");
        }
 
    }

    bool
    isValid() const
    {
        return m_isValid;
    }

    int 
    getNumFrames() const
    {
        return int(m_timingInfo.getNumTimes());
    }

    int 
    getFrameWidth() const
    {
        return int(m_dims[2]);
    }

    int 
    getFrameHeight() const
    {
        return int(m_dims[1]);
    }

    size_t 
    getFrameSizeInBytes() const
    {
        return m_frameSizeInBytes;
    }

    void 
    getFrame(uint32_t inFrameNumber, void * outBuffer, size_t inBufferSize)
    {
        try {
            
            H5::DataSpace fileSpace(m_dataSpace);            
            hsize_t fileStart[3] = {(hsize_t)inFrameNumber, 0, 0};
            hsize_t fileCount[3] = {1, m_dims[1], m_dims[2]};            
            fileSpace.selectHyperslab(H5S_SELECT_SET, fileCount, fileStart);
            
            H5::DataSpace bufferSpace(3, fileCount);
            hsize_t bufferStart[3] = { 0, 0, 0 };
            bufferSpace.selectHyperslab(H5S_SELECT_SET, fileCount, bufferStart);
            
            m_dataSet.read(outBuffer, m_dataType, bufferSpace, fileSpace);
        }
        catch(H5::DataSetIException error)
        {
            std::cerr << "Exception in " << error.getFuncName() << ": " << std::endl << error.getDetailMsg() << std::endl;
            m_isValid = false;
        }
    }

    double 
    getDurationInSeconds() const
    {
        return m_timingInfo.getDuration().toDouble();
    }

    isx::TimingInfo
    getTimingInfo() const
    {
        return m_timingInfo;
    }
    
    void writeFrame(size_t inFrameNumber, void * inBuffer, size_t inBufferSize);

    void
    serialize(std::ostream& strm) const
    {
        strm << m_path;
    }

private:

    /// A method to create a dummy TimingInfo object from the number of frames.
    ///
    isx::TimingInfo
    createDummyTimingInfo(uint32_t numFrames)
    {
        isx::Time start = isx::Time();
        isx::Ratio step(1, 30);
        return isx::TimingInfo(start, step, numFrames);
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

    void createDataSet(const std::string &name, const H5::DataType &data_type, const H5::DataSpace &data_space);
    std::vector<std::string> splitPath(const std::string &s);
};


Movie::Movie()
{
    m_pImpl.reset(new Impl());
}

Movie::Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath)
{
    if (false == inHdf5FileHandle->isReadOnly())
    {
        ISX_THROW_EXCEPTION_FILEIO("File was opened with no read permission");
    }
    
    m_pImpl.reset(new Impl(inHdf5FileHandle->get(), inPath));
}

Movie::Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath, size_t inNumFrames, size_t inFrameWidth, size_t inFrameHeight)
{
    if (false == inHdf5FileHandle->isReadWrite())
    {
        ISX_THROW_EXCEPTION_FILEIO("File was opened with no write permission");
    }
    
    m_pImpl.reset(new Impl(inHdf5FileHandle->get(), inPath, inNumFrames, inFrameWidth, inFrameHeight));
}

Movie::~Movie()
{
}

bool
Movie::isValid() const
{
    return m_pImpl->isValid();
}

int 
Movie::getNumFrames() const
{
    return m_pImpl->getNumFrames();
}

int 
Movie::getFrameWidth() const
{
    return m_pImpl->getFrameWidth();
}

int 
Movie::getFrameHeight() const
{
    return m_pImpl->getFrameHeight();
}

size_t 
Movie::getFrameSizeInBytes() const
{
    return m_pImpl->getFrameSizeInBytes();
}

void 
Movie::getFrame(uint32_t inFrameNumber, void * outBuffer, size_t inBufferSize)
{
    m_pImpl->getFrame(inFrameNumber, outBuffer, inBufferSize);
}

double 
Movie::getDurationInSeconds() const
{
    return m_pImpl->getDurationInSeconds();
}

isx::TimingInfo
Movie::getTimingInfo() const
{
    return m_pImpl->getTimingInfo();
}

void 
Movie::serialize(std::ostream& strm) const
{
    m_pImpl->serialize(strm);
}

void 
Movie::writeFrame(size_t inFrameNumber, void * inBuffer, size_t inBufferSize)
{
    m_pImpl->writeFrame(inFrameNumber, inBuffer, inBufferSize);
}

void
Movie::Impl::writeFrame(size_t inFrameNumber, void * inBuffer, size_t inBufferSize)
{
    // Make sure the movie is valid
    if (!m_isValid)
    {
        ISX_THROW_EXCEPTION_FILEIO("Writing frame to invalid movie");
    }
         
    // Check that buffer size matches dataspace definition
    if (inBufferSize != m_frameSizeInBytes)
    {
        ISX_THROW_EXCEPTION_USRINPUT("The buffer size does not match the the frame size in the file");
    }
        
    // Check that frame number is within range
    if (inFrameNumber > m_maxdims[0])
    {
        ISX_THROW_EXCEPTION_USRINPUT("Frame number exceeds the total number of frames in the movie");
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
       ISX_THROW_EXCEPTION_DATAIO("Failed to write frame to movie");
    }     
 
}

void 
Movie::Impl::createDataSet (const std::string &name, const H5::DataType &data_type, const H5::DataSpace &data_space)
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
        ISX_THROW_EXCEPTION_DATAIO("Dataset dimension mismatch");
    }
        
    for (int i(0); i < nDims; i++)
    {
        if(dims[i] != m_dims[i])
        {
            ISX_THROW_EXCEPTION_DATAIO("Dataset size mismatch");
        }
            
        if(maxdims[i] != m_maxdims[i])
        {
            ISX_THROW_EXCEPTION_DATAIO("Dataset size mismatch");
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
Movie::Impl::splitPath(const std::string &s)
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

} // namespace isx

