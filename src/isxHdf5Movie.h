#ifndef ISX_HDF5MOVIE_H
#define ISX_HDF5MOVIE_H

#include "isxCoreFwd.h"
#include "isxHdf5FileHandle.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxVideoFrame.h"
#include "isxHdf5Utils.h"
#include "isxTimingInfo.h"

namespace isx {    

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
        const std::string & 
            getPath()
        {
            return m_path;
        }

        void readProperties(TimingInfo & timingInfo);
        void writeProperties(TimingInfo & timingInfo);

    private:

        H5::CompType getTimingInfoType();

        SpH5File_t m_H5File;
        std::string m_path;

        H5::DataSet m_dataSet;
        H5::DataSpace m_dataSpace;
        H5::DataType m_dataType;

        int m_ndims;
        std::vector<hsize_t> m_dims;
        std::vector<hsize_t> m_maxdims;

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




} // namespace isx

#endif
