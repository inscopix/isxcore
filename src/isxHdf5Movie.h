#ifndef ISX_HDF5MOVIE_H
#define ISX_HDF5MOVIE_H

#include "isxHdf5FileHandle.h"
#include "isxException.h"
#include "isxAssert.h"

namespace isx {

    class Hdf5Movie
    {
    public:
        void getFrame(isize_t inFrameNumber, const SpU16VideoFrame_t & vf);

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
