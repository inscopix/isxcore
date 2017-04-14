#ifndef ISX_TIFF_MOVIE
#define ISX_TIFF_MOVIE

#include <isxCoreFwd.h>
#include <isxCore.h>
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

        /// Contructor
        /// \param inFileName the filename for one TIFF movie file
        TiffMovie(const std::string & inFileName);
        
        /// Destructor
        ///
        ~TiffMovie();

        /// Get a movie frame
        /// \param inFrameNumber frame index
        /// \param vf output
        /// \throw  isx::ExceptionDataIO    If inFrameNumber is out of range.
        void getFrame(isize_t inFrameNumber, const SpVideoFrame_t & vf);

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

    private:

        isize_t getNumDirectories();

        std::string m_fileName;
        TIFF *      m_tif;

        isize_t m_frameWidth;
        isize_t m_frameHeight;
        isize_t m_numFrames;

    };
}

#endif //ISX_TIFF_MOVIE