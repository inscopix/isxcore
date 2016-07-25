#ifndef ISX_MOSAIC_MOVIE_H
#define ISX_MOSAIC_MOVIE_H

#include "isxWritableMovie.h"
#include <fstream>

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
{
public:

    /// The access type of the file (e.g. read only).
    enum class FileAccessType
    {
        NONE,
        READ_ONLY,
        READ_WRITE
    };

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

    std::string getName() override;

    void serialize(std::ostream & strm) const override;

private:

    /// True if the movie file is valid, false otherwise.
    bool m_valid;

    /// The name of the movie file.
    std::string m_fileName;

    /// The timing information of the movie.
    TimingInfo m_timingInfo;

    /// The spacing information of the movie.
    SpacingInfo m_spacingInfo;

    /// The movie file handle.
    std::fstream m_file;

    /// The current access of the movie file.
    FileAccessType m_fileAccess;

    /// The header offset.
    std::ios::pos_type m_headerOffset;

    /// Opens the movie file for reading.
    ///
    /// If the file is already open (for reading or writing) this does
    /// nothing.
    ///
    /// \throw  isx::ExceptionFileIO    If opening the movie file fails.
    void openForReadOnly();

    /// Opens the movie file for writing.
    ///
    /// If the file is already open for reading, this closes the file and
    /// opens it in write mode.
    /// If the file is already open for writing, this does nothing.
    ///
    /// \throw  isx::ExceptionFileIO    If opening the movie file fails.
    void openForReadWrite();

    /// Read the header to populate information about the movie.
    ///
    /// This leaves the file open where the get position is just after the
    /// header.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the header from the file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the header fails.
    void readHeader();

    /// Write the header containing information about the movie.
    ///
    /// This leaves the file open where the put position is just after the
    /// header.
    void writeHeader();

    /// Writes zero data to a file for initialization purposes.
    ///
    /// This assumes the file is already open with write access with a put
    /// position that is just after the header.
    void writeZeroData();
};

}
#endif // ISX_MOSAIC_MOVIE_H
