#ifndef ISX_NVISION_MOVIE_FILE_H
#define ISX_NVISION_MOVIE_FILE_H

#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxVideoFrame.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"

namespace isx
{

/// Object representing the file format of nVision behavioural movies
/// nVision movies have an .isxb extension
/// The file format specification is available here: https://inscopix.atlassian.net/l/c/1CaeAXiX
/// The movie consists of 4 sections:
/// 1. Header containing timing info + offsets for remaining segments in the file.
/// 2. Video data stored in an MJPEG container.
/// MJPEG is a simple video container which contains a series of frames that are each individually compressed using JPEG.
/// Dropped frames are not stored in the file.
/// 3. Per-frame metadata in JSON format.
/// This includes TSC values for each frame, which can be used for playback synchronization.
/// An average sampling rate is calculated using the timestamps of the first and last valid frames.
/// This section may also contain metadata about COM, behavioural events, zone occupancy, etc.
/// 4. Session metadata in JSON format.
/// This includes information about the acquisition settings from IDAS.
///
class NVisionMovieFile
{
public:
    /// Read constructor.
    ///
    /// \param  inFileName      The name of the movie file.
    ///
    NVisionMovieFile(const std::string & inFileName);

    /// \return True if the movie file is valid, false otherwise.
    ///
    bool
    isValid() const;

    /// Read a frame in the file by index.
    ///
    /// \param  inFrameNumber   The index of the frame.
    /// \return                 The frame read from the file.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionDataIO    If inFrameNumber is out of range.
    SpVideoFrame_t
    readFrame(isize_t inFrameNumber);

    /// \return     The name of the file.
    ///
    const
    std::string &
    getFileName() const;

    /// \return     The timing information read from the movie.
    ///
    const
    isx::TimingInfo &
    getTimingInfo() const;

    /// \return     The TimingInfos_t of a MovieSeries.
    ///             For a regular movie this will contain one TimingInfo object
    ///             matching getTimingInfo.
    ///
    const isx::TimingInfos_t &
    getTimingInfosForSeries() const;

    /// \return     The spacing information read from the movie.
    ///
    const
    isx::SpacingInfo &
    getSpacingInfo() const;

    /// \return     The data type of a pixel value.
    ///
    DataType
    getDataType() const;

    /// Struct representing file header contents
    struct Header {
        uint64_t m_fileVersion; /// File format version.
        uint64_t m_headerSize; /// Header size.

        uint64_t m_epochMs; /// Unix epoch time in ms.
        int64_t  m_utcOffset; /// Utc offset time in mins.

        uint64_t m_numFrames; /// Total number of valid frames (excluding dropped frame).
        uint64_t m_numDrops; /// Number of dropped frames.

        uint64_t m_videoOffset; /// Absolute byte offset of video data.
        uint64_t m_videoSize; /// Video data size in bytes.

        uint64_t m_metaOffset; /// Absolute byte offset of meta data.
        uint64_t m_metaSize; /// Metadata size in bytes.

        uint64_t m_sessionOffset; /// Absolute byte offset of session data.
        uint64_t m_sessionSize; /// Session data size in bytes.
    };

private:
    /// Initialize the file stream which reads metadata segments from the file.
    ///
    void initializeFileStream();

    /// Reads the file header to determine how to read metadata segments from the file.
    ///
    void readHeader();

    /// Reads the segment of the file storing per-frame metadata.
    /// This initializes the timing info of the movie.
    ///
    void readMetadataSegment();

    /// Reads the segment of the file storing session metadata.
    /// This initialize the spacing info of the movie.
    void readSessionSegment();    

    /// Return black frame compatible with current video stream's dimensions and with
    /// timing info set according to inFrameNumber.
    SpVideoFrame_t
    getBlackFrame(isize_t inFrameNumber);

    /// True if the movie file is valid, false otherwise.
    bool m_valid = false;

    /// The name of the movie file.
    std::string m_fileName;

    /// The timing information of the movie.
    TimingInfos_t m_timingInfos;

    /// The spacing information of the movie.
    SpacingInfo m_spacingInfo;

    /// The data type of the pixel values.
    DataType m_dataType = DataType::U8;

    /// The file stream
    std::fstream m_file;

    /// Contents of file header.
    Header m_header;
    
    /// Size of the file header in bytes.
    const static size_t s_headerSizeInBytes = 96;
};

} // namespace isx

#endif // ISX_NVISION_MOVIE_FILE_H
