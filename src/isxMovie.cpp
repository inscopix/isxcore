#include "isxMovie.h"
#include "isxHdf5FileHandle.h"
#include "H5Cpp.h"
#include <iostream>

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
            
            bool bExists = true;
            
            try 
            {  // Determine if the dataset exists in the HDF5 file
                m_dataSet = m_H5File->openDataSet(m_path);
            }
            catch (H5::GroupIException not_found_error)
            {
                bExists = false;
            }

            if (bExists)
            {
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

    bool setMovieSize(int inNumFrames, int inFrameWidth, int inFrameHeight);

    bool writeFrame(uint32_t inFrameNumber, void * inBuffer, size_t inBufferSize);

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
};


Movie::Movie()
{
    m_pImpl.reset(new Impl());
}

Movie::Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath)
{
    m_pImpl.reset(new Impl(inHdf5FileHandle->get(), inPath));
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

bool 
Movie::setMovieSize(int inNumFrames, int inFrameWidth, int inFrameHeight)
{
    return m_pImpl->setMovieSize(inNumFrames, inFrameWidth, inFrameHeight);
}

bool 
Movie::writeFrame(uint32_t inFrameNumber, void * inBuffer, size_t inBufferSize)
{
    return m_pImpl->writeFrame(inFrameNumber, inBuffer, inBufferSize);
}

bool
Movie::Impl::setMovieSize(int inNumFrames, int inFrameWidth, int inFrameHeight)
{
    /* Set rank, dimensions and max dimensions */
    m_ndims = 3;
    m_dims.resize(m_ndims);
    m_maxdims.resize(m_ndims);

    if (inNumFrames > 0)
    {
        m_dims[0] = inNumFrames;
        m_maxdims[0] = inNumFrames;
    }
    else
    {
        m_dims[0] = 10;     // default number of frames at creation time
        m_maxdims[0] = H5S_UNLIMITED;
    }

    m_dims[1] = inFrameHeight;
    m_maxdims[1] = inFrameHeight;

    m_dims[2] = inFrameWidth;
    m_maxdims[2] = inFrameWidth;

    /* Create the dataspace */
    m_dataSpace = H5::DataSpace(m_ndims, m_dims.data(), m_maxdims.data());

    /* Modify dataset creation properties, i.e. enable chunking  */
    H5::DSetCreatPropList prop;
    if (inNumFrames <= 0)
    {
        std::vector<hsize_t> chunk_dims(3);
        chunk_dims[0] = 1;
        chunk_dims[1] = 16;
        chunk_dims[2] = 16;
        prop.setChunk(m_ndims, chunk_dims.data());
    }

    /* Create a new dataset within the file */
    m_dataType = H5::PredType::STD_U16LE;
    m_dataSet = H5::DataSet(m_H5File->createDataSet(m_path, m_dataType, m_dataSpace, prop));

 }

bool
Movie::Impl::writeFrame(uint32_t inFrameNumber, void * inBuffer, size_t inBufferSize)
{
    bool result = false;

    // Check that buffer size matches dataspace definition
    if (inBufferSize != m_dims[1] * m_dims[2])
        return result;

    // Check that frame number is within range
    if ((m_maxdims[0] != H5S_UNLIMITED) && (inFrameNumber > m_maxdims[0]))
        return result;

    // If this is an extendible dataset, and inFrameNumber exceeds the size of the data, make sure the frame will be appended
    if ((H5S_UNLIMITED == m_maxdims[0]) && (inFrameNumber > m_dims[0]))
    {
        inFrameNumber = m_dims[0];
        m_dims[0] += 1;
        m_dataSet.extend(m_dims.data());
        m_dataSpace = H5::DataSpace(m_dataSet.getSpace());
    }

    // Select Hyperslab
    hsize_t offset[3] = { inFrameNumber, 0, 0 };
    hsize_t dims[3] = { 1, m_dims[1], m_dims[2] };
    m_dataSpace.selectHyperslab(H5S_SELECT_SET, dims, offset);

    // Define memory space.
   H5::DataSpace *memspace = new H5::DataSpace(1, &inBufferSize, NULL);

    // Write data to the dataset.
    m_dataSet.write(inBuffer, m_dataType, *memspace, m_dataSpace);
    
    // Update the return value
    result = true;

    return result;
}


} // namespace isx

