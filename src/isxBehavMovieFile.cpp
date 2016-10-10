
#include "isxBehavMovieFile.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}


#include <cmath>

namespace isx
{

BehavMovieFile::BehavMovieFile()
{}

BehavMovieFile::BehavMovieFile(const std::string & inFileName)
{
    m_fileName = inFileName;

    av_register_all();  // aschildan 10/10/2016: could/should be moved to coreInitialize

    if (avformat_open_input(&m_formatCtx, m_fileName.c_str(), nullptr, nullptr) != 0)
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open movie file: ", m_fileName);
    }

    if (avformat_find_stream_info(m_formatCtx, nullptr) < 0)
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to get stream info: ", m_fileName);
    }

    // find first video stream
    AVStream * firstVideoStream = nullptr;
    for (uint32_t i = 0; i < m_formatCtx->nb_streams; ++i)
    {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            firstVideoStream = m_formatCtx->streams[i];
            break;
        }
    }

    if (firstVideoStream == nullptr)
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to find video stream: ", m_fileName);
    }
    
    AVCodec * pCodec = avcodec_find_decoder(firstVideoStream->codecpar->codec_id);
    if (pCodec == nullptr)
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Unsupported video format: ", m_fileName);
    }
    
    m_codecCtx = avcodec_alloc_context3(pCodec);
    if (m_codecCtx == nullptr)
    {
        ISX_THROW(isx::ExceptionFileIO,
                  "Can't alloc decoder context: ", m_fileName);
    }

    AVDictionary * optionsDict = nullptr;
    if (avcodec_open2(m_codecCtx, pCodec, &optionsDict) < 0)
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open video codec: ", m_fileName);
    }

    m_spacingInfo = SpacingInfo(isx::SizeInPixels_t(firstVideoStream->codecpar->width, firstVideoStream->codecpar->height));

    auto startTime = Time();
    Ratio frameRate(firstVideoStream->avg_frame_rate.num, firstVideoStream->avg_frame_rate.den);
    Ratio avTimeBase(firstVideoStream->time_base.num, firstVideoStream->time_base.den);
    Ratio avDuration = Ratio(firstVideoStream->duration, 1);
    Ratio durationInSeconds = (avDuration * avTimeBase);
    
    double numFramesD = (durationInSeconds * frameRate).toDouble();
    isize_t numFrames = isize_t(std::floor(numFramesD));
    m_timingInfo = TimingInfo(startTime, frameRate.getInverse(), numFrames);
    
    m_valid = true;

    int iBreakpoint = 0;
    ++iBreakpoint;

}
    
BehavMovieFile::~BehavMovieFile()
{
    avcodec_free_context(&m_codecCtx);
    avformat_close_input(&m_formatCtx);
}

/// \return True if the movie file is valid, false otherwise.
///
bool 
BehavMovieFile::isValid() const
{
    return m_valid;
}

/// Read a frame in the file by index.
///
/// \param  inFrameNumber   The index of the frame.
/// \return                 The frame read from the file.
///
/// \throw  isx::ExceptionFileIO    If reading the movie file fails.
/// \throw  isx::ExceptionDataIO    If inFrameNumber is out of range.
SpVideoFrame_t
BehavMovieFile::readFrame(isize_t inFrameNumber)
{
    ISX_ASSERT(!"not implemented");
    return SpVideoFrame_t();
}

const
std::string &
BehavMovieFile::getFileName() const
{
    return m_fileName;
}

/// \return     The timing information read from the movie.
///
const
isx::TimingInfo &
BehavMovieFile::getTimingInfo() const
{
    return m_timingInfo;
}

/// \return     The spacing information read from the movie.
///
const
isx::SpacingInfo &
BehavMovieFile::getSpacingInfo() const
{
    return m_spacingInfo;
}

/// \return     The data type of a pixel value.
///
DataType
BehavMovieFile::getDataType() const
{
    return m_dataType;
}

/// \return     The size of a pixel value in bytes.
///
isize_t
BehavMovieFile::getPixelSizeInBytes() const
{
    ISX_ASSERT(!"not implemented");
    return 0;
}

/// \return     The size of a row in bytes.
///
isize_t
BehavMovieFile::getRowSizeInBytes() const
{
    ISX_ASSERT(!"not implemented");
    return 0;
}

/// \return     The size of a frame in bytes.
///
isize_t
BehavMovieFile::getFrameSizeInBytes() const
{
    ISX_ASSERT(!"not implemented");
    return 0;
}

/// Seek to the location of a frame for reading.
///
/// \param  inFile          The input file stream whose input position
///                         will be modified to be at the given frame number.
/// \param  inFrameNumber   The number of the frame to which to seek.
void
BehavMovieFile::seekForReadFrame(
    std::ifstream & inFile,
    isize_t inFrameNumber)
{
    ISX_ASSERT(!"not implemented");
}

} // namespace isx
