#ifndef ISX_MOSAIC_MOVIE_H
#define ISX_MOSAIC_MOVIE_H

#include "isxWritableMovie.h"
#include "isxMosaicMovieFile.h"

#include <memory>

namespace isx
{

/// Encapsulates movie information and data.
///
/// All information and data is read from and written to one file.
/// All IO operations on that file are performed by the IoQueue.
class MosaicMovie : public WritableMovie
{
public:

    /// Empty constructor.
    ///
    /// This creates a valid c++ object but an invalid movie.
    MosaicMovie();

    /// Read constructor.
    ///
    /// This opens an existing movie from a file.
    ///
    /// \param  inFileName  The name of the movie file.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the movie file fails.
    MosaicMovie(const std::string & inFileName);

    /// Write constructor.
    ///
    /// This creates a new movie in a file and initializes the data with
    /// zeros.
    ///
    /// \param  inFileName      The name of the movie file.
    /// \param  inTimingInfo    The timing information of the movie.
    /// \param  inSpacingInfo   The spacing information of the movie.
    ///
    /// \throw  isx::ExceptionFileIO    If writing the movie file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the movie data fails.
    MosaicMovie(const std::string & inFileName,
                const TimingInfo & inTimingInfo,
                const SpacingInfo & inSpacingInfo);

    // Overrides - see base classes for documentation
    bool isValid() const override;

    SpU16VideoFrame_t getFrame(isize_t inFrameNumber) override;

    SpU16VideoFrame_t getFrame(const Time & inTime) override;

    void getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback) override;

    void getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback) override;

    void writeFrame(const SpU16VideoFrame_t & inVideoFrame) override;

    const isx::TimingInfo & getTimingInfo() const override;

    const isx::SpacingInfo & getSpacingInfo() const override;

    std::string getName() const override;

    void serialize(std::ostream & strm) const override;

private:

    /// True if the movie file is valid, false otherwise.
    bool m_valid;

    /// The shared pointer to the movie file that stores data.
    std::shared_ptr<MosaicMovieFile> m_file;

};

}
#endif // ISX_MOSAIC_MOVIE_H
