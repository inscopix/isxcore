#ifndef ISX_BEHAV_MOVIE_FILE_H
#define ISX_BEHAV_MOVIE_FILE_H

#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxVideoFrame.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxDataSet.h"
#include "isxAsync.h"

#include <limits>

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
    /// \param  inFileName      The name of the movie file.
    /// \param  inProperties    The properties for this movie, including start time, # frames,
    ///                         and gopsize (cached so we don't have to scan the file every time
    ///                         an object is instantiated)
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the movie file fails.
    BehavMovieFile(const std::string & inFileName, const DataSet::Properties & inProperties);

    /// Destructor
    ///
    ~BehavMovieFile();
    
    /// Retrieve properties (# frames and gopsize) for a behavioral movie with the given filename.
    ///
    /// \param inFileName       The name of the movie file.
    /// \param outProperties    Reference of a Properties object to fill with # frames and gopsize.
    /// \param inCheckInCB      Callback function to call periodically to report progress and check
    ///                         for cancellation.
    static
    bool
    getBehavMovieProperties(const std::string & inFileName, DataSet::Properties & outProperties, AsyncCheckInCB_t inCheckInCB = nullptr);
    
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

private:
    /// Read constructor, used to create instance and get # frames and gopsize from it.
    ///
    /// \param  inFileName      The name of the movie file.
    ///
    BehavMovieFile(const std::string & inFileName);

    bool
    scanAllFrames(int64_t & outFrameCount, int64_t & outGopSize, AsyncCheckInCB_t inCheckInCB);

    bool
    initializeFromStream(const Time & inStartTime, int64_t inGopSize, int64_t inNumFrames);

    /// \return duration of number of frames in units of ffmpeg's time_base
    /// used for calculating time stamps
    int64_t
    timeBaseUnitsForFrames(isize_t inFrameNumber) const;

    /// \return true if the two given Presentation Time Stamps (pts) match
    bool
    isPtsMatch(int64_t inTargetPts, int64_t inTestPts) const;
    
    /// Seek to given frame in stream (if needed) and read packet
    ///
    int64_t
    seekFrameAndReadPacket(isize_t inFrameNumber);

    /// Find next packet for given stream index
    void
    readPacketFromStream(int inStreamIndex, const std::string & inContextForError);

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
    
    // ffmpeg
    AVFormatContext *           m_formatCtx = nullptr;
    AVCodecContext *            m_videoCodecCtx  = nullptr;
    int                         m_videoStreamIndex = 0;
    AVStream *                  m_videoStream = nullptr;
    Ratio                       m_videoPtsFrameDelta;
    int64_t                     m_videoPtsStartOffset = 0;
    
    std::unique_ptr<AVPacket>   m_pPacket;
    Ratio                       m_timeBase;
    
    isize_t                     m_lastVideoFrameNumber = 0;
    int64_t                     m_lastPktPts = -1;

    isize_t                     m_gopSize = 0;
};

} // namespace isx
#endif // ISX_BEHAV_MOVIE_FILE_H
