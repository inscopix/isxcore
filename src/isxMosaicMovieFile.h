#ifndef ISX_MOSAIC_MOVIE_FILE_H
#define ISX_MOSAIC_MOVIE_FILE_H

#include "isxMovieDefs.h"
#include "isxVideoFrame.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"

#include <fstream>

namespace isx
{

/// Encapsulates movie information and data in a file.
///
/// All information is read from and written to one file.
/// The file stores the information or meta-data as a JSON formatted
/// string header.
/// The file stores the movie frame data in uncompressed binary form
/// after the header.
class MosaicMovieFile
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
    MosaicMovieFile();

    /// Read constructor.
    ///
    /// This opens an existing movie file for read access.
    ///
    /// \param  inFileName  The name of the movie file.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the movie file fails.
    MosaicMovieFile(const std::string & inFileName);

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
    MosaicMovieFile(const std::string & inFileName,
                const TimingInfo & inTimingInfo,
                const SpacingInfo & inSpacingInfo);

    /// Destructor.
    ///
    ~MosaicMovieFile();

    /// \return True if the movie file is valid, false otherwise.
    ///
    bool isValid() const;

    /// Read a frame in the file by index.
    ///
    /// \param  inFrameNumber   The index of the frame.
    /// \return                 The frame read from the file.
    SpU16VideoFrame_t readFrame(isize_t inFrameNumber);

    /// Write a frame to the file.
    ///
    /// \param  inVideoFrame    The frame to write to the file.
    void writeFrame(const SpU16VideoFrame_t & inVideoFrame);

    /// Initialize for reading.
    ///
    /// \param  inFileName  The name of the movie file.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the movie file fails.
    void initialize(const std::string & inFileName);

    /// Initialize for writing.
    ///
    /// \param  inFileName      The name of the movie file.
    /// \param  inTimingInfo    The timing information of the movie.
    /// \param  inSpacingInfo   The spacing information of the movie.
    ///
    /// \throw  isx::ExceptionFileIO    If writing the movie file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the movie data fails.
    void initialize(const std::string & inFileName,
                    const TimingInfo & inTimingInfo,
                    const SpacingInfo & inSpacingInfo);

    /// \return     The name of the file.
    ///
    std::string getFileName() const;

    /// \return     The timing information read from the movie.
    ///
    const isx::TimingInfo & getTimingInfo() const;

    /// \return     The spacing information read from the movie.
    ///
    const isx::SpacingInfo & getSpacingInfo() const;

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
#endif // ISX_MOSAIC_MOVIE_FILE_H
