#ifndef ISX_BEHAV_MOVIE_FILE_H
#define ISX_BEHAV_MOVIE_FILE_H

#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxVideoFrame.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"

// ffmpeg forwards
struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVPacket;

namespace isx
{

/// Encapsulates behavioral movie information and data in a file.
///
class BehavMovieFile
{
public:

    /// Empty constructor.
    ///
    /// This creates a valid c++ object but an invalid movie file.
    BehavMovieFile();

    /// Read constructor.
    ///
    /// This opens a movie file and populates TimingInfo and SpacingInfo
    /// objects.
    ///
    /// \param  inFileName  The name of the movie file.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the movie file fails.
    BehavMovieFile(const std::string & inFileName);

    /// Destructor
    ///
    ~BehavMovieFile();
    
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

    /// \return     The spacing information read from the movie.
    ///
    const
    isx::SpacingInfo &
    getSpacingInfo() const;

    /// \return     The data type of a pixel value.
    ///
    DataType
    getDataType() const;

private:
    bool
    initializeFromStream(int inIndex);

    /// \return duration of number of frames in units of ffmpeg's time_base
    /// used for calculating time stamps
    int64_t
    timeBaseUnitsForFrames(isize_t inFrameNumber) const;

    /// \return true if the two given pts match
    bool
    isPtsMatch(int64_t inPts1, int64_t inPts2) const;

    /// \return     The size of a pixel value in bytes.
    ///
    isize_t
    getPixelSizeInBytes() const;

    /// \return     The size of a row in bytes.
    ///
    isize_t
    getRowSizeInBytes() const;

    /// \return     The size of a frame in bytes.
    ///
    isize_t
    getFrameSizeInBytes() const;

    /// \return     Start time used in timing info.
    /// This is a hack needed until we have a way of requesting the user to define a start time on file import
    Time getStartTime() const;

    /// Seek to the location of a frame for reading.
    ///
    /// \param  inFile          The input file stream whose input position
    ///                         will be modified to be at the given frame number.
    /// \param  inFrameNumber   The number of the frame to which to seek.
    void
    seekForReadFrame(
        std::ifstream & inFile,
        isize_t inFrameNumber);

    /// True if the movie file is valid, false otherwise.
    bool m_valid = false;

    /// The name of the movie file.
    std::string m_fileName;

    /// The timing information of the movie.
    TimingInfo m_timingInfo;

    /// The spacing information of the movie.
    SpacingInfo m_spacingInfo;

    /// The data type of the pixel values.
    DataType m_dataType = DataType::U8;
    
    
    // ffmpeg
    AVFormatContext *           m_formatCtx = nullptr;
    AVCodecContext *            m_videoCodecCtx  = nullptr;
    int                         m_videoStreamIndex = 0;
    AVStream *                  m_videoStream = nullptr;
    Ratio                       m_videoPtsFrameDelta;
    int64_t                     m_videoPtsStartOffset = 0;
    
    std::unique_ptr<AVPacket>   m_pPacket;
    Ratio                       m_timeBase;
    
    isize_t                     m_lastVideoFrameNumber = std::numeric_limits<uint64_t>::max();
    int64_t                     m_lastPktPts = -1;

};

} // namespace isx
#endif // ISX_BEHAV_MOVIE_FILE_H
