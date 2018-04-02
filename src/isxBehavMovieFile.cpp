
#include "isxBehavMovieFile.h"
#include "isxPathUtils.h"
#include "isxMovie.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}


#include <cstring>
#include <cmath>
#include <algorithm>
#include <limits>
#include <memory>
#include <fstream>

// Rough outline of what this code is doing (July 24th 2017)
// Compressed video is usually stored in a "container" file. The container holds 
// any number of compressed video streams, plus compressed audio streams plus
// subtitles and other data streams.
// FFMPEG retrieves the compressed data from the container file in "packets".
// A packet contains either compressed video or compressed audio. 
// Since we currently do not support audio playback we ignore any audio packets.
// Frames in a video stream are usually compressed as either
// I - "Intra-coded picture” - this is full frame encoded similar to JPG (lossy).
// P - "Predicted picture” - this holds only the changes from the previous frame.
// B - “Bi-directional predicted picture” - this holds difference from previous
//     and following (future) frames. 
// We do not support video streams that contain B frames.
// A video packet usually contains data for one frame. It has a PTS 
// (presentation time stamp) and a DTS (decoding time stamp).  Most times are
// in units of “time base” which is retrieved from the stream at init time.
// We currently ignore the DTS, as far as I know it is only needed for streams
// with B frames, because only when there are B frames the PTS of subsequent
// packets in the stream are not strictly increasing (but presumably the DTS
// are).  
// The distance (in number of frames) between two I frames in a video stream
// is called the GOP size (Group Of Pictures).  Decoding (after seeking) has to 
// start with an I frame or there is corruption due to insufficient frame data.
// The larger the GOP size, the better compression can be achieved at the cost
// of seek performance.  I found that a GOP size of about 10 results in decent
// seek performance.  Anything larger will cause us to issue a warning to the
// user when importing.  The conversion (to canonical format) with ffmpeg sets
// the GOP size to 10 and removes all B frames.
// In some of the sample videos I saw a GOP size of 300.
// 
// Code overview:
// FFMPEG initialization (ctor & initializeFromStream) follows largely what is
// done in other sample code or what the documentation says.  
// AVFormatContext deals with the container file and reading packets from a 
//     stream.
// AVCodecContext deals with decoding packets into frames.
// AVStream describes a videos stream (frame rate, time base, duration etc).
// AVPacket contains compressed video data, needs to be explicitly released.
//
// Two constructors:
// 1 - with filename only is used during import.
// We scan (scanAllFrames) through the imported file, reading all packets with
// FFMPEG but not doing any decoding (which would be much slower).  We look
// for B frames, the GOP size, and count the total number of frames (video
// packets). GOP size and number of frames is stored in the Dataset’s properties.
// 2 - with filename and properties is used during visualization.
//
// Regular playback (from start, no seeking):
// readFrame returns an isx::VideoFrame given a frame index.  The frame index
// is passed to seekFrameAndReadPacket which during regular playback just reads
// the next packet into m_pPacket and then sends it to the video codec for 
// decoding (avcodec_send_packet).
// Upon return from seekFrameAndReadPacket, readFrame tries to receive the 
// decoded frame (avcodec_receive_frame). If the PTS matches, the frame data is
// copied into the isx::VideoFrame.
// According to the documentation (probably out-dated), video packets may
// may contain data for multiple video frames, so there is a loop issuing
// more avcodec_receive_frame calls until PTS match is found. I think this loop
// is actually not necessary.
// 
// Seeking:
// If seekFrameAndReadPacket determines that the requested frame index does
// not match next packet’s PTS, we seek. Decoding needs to start with I frames, so
// we always seek to at least beyond the requested PTS plus (or minus - depending
// on seek direction) the number frames per GOP.  Seeking is done in three stages:
// A - seek with FFMPEG avformat_seek_file,
// B - then check if the PTS of the next packet is “early” enough. I found that
//     sometimes avformat_seek_file would not seek far enough so I continue to
//     decrease the “seekFrameNumber” until we get a packet PTS that is not 
//     greater than what we requested.
// C - start reading and decoding packets until we find the packet PTS that 
//     matches the frame index that was requested (this is the outer while
//     loop in readFrame).


//#if ISX_OS_MACOS
//#pragma clang diagnostic ignored "-Wdeprecated-declarations"
//#endif

// set to 1 to turn on behavioral frame read debug logging
#if 0
#define ISX_BEHAV_READ_DEBUG_LOGGING 1
#define ISX_BEHAV_READ_LOG_DEBUG(...) ISX_LOG_DEBUG(__VA_ARGS__)
#else
#define ISX_BEHAV_READ_DEBUG_LOGGING 0
#define ISX_BEHAV_READ_LOG_DEBUG(...)
#endif

namespace
{
    std::string
    isxAvErrorCodeToString(int inErrorCode)
    {
        char buf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(inErrorCode, &buf[0], AV_ERROR_MAX_STRING_SIZE);
        return std::string(&buf[0]);
    }

    const char errorMessageUserManual[] = "Import of behavioral video failed. Please refer to the User Manual - Behavioral Movie section for help on supported video formats and how to convert your files.";
}

namespace isx
{

BehavMovieFile::BehavMovieFile()
{}

BehavMovieFile::BehavMovieFile(const std::string & inFileName, const DataSet::Properties & inProperties)
    : BehavMovieFile(inFileName)
{
    Time startTime;
    int64_t gopSize = -1;
    int64_t numFrames = -1;
    if (inProperties.find(DataSet::PROP_MOVIE_START_TIME) != inProperties.end())
    {
        auto t = inProperties.at(DataSet::PROP_MOVIE_START_TIME);
        startTime = t.value<Time>();
    }
    else
    {
        ISX_THROW(isx::ExceptionFileIO, "Could not find start time property.");
    }

    if (inProperties.find(DataSet::PROP_BEHAV_GOP_SIZE) != inProperties.end())
    {
        auto t = inProperties.at(DataSet::PROP_BEHAV_GOP_SIZE);
        gopSize = t.value<int64_t>();
    }
    else
    {
        ISX_THROW(isx::ExceptionFileIO, "Could not find gop size property.");
    }

    if (gopSize > sMaxSupportedGopSize)
    {
        ISX_LOG_ERROR("Behavioral video import, GOP size over limit (", gopSize, " > ", sMaxSupportedGopSize, "): ", m_fileName);
    }

    if (inProperties.find(DataSet::PROP_BEHAV_NUM_FRAMES) != inProperties.end())
    {
        auto t = inProperties.at(DataSet::PROP_BEHAV_NUM_FRAMES);
        numFrames = t.value<int64_t>();
    }
    else
    {
        ISX_THROW(isx::ExceptionFileIO, "Could not find number of frames property.");
    }

    if (!initializeFromStream(startTime, gopSize, numFrames))
    {
        return;
    }

    m_dataType = DataType::U8; // TODO aschildan 10/19/2016: adjust this when we support color movies

    m_valid = true;
}

BehavMovieFile::BehavMovieFile(const std::string & inFileName)
{
    m_fileName = inFileName;

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
    int firstVideoStreamIndex = -1;
    for (uint32_t i = 0; i < m_formatCtx->nb_streams; ++i)
    {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            firstVideoStreamIndex = i;
            break;
        }
    }
    
    if (firstVideoStreamIndex == -1)
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to find video stream: ", m_fileName);
    }
    m_videoStreamIndex = firstVideoStreamIndex;
    m_pPacket.reset(new AVPacket);

    m_valid = true;
}
    
BehavMovieFile::~BehavMovieFile()
{
    avcodec_free_context(&m_videoCodecCtx);
    avformat_close_input(&m_formatCtx);
}

bool
BehavMovieFile::isValid() const
{
    return m_valid;
}

/// \return true if the two given Presentation Time Stamps (pts) match
bool
BehavMovieFile::isPtsMatch(int64_t inTargetPts, int64_t inTestPts) const
{
    if (inTestPts == AV_NOPTS_VALUE)
    {
        return false;
    }
    // fudge needed for a small number of frames that have an
    // actual pts that is earlier than their calculated pts (from frame number, frame rate and time base)
    // set fudge to 5% of m_videoPtsFrameDelta (pts delta between frames)
    int64_t fudge = int64_t(std::floor(m_videoPtsFrameDelta.toDouble() * 0.05));
    ISX_BEHAV_READ_LOG_DEBUG("isPtsMatch: ", inTestPts - inTargetPts);
    return inTestPts >= (inTargetPts - fudge);
}

void
BehavMovieFile::readPacketFromStream(int inStreamIndex, const std::string & inContextForError)
{
    bool isMatchingStream = false;

    while (!isMatchingStream)
    {
        auto readRet = av_read_frame(m_formatCtx, m_pPacket.get());
        if (readRet == AVERROR_EOF)
        {
            m_endOfFile = true;
            ISX_BEHAV_READ_LOG_DEBUG(inContextForError, " - EOF!");
            break;
        }
        else if (readRet < 0)
        {
            ISX_THROW(
                isx::ExceptionFileIO,
                inContextForError, " failed to read packet: ",
                m_fileName,
                " - ",
                isxAvErrorCodeToString(readRet));
        }
        isMatchingStream = m_pPacket && m_pPacket->stream_index == inStreamIndex;
        if (isMatchingStream)
        {
            if (m_pPacket->pts == AV_NOPTS_VALUE)
            {
                ISX_THROW(isx::ExceptionFileIO, "Video packet has no PTS: ", m_fileName);
            }
        }
    }

    return;
}
    
int64_t
BehavMovieFile::seekFrameAndReadPacket(isize_t inFrameNumber)
{
    const int64_t requestedPts = timeBaseUnitsForFrames(inFrameNumber) + m_videoPtsStartOffset;
    const int64_t deltaFromCurrent = requestedPts - m_lastPktPts;
    const int64_t deltaFromExpected = deltaFromCurrent - timeBaseUnitsForFrames(1);
    ISX_BEHAV_READ_LOG_DEBUG("deltaFromExpected: ", deltaFromExpected, ", thresh: ", int64_t(m_videoPtsFrameDelta.toDouble() + 0.5f));

    if (inFrameNumber == 0)
    {
        av_seek_frame(m_formatCtx, m_videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(m_videoCodecCtx);
        readPacketFromStream(m_videoStreamIndex, "BehavMovieFile::seekFrameAndReadPacket(0)");
    }
    else if (-deltaFromExpected > timeBaseUnitsForFrames(1)
             || deltaFromExpected > timeBaseUnitsForFrames(m_gopSize))
    {
        // We get here and start seeking if the next frame is more than one frame
        // before the expected frame or more than the gop-size after the expected frame

        if (inFrameNumber == m_lastVideoFrameNumber + 1)
        {
            ISX_BEHAV_READ_LOG_DEBUG("Last frame had timestamp for this frame, repeating it.");
        }

        int64_t seekFrameNumber = int64_t(inFrameNumber) - int64_t(m_gopSize);
        int64_t seekPts = timeBaseUnitsForFrames(seekFrameNumber) + m_videoPtsStartOffset;
        int flags = (deltaFromCurrent < 0) ? AVSEEK_FLAG_BACKWARD : 0;

        int64_t pts = AV_NOPTS_VALUE;
        m_endOfFile = false;
        while ((pts == AV_NOPTS_VALUE ||  pts > requestedPts) && !m_endOfFile)
        {
            if (seekFrameNumber <= 0)
            {
                av_seek_frame(m_formatCtx, m_videoStreamIndex, 0, flags);
                avcodec_flush_buffers(m_videoCodecCtx);
                readPacketFromStream(m_videoStreamIndex, "BehavMovieFile::seekFrameAndReadPacket(<=0)");
                break;
            }
            else
            {
                auto seekRet = av_seek_frame(m_formatCtx, m_videoStreamIndex, seekPts, flags);
                if (seekRet < 0)
                {
                    ISX_THROW(
                        isx::ExceptionFileIO,
                        "Failed to read video packet: ",
                        m_fileName,
                        " - ",
                        isxAvErrorCodeToString(seekRet));
                }
                avcodec_flush_buffers(m_videoCodecCtx);
                readPacketFromStream(m_videoStreamIndex, "BehavMovieFile::seekFrameAndReadPacket after seek");

                pts = m_pPacket->pts;
                if (pts > requestedPts)
                {
                    ISX_BEHAV_READ_LOG_DEBUG("seekFrame scanning back: requestedPts: ", requestedPts, ", seekPts: ", seekPts, ", pts:" , pts);

                    av_packet_unref(m_pPacket.get());

                    // if we need to seek again, we will seek backward from current
                    flags |= AVSEEK_FLAG_BACKWARD;
                    --seekFrameNumber;
                    seekPts = timeBaseUnitsForFrames(seekFrameNumber) + m_videoPtsStartOffset;
                }
            }
        }

        ISX_BEHAV_READ_LOG_DEBUG("seekFrame done: requestedPts: ", requestedPts, ", seekPts: ", seekPts);
    }
    else
    {
        if (!m_endOfFile)
        {
            readPacketFromStream(m_videoStreamIndex, "BehavMovieFile::seekFrameAndReadPacket next frame");
        }
    }

    if (!m_endOfFile)
    {
        auto sendPacketRes = avcodec_send_packet(m_videoCodecCtx, m_pPacket.get());
        if (sendPacketRes != 0)
        {
            av_packet_unref(m_pPacket.get());
            ISX_THROW(isx::ExceptionFileIO,
                      "Failed to decode video packet: ", m_fileName);
        }
    }
    av_packet_unref(m_pPacket.get());

    return requestedPts;
}
    
SpVideoFrame_t
BehavMovieFile::getBlackFrame(isize_t inFrameNumber)
{
    Time t = getTimingInfo().convertIndexToStartTime(inFrameNumber);
    auto ret = std::make_shared<VideoFrame>(
        m_spacingInfo, m_spacingInfo.getNumPixels().getWidth(), 1, DataType::U8, t, inFrameNumber);
    std::memset(ret->getPixels(), 0, ret->getImageSizeInBytes());
    return ret;
}

SpVideoFrame_t
BehavMovieFile::readFrame(isize_t inFrameNumber)
{
    ISX_ASSERT(m_videoCodecCtx && m_formatCtx);
    auto requestedPts = seekFrameAndReadPacket(inFrameNumber);
    if (m_endOfFile)
    {
        return getBlackFrame(inFrameNumber);
    }

    auto pFrame = av_frame_alloc();
    auto recvResult = avcodec_receive_frame(m_videoCodecCtx, pFrame);
    auto pts = pFrame->pkt_pts;

    while (!isPtsMatch(requestedPts, pts))
    {
        readPacketFromStream(m_videoStreamIndex, "read for decode");
        if (m_endOfFile)
        {
            break;
        }
        ISX_BEHAV_READ_LOG_DEBUG("req: ", requestedPts, ", packet pts: ", m_pPacket->pts);
        if (avcodec_send_packet(m_videoCodecCtx, m_pPacket.get()) != 0)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Failed to retrieve video packet: ", m_fileName);
        }
        av_packet_unref(m_pPacket.get());
        
        recvResult = 0;
        // Apparently AVPackets may contain multiple frames, so we keep decoding until we either found
        // the pts we want or we get no more frames (recvResult != 0)
        while (!isPtsMatch(requestedPts, pts) && recvResult == 0)
        {
            recvResult = avcodec_receive_frame(m_videoCodecCtx, pFrame);
            if (recvResult != 0 && recvResult != AVERROR(EAGAIN))
            {
                ISX_THROW(isx::ExceptionFileIO,
                          "Failed to decode video: ", m_fileName, ", error: ", recvResult);
            }
            pts = pFrame->pkt_pts;
            if (recvResult == 0)
            {
                ISX_BEHAV_READ_LOG_DEBUG("    pts: ", pts, ", delta: ", requestedPts - pts, ", recvResult: ", recvResult);
            }
            else
            {
                ISX_BEHAV_READ_LOG_DEBUG("recvResult: ", recvResult);
            }
        }
    }
    
    if (pFrame->format == AV_PIX_FMT_NONE)
    {
        // most likely this happens when we reached the end of file and no frame was decoded
        ISX_LOG_ERROR("Invalid behavioral frame format, returning black frame.");
        return getBlackFrame(inFrameNumber);
    }

    switch(pFrame->format)
    {
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_YUYV422:
        case AV_PIX_FMT_YUV422P:
        case AV_PIX_FMT_YUV444P:
        case AV_PIX_FMT_YUV410P:
        case AV_PIX_FMT_YUV411P:
        case AV_PIX_FMT_YUVJ420P:
        case AV_PIX_FMT_YUVJ422P:
        case AV_PIX_FMT_YUVJ444P:
        case AV_PIX_FMT_YUV440P:
        case AV_PIX_FMT_YUVJ440P:
        case AV_PIX_FMT_YUVA420P:
            break;
        default:
        {
            ISX_LOG_ERROR("Behavioral video playback, unsupported frame format: ", pFrame->format, ", ", m_fileName);
            ISX_THROW(isx::ExceptionFileIO, errorMessageUserManual);
        }
    }

    ISX_ASSERT(isize_t(pFrame->width) == m_spacingInfo.getNumPixels().getWidth());
    ISX_ASSERT(isize_t(pFrame->height) == m_spacingInfo.getNumPixels().getHeight());
    
    #if ISX_BEHAV_READ_DEBUG_LOGGING
        double ptsd = (Ratio(pts, 1) * m_timeBase).toDouble();
        int64_t delta = requestedPts - pts;
        const char * pictureTypeNames[] = {
            "0", "I", "P", "B", "S", "SI", "SP", "BI"
        };
    #endif
    
    ISX_BEHAV_READ_LOG_DEBUG(
        "req: (", inFrameNumber, ")", requestedPts,
        "\tactual: ", pts,
        "\(", ptsd, "s)",
        "\tdelta: ", delta, std::abs(delta) > timeBaseUnitsForFrames(1) ? "*" : " ",
        "\t type: ", pictureTypeNames[pFrame->pict_type],
        "\tcpn: ", pFrame->coded_picture_number,
        "\tdpn: ", pFrame->display_picture_number);
    
    m_lastPktPts = pFrame->pkt_pts;
    Time t = getTimingInfo().convertIndexToStartTime(inFrameNumber);
    auto ret = std::make_shared<VideoFrame>(
        m_spacingInfo, pFrame->linesize[0], 1, DataType::U8, t, inFrameNumber);
    
    std::memcpy(ret->getPixels(), pFrame->data[0], pFrame->linesize[0] * pFrame->height);
    av_frame_free(&pFrame);

    m_lastVideoFrameNumber = inFrameNumber;
    return ret;
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
    return m_timingInfos[0];
}
    
const isx::TimingInfos_t &
BehavMovieFile::getTimingInfosForSeries() const
{
    return m_timingInfos;
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

bool
BehavMovieFile::scanAllFrames(int64_t & outFrameCount, int64_t & outGopSize, AsyncCheckInCB_t inCheckInCB)
{
    // find length of file
    int64_t fileLength = -1;
    {
        std::fstream s;
        s.open(m_fileName, std::ios::binary | std::ios_base::in);
        s.seekg(0, std::ios_base::end);
        fileLength = int64_t(s.tellg());
        s.close();
    }

    int readRes = 0;
    int64_t frameCount = 0;
    int64_t gopSize = 0;
    int64_t lastIFrame = -1;
    while (!(readRes = av_read_frame(m_formatCtx, m_pPacket.get())))
    {
        if (m_pPacket->stream_index == m_videoStreamIndex)
        {
            if (m_pPacket->pts == AV_NOPTS_VALUE)
            {
                ISX_LOG_ERROR("Behavioral video import, video packets have no PTS: ", m_fileName);
                ISX_THROW(isx::ExceptionFileIO, errorMessageUserManual);
            }
            if (m_pPacket->flags & AV_PKT_FLAG_KEY)
            {
                if (lastIFrame != -1)
                {
                    gopSize = std::max(gopSize, frameCount - lastIFrame);
                }
                lastIFrame = frameCount;
            }
            ++frameCount;
        }
        av_packet_unref(m_pPacket.get());
        if (inCheckInCB)
        {
            auto cancel = inCheckInCB(float(double(m_formatCtx->pb->pos) / double(fileLength)));
            if (cancel)
            {
                return false;
            }
        }
    }
//    Ratio avDuration = Ratio(m_videoStream->duration, 1);
//    auto durationInSeconds = (avDuration * m_timeBase).toDouble();
//    ISX_LOG_DEBUG("durationInSeconds: ", durationInSeconds, ", last_pts: ", (Ratio(last_pts) * m_timeBase).toDouble());
    outFrameCount = frameCount;
    outGopSize = gopSize;
    
    return true;
}

bool
BehavMovieFile::initializeFromStream(const Time & inStartTime, int64_t inGopSize, int64_t inNumFrames)
{
    //
    // this code is lifted from stream_component_open() in ffplay.c
    //
    
    AVCodecContext *avctx;
    AVCodec *codec;
    AVDictionary *opts = NULL;
    int ret = 0;

    if (m_videoStreamIndex >= int(m_formatCtx->nb_streams))
    {
        return false;
    }

    avctx = avcodec_alloc_context3(NULL);
    if (!avctx)
    {
        return false;
    }

    ret = avcodec_parameters_to_context(avctx, m_formatCtx->streams[m_videoStreamIndex]->codecpar);
    if (ret < 0)
    {
        avcodec_free_context(&avctx);
        ISX_THROW(isx::ExceptionFileIO,
              "Failed to get codec parameters: ", m_fileName);
    }

    av_codec_set_pkt_timebase(avctx, m_formatCtx->streams[m_videoStreamIndex]->time_base);

    codec = avcodec_find_decoder(avctx->codec_id);

    if (!codec)
    {
        avcodec_free_context(&avctx);
        ISX_THROW(isx::ExceptionFileIO,
                  "Failed to find codec: ", m_fileName);
    }

    avctx->codec_id = codec->id;
    
    if(codec->capabilities & AV_CODEC_CAP_DR1)
    {
        avctx->flags |= CODEC_FLAG_EMU_EDGE;
    }
    
    if (avctx->codec_type == AVMEDIA_TYPE_VIDEO || avctx->codec_type == AVMEDIA_TYPE_AUDIO)
        av_dict_set(&opts, "refcounted_frames", "1", 0);
    if ((ret = avcodec_open2(avctx, codec, &opts)) < 0)
    {
        avcodec_free_context(&avctx);
        av_dict_free(&opts);
        ISX_THROW(isx::ExceptionFileIO,
                  "Failed to open codec: ", m_fileName);
    }
    
    m_formatCtx->streams[m_videoStreamIndex]->discard = AVDISCARD_DEFAULT;
    switch (avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        ISX_ASSERT(!"Not supported.");
        break;
    case AVMEDIA_TYPE_VIDEO:
        {
            m_videoStream = m_formatCtx->streams[m_videoStreamIndex];

            auto hasBframes = m_videoStream->codecpar ? m_videoStream->codecpar->video_delay != 0 : false;
            hasBframes = hasBframes || avctx->has_b_frames != 0;
            if (hasBframes)
            {
                ISX_LOG_ERROR("Can't decode video with bipredictive frames: ", m_fileName);
                ISX_THROW(isx::ExceptionFileIO, errorMessageUserManual);
            }

            m_videoCodecCtx = avctx;
            m_spacingInfo = SpacingInfo(isx::SizeInPixels_t(avctx->width, avctx->height));

            m_timeBase = Ratio(m_videoStream->time_base.num, m_videoStream->time_base.den);

            Ratio avDuration = Ratio(m_videoStream->duration, 1);
            auto durationInSeconds = (avDuration * m_timeBase).toDouble();
            Ratio inverseDuration(1000, int64_t(durationInSeconds * 1000.0));
            Ratio numFrames(inNumFrames);
            Ratio frameRate = numFrames * inverseDuration;
            m_timingInfos = TimingInfos_t{TimingInfo(inStartTime, frameRate.getInverse(), inNumFrames)};
            m_gopSize = inGopSize;
            m_videoPtsFrameDelta = getTimingInfo().getStep() / m_timeBase;
            m_videoPtsStartOffset = m_videoStream->start_time;
        }
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        ISX_ASSERT(!"Not supported.");
        break;
    default:
        break;
    }

    av_dict_free(&opts);

    return true;
}
    
int64_t
BehavMovieFile::timeBaseUnitsForFrames(isize_t inFrameNumber) const
{
    return int64_t(std::floor((Ratio(inFrameNumber, 1) * m_videoPtsFrameDelta).toDouble() + 0.5f));
}
    
    
    
    
bool
BehavMovieFile::getBehavMovieProperties(
    const std::string & inFileName,
    DataSet::Properties & outProperties,
    AsyncCheckInCB_t inCheckInCB)
{
    std::unique_ptr<BehavMovieFile> m(new BehavMovieFile(inFileName));
    int64_t gopSize = -1;
    int64_t numFrames = -1;
    if (m->scanAllFrames(numFrames, gopSize, inCheckInCB))
    {
        outProperties[DataSet::PROP_BEHAV_NUM_FRAMES] = Variant(numFrames);
        outProperties[DataSet::PROP_BEHAV_GOP_SIZE] = Variant(gopSize);

        if (!m->initializeFromStream(Time(), gopSize, numFrames))
        {
            ISX_THROW(isx::ExceptionFileIO, "initializeFromStream failed.");
        }

        // try to read one frame to get some errors (eg invalid frame format) right here
        if (m->readFrame(0) == nullptr)
        {
            ISX_THROW(isx::ExceptionFileIO, "readFrame(0) failed.");
        }

        return true;
    }
    return false;
}

// Note aschildan 6/21/2017: This is some debugging code that is currently not used.
// I'll leave it here for future testing. I found it useful when working on the bug
// MOS-920 "Microsoft LifeCam video has exception at end of file"
//
// The problem with that particular file is that the duration is longer than the
// PTS of the last frame indicates (last frame is short of approx 5 frames worth
// of time). I could not fix this with simpler "hacks" eg, force-setting the duration
// to the last frame's PTS instead of the duration reported in the video stream.
//
// Ultimately we'll probably want to store a lookup table frame_index <-> frame_PTS,
// created when importing a movie. It may be necessary to change the frame retrieval
// to use time instead of frame index for this to be useful (that's a much larger change
// I think).
//
// For now we will return black frames at the end of the movie if we reached EOF before
// actually getting to the end of the reported duration of the movie. I have seen
// Premiere Pro showing black frames (a small number) at the end of a movie, though I'm
// not sure if it is for the same reason.
void
BehavMovieFile::scanAllPts()
{
    int64_t frameCount = 0;
    int readRes = 0;
    while (!(readRes = av_read_frame(m_formatCtx, m_pPacket.get())))
    {
        if (m_pPacket->stream_index == m_videoStreamIndex)
        {
            auto clcPts = timeBaseUnitsForFrames(frameCount) + m_videoPtsStartOffset;
            ISX_LOG_INFO("frame: ", frameCount, " \tpktPts: ", m_pPacket->pts, " \t,clcPts: ", clcPts, " \t, delta: ", m_pPacket->pts - clcPts);
            ++frameCount;
        }
        av_packet_unref(m_pPacket.get());
    }
}
    
} // namespace isx





//////////////////////////////////////////////////////////////////////////
// aschildan 10/19/2016 note:
// version with original code lines commented out - use when we
// add dedicated decoder thread or
// add audio support
//////////////////////////////////////////////////////////////////////////
#if 0
bool
BehavMovieFile::initializeFromStream(int inIndex)
{
    //
    // this code is lifted from stream_component_open() in ffplay.c
    //
    
    AVCodecContext *avctx;
    AVCodec *codec;
    //    const char *forced_codec_name = NULL;
    AVDictionary *opts = NULL;
    //    AVDictionaryEntry *t = NULL;
    //    int sample_rate, nb_channels;
    //    int64_t channel_layout;
    int ret = 0;
    //    int stream_lowres = lowres;
    
    if (inIndex >= int(m_formatCtx->nb_streams))
    {
        return false;
    }
    
    avctx = avcodec_alloc_context3(NULL);
    if (!avctx)
    {
        //        return AVERROR(ENOMEM);
        return false;
    }
    
    ret = avcodec_parameters_to_context(avctx, m_formatCtx->streams[inIndex]->codecpar);
    if (ret < 0)
    {
        avcodec_free_context(&avctx);
        ISX_THROW(isx::ExceptionFileIO,
                  "Failed to get codec parameters: ", m_fileName);
    }
    
    av_codec_set_pkt_timebase(avctx, m_formatCtx->streams[inIndex]->time_base);
    
    codec = avcodec_find_decoder(avctx->codec_id);
    
#if 0
    switch(avctx->codec_type){
        case AVMEDIA_TYPE_AUDIO   : is->last_audio_stream    = stream_index; forced_codec_name =    audio_codec_name; break;
        case AVMEDIA_TYPE_SUBTITLE: is->last_subtitle_stream = stream_index; forced_codec_name = subtitle_codec_name; break;
        case AVMEDIA_TYPE_VIDEO   : is->last_video_stream    = stream_index; forced_codec_name =    video_codec_name; break;
    }
    if (forced_codec_name)
        codec = avcodec_find_decoder_by_name(forced_codec_name);
#endif
    
    if (!codec)
    {
        avcodec_free_context(&avctx);
        ISX_THROW(isx::ExceptionFileIO,
                  "Failed to find codec: ", m_fileName);
    }
    
    avctx->codec_id = codec->id;
#if 0
    if(stream_lowres > av_codec_get_max_lowres(codec)){
        av_log(avctx, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
               av_codec_get_max_lowres(codec));
        stream_lowres = av_codec_get_max_lowres(codec);
    }
    av_codec_set_lowres(avctx, stream_lowres);
    
#if FF_API_EMU_EDGE
    if(stream_lowres) avctx->flags |= CODEC_FLAG_EMU_EDGE;
#endif
    if (fast)
        avctx->flags2 |= AV_CODEC_FLAG2_FAST;
#endif
    
#if 1 //FF_API_EMU_EDGE
    if(codec->capabilities & AV_CODEC_CAP_DR1)
        avctx->flags |= CODEC_FLAG_EMU_EDGE;
#endif
    
#if 0
    opts = filter_codec_opts(codec_opts, avctx->codec_id, m_formatCtx, m_formatCtx->streams[inIndex], codec);
    if (!av_dict_get(opts, "threads", NULL, 0))
        av_dict_set(&opts, "threads", "auto", 0);
    if (stream_lowres)
        av_dict_set_int(&opts, "lowres", stream_lowres, 0);
#endif
    if (avctx->codec_type == AVMEDIA_TYPE_VIDEO || avctx->codec_type == AVMEDIA_TYPE_AUDIO)
        av_dict_set(&opts, "refcounted_frames", "1", 0);
    if ((ret = avcodec_open2(avctx, codec, &opts)) < 0)
    {
        avcodec_free_context(&avctx);
        av_dict_free(&opts);
        ISX_THROW(isx::ExceptionFileIO,
                  "Failed to open codec: ", m_fileName);
    }
#if 0
    if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX)))
    {
        av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
        //        ret =  AVERROR_OPTION_NOT_FOUND;
        //        goto fail;
    }
#endif
    m_formatCtx->streams[inIndex]->discard = AVDISCARD_DEFAULT;
    switch (avctx->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            ISX_ASSERT(!"Not supported.");
#if 0
#if CONFIG_AVFILTER
        {
            AVFilterLink *link;
            
            is->audio_filter_src.freq           = avctx->sample_rate;
            is->audio_filter_src.channels       = avctx->channels;
            is->audio_filter_src.channel_layout = get_valid_channel_layout(avctx->channel_layout, avctx->channels);
            is->audio_filter_src.fmt            = avctx->sample_fmt;
            if ((ret = configure_audio_filters(is, afilters, 0)) < 0)
                goto fail;
            link = is->out_audio_filter->inputs[0];
            sample_rate    = link->sample_rate;
            nb_channels    = avfilter_link_get_channels(link);
            channel_layout = link->channel_layout;
        }
#else
            sample_rate    = avctx->sample_rate;
            nb_channels    = avctx->channels;
            channel_layout = avctx->channel_layout;
#endif
            
            /* prepare audio output */
            if ((ret = audio_open(is, channel_layout, nb_channels, sample_rate, &is->audio_tgt)) < 0)
                goto fail;
            is->audio_hw_buf_size = ret;
            is->audio_src = is->audio_tgt;
            is->audio_buf_size  = 0;
            is->audio_buf_index = 0;
            
            /* init averaging filter */
            is->audio_diff_avg_coef  = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
            is->audio_diff_avg_count = 0;
            /* since we do not have a precise anough audio FIFO fullness,
             we correct audio sync only if larger than this threshold */
            is->audio_diff_threshold = (double)(is->audio_hw_buf_size) / is->audio_tgt.bytes_per_sec;
            
            is->audio_stream = stream_index;
            is->audio_st = ic->streams[stream_index];
            
            decoder_init(&is->auddec, avctx, &is->audioq, is->continue_read_thread);
            if ((is->ic->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) && !is->ic->iformat->read_seek) {
                is->auddec.start_pts = is->audio_st->start_time;
                is->auddec.start_pts_tb = is->audio_st->time_base;
            }
            if ((ret = decoder_start(&is->auddec, audio_thread, is)) < 0)
                goto out;
            SDL_PauseAudio(0);
#endif
            break;
        case AVMEDIA_TYPE_VIDEO:
        {
            m_videoStreamIndex = inIndex;
            m_videoStream = m_formatCtx->streams[inIndex];
            m_videoCodecCtx = avctx;
            m_spacingInfo = SpacingInfo(isx::SizeInPixels_t(avctx->width, avctx->height));
            
            m_timeBase = Ratio(m_videoStream->time_base.num, m_videoStream->time_base.den);
            Ratio avDuration = Ratio(m_videoStream->duration, 1);
            Ratio durationInSeconds = (avDuration * m_timeBase);
            
            Ratio frameRate(m_videoStream->avg_frame_rate.num, m_videoStream->avg_frame_rate.den);
            double numFramesD = (durationInSeconds * frameRate).toDouble();
            isize_t numFrames = isize_t(std::floor(numFramesD));
            m_timingInfo = TimingInfo(m_startTime, frameRate.getInverse(), numFrames);
            
            m_videoPtsFrameDelta = m_timingInfo.getStep() * m_timeBase.getInverse();
            m_videoPtsStartOffset = m_videoStream->start_time;
        }
            
#if 0
            decoder_init(&is->viddec, avctx, &is->videoq, is->continue_read_thread);
            if ((ret = decoder_start(&is->viddec, video_thread, is)) < 0)
                goto out;
            is->queue_attachments_req = 1;
#endif
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            ISX_ASSERT(!"Not supported.");
#if 0
            is->subtitle_stream = stream_index;
            is->subtitle_st = ic->streams[stream_index];
            
            decoder_init(&is->subdec, avctx, &is->subtitleq, is->continue_read_thread);
            if ((ret = decoder_start(&is->subdec, subtitle_thread, is)) < 0)
                goto out;
#endif
            break;
        default:
            break;
    }
    
    av_dict_free(&opts);
    
    return true;
}
#endif
