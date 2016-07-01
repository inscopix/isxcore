#ifndef ISX_HDF5MOVIE_H
#define ISX_HDF5MOVIE_H

#include "isxCoreFwd.h"
#include "isxHdf5FileHandle.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxVideoFrame.h"
#include "isxHdf5Utils.h"

namespace isx {

    /// type for an nvista movie video frame
    ///
    typedef VideoFrame<uint16_t> U16VideoFrame_t;

    /// shared_ptr type for an nvista movie video frame
    ///
    typedef std::shared_ptr<U16VideoFrame_t> SpU16VideoFrame_t;

    class Hdf5Movie
    {
    public:
        Hdf5Movie(const SpH5File_t & inHdf5File, const std::string & inPath);
        Hdf5Movie(const SpH5File_t & inHdf5File, const std::string & inPath, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight);
        ~Hdf5Movie();

        void getFrame(isize_t inFrameNumber, const SpU16VideoFrame_t & vf);
        void writeFrame(isize_t inFrameNumber, void * inBuffer, isize_t inBufferSize);

        isize_t 
            getFrameSizeInBytes() 
        { 
            return m_dims[1] * m_dims[2] * 2; 
        }

        isize_t 
            getNumFrames() const 
        { 
            return m_dims[0]; 
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

    private:
        SpH5File_t m_H5File;
        std::string m_path;

        H5::DataSet m_dataSet;
        H5::DataSpace m_dataSpace;
        H5::DataType m_dataType;

        int m_ndims;
        std::vector<hsize_t> m_dims;
        std::vector<hsize_t> m_maxdims;
        
    };




} // namespace isx

#endif
