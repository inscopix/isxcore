
#include "isxBehavMovieFile.h"
#include "isxPathUtils.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}


#include <cstring>
#include <cmath>
#include <algorithm>
#include <limits>

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

namespace isx
{

BehavMovieFile::BehavMovieFile()
{}

BehavMovieFile::BehavMovieFile(const std::string & inFileName, const Time & inStartTime)
{
    m_fileName = inFileName;
    m_startTime = inStartTime;

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
    
    if (!initializeFromStream(firstVideoStreamIndex))
    {
        return;
    }

    m_pPacket.reset(new AVPacket);
    
    m_dataType = DataType::U8; // TODO aschildan 10/19/2016: adjust this when we support color movies

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

SpVideoFrame_t
BehavMovieFile::readFrame(isize_t inFrameNumber)
{
    static const isize_t sGopSize = 10; // TODO aschildan 10/12/2016: need to calculate this, this value is extracted from noldus video
    
    ISX_ASSERT(m_videoCodecCtx && m_formatCtx);

    int64_t requestedPts = timeBaseUnitsForFrames(inFrameNumber) + m_videoPtsStartOffset;
    int64_t deltaFromCurrent = requestedPts - m_lastPktPts;
    int64_t deltaFromExpected = deltaFromCurrent - timeBaseUnitsForFrames(1);
    ISX_BEHAV_READ_LOG_DEBUG("deltaFromExpected: ", deltaFromExpected, ", thresh: ", int64_t(m_videoPtsFrameDelta.toDouble() + 0.5f));
    
    bool seeking = false;
    
    if (inFrameNumber == 0)
    {
        av_seek_frame(m_formatCtx, m_videoStreamIndex, 0, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(m_videoCodecCtx);
        seeking = true;
    }
    else if (-deltaFromExpected > timeBaseUnitsForFrames(1)
           || deltaFromExpected > timeBaseUnitsForFrames(sGopSize))
    {
        // We get here and start seeking if the next frame is more than one frame
        // before the expected frame or more than the gop-size after the expected frame
        
        if (inFrameNumber == m_lastVideoFrameNumber + 1)
        {
            ISX_BEHAV_READ_LOG_DEBUG("Last frame had timestamp for this frame, repeating it.");
        }
        
        
        isize_t seekFrameNumber = inFrameNumber - sGopSize;
        int64_t seekPts = timeBaseUnitsForFrames(seekFrameNumber) + m_videoPtsStartOffset;
        
        if (inFrameNumber < sGopSize)
        {
            seekFrameNumber = 0;
            seekPts = 0;
        }
        
        int flags = AVSEEK_FLAG_ANY | ((deltaFromCurrent < 0) ? AVSEEK_FLAG_BACKWARD : 0);
        ISX_BEHAV_READ_LOG_DEBUG("req: ", requestedPts, ", seek: ", seekPts);
        av_seek_frame(m_formatCtx, m_videoStreamIndex, seekPts, flags);
        avcodec_flush_buffers(m_videoCodecCtx);
        seeking = true;
    }

    AVFrame * pFrame = av_frame_alloc();
    int recvResult = 0;
    int64_t pts = AV_NOPTS_VALUE;

    if (!seeking)
    {
        recvResult = avcodec_receive_frame(m_videoCodecCtx, pFrame);
        pts = pFrame->pkt_pts;
    }

    while (!isPtsMatch(requestedPts, pts)
           && av_read_frame(m_formatCtx, m_pPacket.get()) >= 0)
    {
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

    ISX_ASSERT(pFrame->format == AVPixelFormat::AV_PIX_FMT_YUV420P);

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
BehavMovieFile::initializeFromStream(int inIndex)
{
    //
    // this code is lifted from stream_component_open() in ffplay.c
    //
    
    AVCodecContext *avctx;
    AVCodec *codec;
    AVDictionary *opts = NULL;
    int ret = 0;

    if (inIndex >= int(m_formatCtx->nb_streams))
    {
        return false;
    }

    avctx = avcodec_alloc_context3(NULL);
    if (!avctx)
    {
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
    
    m_formatCtx->streams[inIndex]->discard = AVDISCARD_DEFAULT;
    switch (avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        ISX_ASSERT(!"Not supported.");
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
            
            m_timingInfos = TimingInfos_t{TimingInfo(m_startTime, frameRate.getInverse(), numFrames)};
            
            m_videoPtsFrameDelta = getTimingInfo().getStep() * m_timeBase.getInverse();
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
