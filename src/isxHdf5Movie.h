#ifndef ISX_HDF5MOVIE_H
#define ISX_HDF5MOVIE_H

#include "isxCoreFwd.h"
#include "isxHdf5FileHandle.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxHdf5Utils.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxMovieDefs.h"


namespace isx {    

    /// A class that reads and writes from/to a movie contained in a HDF5 dataset
    ///
    class Hdf5Movie
    {
    public:
        
        /// Constructor for existing movie
        /// \param inHdf5File   HDF5 file
        /// \param inPath       Path of dataset within the file
        Hdf5Movie(const SpH5File_t & inHdf5File, const std::string & inPath);
        
        /// Constructor for a new movie
        /// \param inHdf5File   HDF5 file
        /// \param inPath       Path of dataset within the file
        /// \param inNumFrames  Total number of frames
        /// \param inFrameWidth Width in pixels
        /// \param inFrameHeight Height in pixels
        Hdf5Movie(const SpH5File_t & inHdf5File, const std::string & inPath, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight);

        /// Destructor
        ///
        ~Hdf5Movie();

        /// Get a movie frame
        /// \param inFrameNumber frame index
        /// \param vf output
        void getFrame(isize_t inFrameNumber, const SpU16VideoFrame_t & vf);
        
        /// Write a new frame to the movie
        /// \param inFrameNumber    frame index
        /// \param inBuffer         data buffer
        /// \param inBufferSize     buffer size
        void writeFrame(isize_t inFrameNumber, void * inBuffer, isize_t inBufferSize);

        /// \return the number of bytes per frame
        ///
        isize_t 
            getFrameSizeInBytes() 
        { 
            return m_dims[1] * m_dims[2] * 2; 
        }

        /// \return the total number of frames in the movie
        ///
        isize_t 
            getNumFrames() const 
        { 
            return m_dims[0]; 
        }

        /// return the width in pixels
        ///
        isize_t
            getFrameWidth() const
        {
            return m_dims[2];
        }

        /// \return the height in pixels
        ///
        isize_t
            getFrameHeight() const
        {
            return m_dims[1];
        }
        
        /// return the path of the movie within the file
        ///
        const std::string & 
            getPath()
        {
            return m_path;
        }

        /// Read timing info properties
        /// \param timingInfo the timing information
        void readProperties(TimingInfo & timingInfo);
        
        /// Write timing info properties
        /// \param timingInfo the timing information
        void writeProperties(TimingInfo & timingInfo);

    private:

        /// \return the HDF5 timing structure
        ///
        H5::CompType getTimingInfoType();

        SpH5File_t m_H5File;
        std::string m_path;

        H5::DataSet m_dataSet;
        H5::DataSpace m_dataSpace;
        H5::DataType m_dataType;

        static const hsize_t s_numDims;
        isx::internal::HSizeVector_t m_dims;
        isx::internal::HSizeVector_t m_maxdims;

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
