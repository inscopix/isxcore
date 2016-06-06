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
                std::cerr << "Unsupported data type." << std::endl;
            }
            
            
        }  // end of try block
        
        // catch failure caused by the H5File operations
        catch(H5::FileIException error)
        {
            error.printError();
        }
        
        // catch failure caused by the DataSet operations
        catch(H5::DataSetIException error)
        {
            error.printError();
        }
        
        catch(...)
        {
            std::cerr << "Unhandled exception." << std::endl;
        }
    }
    
    
    Impl(const SpH5File_t & inHdf5File, const std::string & inPath, size_t inNumFrames, size_t inFrameWidth, size_t inFrameHeight)
    : m_H5File(inHdf5File)
    , m_path(inPath)
    , m_isValid(false)    
    {
        std::assert(inNumFrames && inFrameWidth && inFrameHeight);
 
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
            error.printError(); 
        }
        catch(H5::FileIException error)
        {
            error.printError(); 
        }
        catch(H5::GroupIException error)
        {
            error.printError(); 
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
        return int(m_dims[0]);
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
        // TODO aschildan 4/21/2016: Fix to take actual framerate into account
        return double(getNumFrames()) / 30.0;
    }
    
    void writeFrame(size_t inFrameNumber, void * inBuffer, size_t inBufferSize);

    void
    serialize(std::ostream& strm) const
    {
        strm << m_path;
    }

private:

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
        throw ExceptionFileIO("isx::Movie::Movie", "File was opened with no read permission");
    }
    
    m_pImpl.reset(new Impl(inHdf5FileHandle->get(), inPath));
}

Movie::Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath, size_t inNumFrames, size_t inFrameWidth, size_t inFrameHeight)
{
    if (false == inHdf5FileHandle->isReadWrite())
    {
        throw ExceptionFileIO("isx::Movie::Movie", "File was opened with no write permission");
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
        throw ExceptionFileIO("isx::Movie::Impl::writeFrame", "Writing frame to invalid movie");
         
    // Check that buffer size matches dataspace definition
    if (inBufferSize != m_frameSizeInBytes)
        throw ExceptionUserInput("isx::Movie::Impl::writeFrame", "The buffer size does not match the the frame size in the file");
        
    // Check that frame number is within range
    if (inFrameNumber > m_maxdims[0])
        throw ExceptionUserInput("isx::Movie::Impl::writeFrame", "Frame number exceeds the total number of frames in the movie");
           
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
       throw ExceptionDataIO("isx::Movie::Impl::writeFrame", "Failed to write frame to movie");
    }     
 
}


void 
Movie::Impl::createDataSet (const std::string &name, const H5::DataType &data_type, const H5::DataSpace &data_space)
{
    // Parse name and create the hierarchy tree (if not in the file already). 
    // Every level other than the last one is created as a group. The last level is the dataset
    std::vector<std::string> tree = splitPath(name);
    std::string location("/");
    for (int i(0); i < tree.size() - 1; ++i)
    {
        location += tree[i];  
        try
        {
            m_H5File->openGroup(location);
        }
        catch (...)
        {
            m_H5File->createGroup(location);
        }
        
        location += "/";
        
    }

    // Create it if dataset doesn't exist. Overwrite it otherwise
    try
    {
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
            throw ExceptionDataIO("isx::Movie::Impl::createDataSet", "Dataset dimension mismatch");
        
        for (int i(0); i < nDims; i++)
        {
            if(dims[i] != m_dims[i])
                throw ExceptionDataIO("isx::Movie::Impl::createDataSet", "Dataset size mismatch");
            
            if(maxdims[i] != m_maxdims[i])
                throw ExceptionDataIO("isx::Movie::Impl::createDataSet", "Dataset size mismatch");
        }
        
        // Dataset is valid if we get here
        m_dataType = type;
        m_dataSpace = space;
        
        if (m_dataType == H5::PredType::STD_U16LE)
        {
            m_frameSizeInBytes = m_dims[1] * m_dims[2] * 2;
        }
    }
    catch(...)
    {
        m_dataSet = H5::DataSet(m_H5File->createDataSet(name, data_type, data_space));  
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
        tokens.push_back(item);
    }
    return tokens;
}

} // namespace isx

