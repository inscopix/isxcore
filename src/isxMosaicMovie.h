#ifndef ISX_MOSAIC_MOVIE_H
#define ISX_MOSAIC_MOVIE_H

#include "isxWritableMovie.h"
#include "isxMosaicMovieFile.h"

#include <memory>

namespace isx
{

/// Encapsulates movie information and data.
///
/// All information is read from and written to one file.
/// The file stores the information or meta-data as a JSON formatted
/// string header.
/// The file stores the movie frame data in uncompressed binary form
/// after the header.
class MosaicMovie : public WritableMovie
                  , public std::enable_shared_from_this<MosaicMovie>
{
public:

    /// Empty constructor.
    ///
    /// This creates a valid c++ object but an invalid movie.
    MosaicMovie();

    /// Read constructor.
    ///
    /// This opens an existing movie file for read access.
    ///
    /// \param  inFileName  The name of the movie file.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the movie file fails.
    MosaicMovie(const std::string & inFileName);

    /// Write constructor.
    ///
    /// This opens a new file for read/write access and initializes the movie
    /// data with zeros.
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

    /// Destructor.
    ///
    ~MosaicMovie();

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

typedef std::shared_ptr<MosaicMovie> SpMosaicMovie_t;
typedef std::weak_ptr<MosaicMovie> WpMosaicMovie_t;

}
#endif // ISX_MOSAIC_MOVIE_H
