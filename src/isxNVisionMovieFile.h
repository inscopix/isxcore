#ifndef ISX_NVISION_MOVIE_FILE_H
#define ISX_NVISION_MOVIE_FILE_H

#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxVideoFrame.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include <fstream>

// ffmpeg forwards
struct AVFormatContext;
struct AVCodecContext;
struct AVPacket;
struct AVFrame;

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
    /// \param inEnableWrite    Flag indicating whether the file should be open for read & write.
    ///                         This used for updating metadata on existing isxb movies.
    ///
    NVisionMovieFile(
        const std::string & inFilename,
        const bool inEnableWrite = false
    );

    /// Write constructor.
    ///
    /// \param  inFileName      The name of the movie file.
    /// \param inTimingInfo     The timing info of the movie.
    /// \param inSpacingInfo    The spacing info of the movie
    ///
    /// \throw  isx::ExceptionFileIO    If opening the file for write fails.
    /// \throw  isx::ExceptionUserInput     If the width of the movie is not a multiple of 64 - see isResolutionSupported for more info.
    //?
    ///
    NVisionMovieFile(
        const std::string & inFilename,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo);

    /// Destructor
    ///
    ~NVisionMovieFile();

    /// Checks if the resolution of the movie is supported for this file format.
    // The MJPEG encoder only supports video frames that have a width which is a multiple of 64.
    ///
    static bool isResolutionSupported(const SpacingInfo & inSpacingInfo);

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
    /// \throw  isx::ExceptionUserInput    If inFrameNumber is out of range.
    /// \throw  isx::ExceptionUserInput    If the file is open for write
    ///
    SpVideoFrame_t
    readFrame(isize_t inFrameNumber);

    /// Writes a frame in the file.
    ///
    /// \throw  isx::ExceptionFileIO    If writing the movie file fails.
    /// \throw  isx::ExceptionUserInput    If the file is open for read
    ///
    void
    writeFrame(const SpVideoFrame_t & inFrame);

    /// Read metadata for a particular frame in the file.
    ///
    /// \param  inFrameNumber   The index of the frame.
    /// \return                 The frame metadata read from the file.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionUserInput    If inFrameNumber is out of range.
    ///
    std::string
    readFrameMetadata(const isize_t inFrameNumber);

    /// Write metadata for the frame written to the file.
    ///
    /// \param inFrameMetadata  The metadata for the frame that was just written to the file
    ///
    /// \throw  isx::ExceptionUserInput    If frame metadata has been written for all frames in the file already.
    ///
    void
    writeFrameMetadata(const std::string inFrameMetadata);

    /// \return     True if the movie has specific timestamp (e.g. TSC) for each frame,
    ///             false otherwise.
    bool hasFrameTimestamps() const;
    
    /// Read the timestamp associated with a frame on the calling thread.
    ///
    /// \param  inIndex     The index of a frame in this movie.
    /// \return             The timestamp associated with the given frame,
    ///                     or 0 if it does not have one.
    uint64_t readFrameTimestamp(const isize_t inFrameNumber);

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

    /// \return The extra properties formatted as a JSON string.
    /// This is stored in the session metadata section of the file.
    ///
    std::string
    getExtraProperties() const;
    
    /// Set the extra properties formatted as a JSON string.
    /// This is stored in the session metadata section of the file.
    ///
    /// \param inExtraProperties        The session metadata formatted as a JSON string.
    void
    setExtraProperties(const std::string inExtraProperties);

    void
    closeForWriting();

    /// Struct representing file header contents
    ///
    struct Header {
        uint64_t m_fileVersion = 0; /// File format version.
        uint64_t m_headerSize = 0; /// Header size.

        uint64_t m_epochMs = 0; /// Unix epoch time in ms.
        int64_t  m_utcOffset = 0; /// Utc offset time in mins.

        uint64_t m_numFrames = 0; /// Total number of valid frames (excluding dropped frame).
        uint64_t m_numDrops = 0; /// Number of dropped frames.

        uint64_t m_videoOffset = 0; /// Absolute byte offset of video data.
        uint64_t m_videoSize = 0; /// Video data size in bytes.

        uint64_t m_metaOffset = 0; /// Absolute byte offset of meta data.
        uint64_t m_metaSize = 0; /// Metadata size in bytes.

        uint64_t m_sessionOffset = 0; /// Absolute byte offset of session data.
        uint64_t m_sessionSize = 0; /// Session data size in bytes.
    };

private:
    /// Initialize the file stream which reads metadata segments from the file.
    ///
    /// \param inOpenMode   Specifies the mode to open the file.
    void initializeFileStream(const std::ios_base::openmode inOpenMode);

    /// Initializes the codec to be used for decoding compressed video data read from the file.
    ///
    void initializeDecoder();

    /// Initializes the codec to be used for encoding compressed video data to be written to the file.
    ///
    void initializeEncoder();

    /// Reads the file header to determine how to read metadata segments from the file.
    ///
    void readHeader();

    /// Write the file header which determines how to read metadata segments from the file.
    ///
    void writeHeader();

    // /// Reads the segment of the file storing per-frame metadata.
    // /// This initializes the timing info of the movie.
    // /// Reads the segment of the file storing session metadata.
    // /// This initialize the spacing info of the movie.
    void readMetadata();

    void writeMetadata();

    /// Decodes current packet in order to get decompressed video frame
    ///
    /// \param inFrameNumber    The index of the frame used to set the timing info of the output video frame.
    /// \return                 A pointer to the video frame containing the decoded data
    ///
    SpVideoFrame_t
    decodePacket(size_t inFrameNumber, AVPacket * m_pPacket);

    /// Return a video frame compatible with current video stream's dimensions and
    /// with timing info set according to the input frame number.
    ///
    /// \param inIndex    The index of the frame used to set the timing info of the output video frame.
    /// \return                 A pointer to the black video frame
    ///
    SpVideoFrame_t
    makeVideoFrame(const isize_t inIndex) const;

    /// Check if the file stream is good, if not throw an exception.
    ///
    /// \param inMessage    The message for the exception to throw.
    ///
    void
    checkFileGood(const std::string & inMessage) const;

    /// True if the movie file is valid, false otherwise.
    bool m_valid = false;

    /// The name of the movie file.
    std::string m_fileName;

    /// True if the file is opened for writing.    
    bool m_enableWrite = false;

    /// The timing information of the movie.
    TimingInfos_t m_timingInfos;

    /// The spacing information of the movie.
    SpacingInfo m_spacingInfo;

    /// The data type of the pixel values.
    DataType m_dataType = DataType::U8;

    /// The extra properties from the session segment.
    std::string m_extraProperties;

    /// The file stream
    std::fstream m_file;

    /// Contents of file header.
    Header m_header;
    
    /// Size of the file header in bytes.
    const static size_t s_headerSizeInBytes = 96;

    /// Stores header information about the format of the video container
    AVFormatContext *           m_formatCtx = nullptr;

    /// Stores information about the codec to use for decoding compressed video data
    AVCodecContext *            m_videoCodecCtx  = nullptr;

    /// Buffer to store video data which is sent to the codec for encoding.
    AVFrame *                   m_avFrame = nullptr;

    /// Index of the first video stream in the container
    int                         m_videoStreamIndex = 0;

    // /// Vector of frame metadata extracted from the per-frame metadata section of the file
    std::vector<std::string> m_frameMetadatas;

    /// Index of the previous frame that was read from the file
    size_t m_previousFrameNumber = 0;

    /// Version of the file format.
    const static uint64_t m_fileVersion = 100;
};

} // namespace isx

#endif // ISX_NVISION_MOVIE_FILE_H
