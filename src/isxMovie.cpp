#include "isxMovie.h"
#include "isxRecording.h"
#include "isxRecording_internal.h"
#include "H5Cpp.h"

#include <iostream>

namespace isx {
class Movie::Impl
{
public:
    ~Impl(){};
    Impl(){};
    Impl(const SpRecording_t & inRecording, const std::string & inPath)
    : m_H5File(inRecording->m_pImpl->getH5FileRef())
    , m_path(inPath)
    {
        try
        {
            // Turn off the auto-printing when failure occurs so that we can
            // handle the errors appropriately
            H5::Exception::dontPrint();
            
            // Open an existing dataset from H5File.
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

        // TODO asweet 2015/05/31 : the start and step should be read from
        // the file but it doesn't currently contain these, so picking some
        // dummy values
        isx::Time start = isx::Time();
        isx::Ratio step(1, 30);
        uint32_t numFrames = m_dims[0];
        m_timingInfo = isx::TimingInfo(start, step, numFrames);
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

private:
    bool m_isValid = false;
    Recording::Impl::tH5File_SP m_H5File;
    std::string m_path;

    
    H5::H5File m_file;
    H5::DataSet m_dataSet;
    H5::DataSpace m_dataSpace;
    H5::DataType m_dataType;
    
    int m_ndims;
    std::vector<hsize_t> m_dims;
    std::vector<hsize_t> m_maxdims;
    size_t m_frameSizeInBytes;

    isx::TimingInfo m_timingInfo;
};


Movie::Movie()
{
    m_pImpl.reset(new Impl());
}

Movie::Movie(const SpRecording_t & inRecording, const std::string & inPath)
{
    m_pImpl.reset(new Impl(inRecording, inPath));
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

} // namespace isx

