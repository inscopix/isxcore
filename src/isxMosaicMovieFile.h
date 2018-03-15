#ifndef ISX_MOSAIC_MOVIE_FILE_H
#define ISX_MOSAIC_MOVIE_FILE_H

#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxVideoFrame.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxJsonUtils.h"

#include <ios>
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

    /// Empty constructor.
    ///
    /// This creates a valid c++ object but an invalid movie file.
    MosaicMovieFile();

    /// Read constructor.
    ///
    /// This opens an existing movie file and reads information from its
    /// header.
    ///
    /// \param  inFileName  The name of the movie file.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the movie file fails.
    MosaicMovieFile(const std::string & inFileName);

    /// Write constructor.
    ///
    /// This opens a new file, writes header information and initializes the
    /// movie data with zeros.
    ///
    /// \param  inFileName      The name of the movie file.
    /// \param  inTimingInfo    The timing information of the movie.
    /// \param  inSpacingInfo   The spacing information of the movie.
    /// \param  inDataType      The pixel value data type.
    ///
    /// \throw  isx::ExceptionFileIO    If writing the movie file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the movie data fails.
    MosaicMovieFile(const std::string & inFileName,
                const TimingInfo & inTimingInfo,
                const SpacingInfo & inSpacingInfo,
                DataType inDataType,
                const bool inWriteFrameTimeStamps = false);

    /// Default destructor
    ///
    ~MosaicMovieFile();

    /// \return True if the movie file is valid, false otherwise.
    ///
    bool isValid() const;
    
    /// Close this file for writing.  This writes the header containing
    /// metadata at the end of the file.  Any attempts to write frames after
    /// this is called will result in an exception.
    ///
    /// \param inTimingInfo  a new timing information object to be set in the file
    /// before finishing writing it
    void
    closeForWriting(const TimingInfo & inTimingInfo = TimingInfo());

    /// Read a frame in the file by index.
    ///
    /// \param  inFrameNumber       The index of the frame.
    /// \param  inWithHeaderFooter  If true, create a frame a with header/footer rows.
    /// \return                     The frame read from the file.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionDataIO    If inFrameNumber is out of range.
    SpVideoFrame_t readFrame(isize_t inFrameNumber, const bool inWithHeaderFooter = false);

    /// \param  inFrameNumber   The index of the frame.
    /// \return                 The frame with the header and footer if it exists.

    /// Write a frame to the file.
    ///
    /// \param  inVideoFrame    The frame to write to the file.
    ///
    /// \throw  isx::ExceptionDataIO    If the frame data type does not match
    ///                                 the movie data type.
    /// \throw  isx::ExceptionFileIO    If writing the movie file fails.
    ///
    /// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
    void writeFrame(const SpVideoFrame_t & inVideoFrame);

    /// \return     The name of the file.
    ///
    std::string getFileName() const;

    /// \return     The timing information read from the movie.
    ///
    const isx::TimingInfo & getTimingInfo() const;

    /// \return     The TimingInfos_t of a MovieSeries.
    ///             For a regular movie this will contain one TimingInfo object
    ///             matching getTimingInfo.
    ///
    virtual
    const isx::TimingInfos_t &
    getTimingInfosForSeries() const;
    
    /// \return     The spacing information read from the movie.
    ///
    const isx::SpacingInfo & getSpacingInfo() const;

    /// \return     The data type of a pixel value.
    ///
    DataType getDataType() const;

    /// \param  inIndex             The index of the frame to generate.
    /// \param  inWithHeaderFooter  If true, create a frame a with header/footer rows.
    /// \return                     The frame associated with the given index.
    SpVideoFrame_t makeVideoFrame(const isize_t inIndex, const bool inWithHeaderFooter = false) const;

    /// \param  inProperties    The extra properties formatted as a JSON string.
    ///
    void
    setExtraProperties(const std::string & inProperties);

    /// \return The extra properties of formatted as a JSON string.
    ///
    std::string
    getExtraProperties() const;

private:
    /// True if the movie file is valid, false otherwise.
    bool m_valid;

    /// The name of the movie file.
    std::string m_fileName;

    /// The timing information of the movie.
    TimingInfos_t m_timingInfos;

    /// The spacing information of the movie.
    SpacingInfo m_spacingInfo;

    /// The header offset.
    std::ios::pos_type m_headerOffset;

    /// The data type of the pixel values.
    DataType m_dataType;

    /// The file stream
    std::fstream m_file;

    bool m_fileClosedForWriting = false;

    /// The version of this file format.
    const static size_t s_version = 1;

    /// True if the frame bytes contain fixed size header and footer lines.
    bool m_hasFrameHeaderFooter = false;

    /// The number of rows in the header.
    const static size_t s_numHeaderRows = 2;

    /// The number of rows in the footer.
    const static size_t s_numFooterRows = s_numHeaderRows;

    /// The number of rows in the header and footer.
    const static size_t s_numHeaderFooterRows = s_numHeaderRows + s_numFooterRows;

    /// The extra properties to write in the JSON footer.
    json m_extraProperties;

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
    /// \param  inDataType      The pixel value data type.
    ///
    /// \throw  isx::ExceptionFileIO    If writing the movie file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the movie data fails.
    void initialize(const std::string & inFileName,
                    const TimingInfo & inTimingInfo,
                    const SpacingInfo & inSpacingInfo,
                    DataType inDataType,
                    const bool inWriteFrameTimeStamps);

    /// Read the header to populate information about the movie.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the header from the file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the header fails.
    void readHeader();

    /// Write the header containing information about the movie.
    ///
    /// This truncates the file when opening it.
    void writeHeader();

    /// \return     The size of a pixel value in bytes.
    ///
    isize_t getPixelSizeInBytes() const;

    /// \return     The size of a row in bytes.
    ///
    isize_t getRowSizeInBytes() const;

    /// \return     The size of a frame in bytes.
    ///
    isize_t getFrameSizeInBytes() const;

    /// Seek to the location of a frame for reading.
    ///
    /// \param  inFrameNumber   The number of the frame to which to seek.
    /// \param  inSkipHeader    If true, skips the frame header if it exists,
    ///                         otherwise include the header.
    void seekForReadFrame(isize_t inFrameNumber, const bool inSkipHeader);

    /// Flush the stream
    ///
    void flush();

    /// Set a new timing info
    /// Notice that is only called from closeForWriting() as it is 
    /// intended to be used during data acquisition, when the client has 
    /// finished writing frame data. 
    void setTimingInfo(const TimingInfo & inTimingInfo);
};

}
#endif // ISX_MOSAIC_MOVIE_FILE_H
