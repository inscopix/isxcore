#ifndef ISX_TIFF_MOVIE
#define ISX_TIFF_MOVIE

#include <isxCoreFwd.h>
#include <isxCore.h>
#include <isxSpacingInfo.h>
#include <isxTime.h>
#include <string>

/// Forward-declare TIFF formats
struct tiff;
typedef struct tiff TIFF;

namespace isx 
{
    /// A class managing a single TIFF file
    ///
    class TiffMovie
    {
    public: 

        /// Constructor
        /// \param inFileName the filename for one TIFF movie file
        TiffMovie(const std::string & inFileName);

        /// Constructor that allows specification of number of directories
        /// to avoid expensive IO to get this.
        /// \param inFileName the filename for one TIFF movie file
        /// \param inNumDirectories the number of directories in the TIFF movie file
        TiffMovie(const std::string & inFileName, const isize_t inNumDirectories);

        /// Destructor
        ///
        ~TiffMovie();

        /// Get a movie frame
        /// \param inFrameNumber frame index
        /// \param vf output
        /// \throw  isx::ExceptionDataIO    If inFrameNumber is out of range.
        void getFrame(isize_t inFrameNumber, const SpVideoFrame_t & vf);

        /// Get a movie frame
        /// \param inFrameNumber global frame index
        /// \param inSpacingInfo Spacing Info  
        /// \param inTimeStamp time stamp  
        /// \return shared pointer to VideoFrame
        /// \throw  isx::ExceptionDataIO    If inFrameNumber is out of range.
        SpVideoFrame_t getVideoFrame(isize_t inFrameNumber, const SpacingInfo & inSpacingInfo, Time inTimeStamp);

        /// \return the total number of frames in the movie
        ///
        isize_t 
        getNumFrames() const;

        /// return the width in pixels
        ///
        isize_t
        getFrameWidth() const;

        /// \return the height in pixels
        ///
        isize_t
        getFrameHeight() const;

        /// return the data type of pixels
        ///
        DataType
        getDataType() const;

    private:

        void initialize(const std::string & inFileName);

        std::string m_fileName;
        tiff *      m_tif;

        isize_t m_frameWidth;
        isize_t m_frameHeight;
        isize_t m_numFrames;
        DataType m_dataType;

    };
}

#endif //ISX_TIFF_MOVIE
