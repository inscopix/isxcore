#include "isxMovie.h"
#include "H5Cpp.h"

#include <iostream>

namespace isx {
class Movie::Impl
{
public:
    ~Impl(){};
    Impl(){};
    Impl(const std::string & inPath)
    : path_(inPath)
    {
        try
        {
            // Turn off the auto-printing when failure occurs so that we can
            // handle the errors appropriately
            H5::Exception::dontPrint();
            
            // Open an existing file and dataset.
            file_ = H5::H5File(path_.c_str(), H5F_ACC_RDONLY);
            dataSet_ = file_.openDataSet("/images");
            dataType_ = dataSet_.getDataType();
            dataSpace_ = dataSet_.getSpace();
            
            ndims_ = dataSpace_.getSimpleExtentNdims();
            dims_.resize(ndims_);
            maxdims_.resize(ndims_);
            dataSpace_.getSimpleExtentDims(&dims_[0], &maxdims_[0]);
            
            if (dataType_ == H5::PredType::STD_U16LE)
            {
                frameSizeInBytes_ = dims_[1] * dims_[2] * 2;
                isValid_ = true;
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

    int 
    getNumFrames() const
    {
        return int(dims_[0]);
    }

    int 
    getFrameWidth() const
    {
        return int(dims_[2]);
    }

    int 
    getFrameHeight() const
    {
        return int(dims_[1]);
    }

    size_t 
    getFrameSizeInBytes() const
    {
        return frameSizeInBytes_;
    }

    void 
    getFrame(uint32_t inFrameNumber, void * outBuffer, size_t inBufferSize)
    {
        try {
            H5::DataSpace fileSpace(dataSpace_);
            hsize_t fileStart[3] = {(hsize_t)inFrameNumber, 0, 0};
            hsize_t fileCount[3] = {1, dims_[1], dims_[2]};
            fileSpace.selectHyperslab(H5S_SELECT_SET, fileCount, fileStart);
            
            H5::DataSpace bufferSpace(3, fileCount);
            hsize_t bufferStart[3] = { 0, 0, 0 };
            bufferSpace.selectHyperslab(H5S_SELECT_SET, fileCount, bufferStart);
            
            dataSet_.read(outBuffer, dataType_, bufferSpace, fileSpace);
        }
        catch(H5::DataSetIException error)
        {
            std::cerr << "Exception in " << error.getFuncName() << ": " << std::endl << error.getDetailMsg() << std::endl;
            isValid_ = false;
        }
    }

    double 
    getDurationInSeconds() const
    {
        ///TODO aschildan 4/21/2016: Fix to take actual framerate into account
        return double(getNumFrames()) / 30.0;
    }

private:
    bool isValid_ = false;
    std::string path_;
    
    H5::H5File file_;
    H5::DataSet dataSet_;
    H5::DataSpace dataSpace_;
    H5::DataType dataType_;
    
    int ndims_;
    std::vector<hsize_t> dims_;
    std::vector<hsize_t> maxdims_;
    size_t frameSizeInBytes_;
};


Movie::Movie()
{
    pImpl.reset(new Impl());
}

Movie::Movie(const std::string & inPath)
{
    pImpl.reset(new Impl(inPath));
}

Movie::~Movie()
{
}

int 
Movie::getNumFrames() const
{
    return pImpl->getNumFrames();
}

int 
Movie::getFrameWidth() const
{
    return pImpl->getFrameWidth();
}

int 
Movie::getFrameHeight() const
{
    return pImpl->getFrameHeight();
}

size_t 
Movie::getFrameSizeInBytes() const
{
    return pImpl->getFrameSizeInBytes();
}

void 
Movie::getFrame(uint32_t inFrameNumber, void * outBuffer, size_t inBufferSize)
{
    pImpl->getFrame(inFrameNumber, outBuffer, inBufferSize);
}

double 
Movie::getDurationInSeconds() const
{
    return pImpl->getDurationInSeconds();
}

} // namespace isx

