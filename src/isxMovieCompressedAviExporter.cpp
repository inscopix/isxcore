#include "isxMovieCompressedAviExporter.h"
#include "isxMovie.h"
#include "isxPathUtils.h"
#include "isxCellSetUtils.h"
#include "isxException.h"

#include <array>
#include <fstream>
#include <algorithm>
#include <cmath>

extern "C"
{
    #include "libavformat/avformat.h"
}

namespace
{

typedef struct VideoOutput
{
    AVStream *avs;
    AVCodecContext *avcc;
    int64_t pts;
    AVFrame *avf;
    AVFrame *avf0;
} VideoOutput;

AVFrame *
allocateAVFrame(enum AVPixelFormat pixelFormat, int width, int height)
{
    AVFrame *avf;
    int ret;
    avf = av_frame_alloc();
    if (!avf)
        return NULL;
    avf->format = pixelFormat;
    avf->width = width;
    avf->height = height;

    ret = av_frame_get_buffer(avf, 32);
    if (ret < 0)
    {
        ISX_THROW(isx::ExceptionFileIO, "Cannot allocate frame.");
    }
    return avf;
}

void
populatePixels(AVFrame *avf, int tIndex, int width, int height, isx::Image *inImg, const float inMinVal, const float inMaxVal)
{
    const bool rescaleDynamicRange = !(inMinVal == -1 && inMaxVal == -1);

    // Y
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            //avf->data[0][y * avf->linesize[0] + x] = 128 + 64 * (2 * (((x + tIndex) / 10) % 2) - 1)*(2 * (((y - tIndex) / 10) % 2) - 1);
            if (rescaleDynamicRange)
            {
                std::vector<float> val = inImg->getPixelValuesAsF32(isx::isize_t(y), isx::isize_t(x));
                avf->data[0][y * avf->linesize[0] + x] = uint8_t(255 * ((val[0] - inMinVal) / (inMaxVal - inMinVal)));
            }
            else
            {
                std::vector<float> val = inImg->getPixelValuesAsF32(isx::isize_t(y), isx::isize_t(x));
                avf->data[0][y * avf->linesize[0] + x] = uint8_t(255 * val[0]);
            }
        }
    }
    // Cb and Cr
    for (int y = 0; y < height / 2; y++)
    {
        for (int x = 0; x < width / 2; x++)
        {
            avf->data[1][y * avf->linesize[1] + x] = 128;
            avf->data[2][y * avf->linesize[2] + x] = 128;
        }
    }
}

int
convertFrameToPacket(AVCodecContext *avcc, AVFrame *avf, AVFormatContext *avFmtCnxt, VideoOutput *vOut)
{
    AVPacket pkt = { 0 };
    int got_packet = 0;

    if (avcodec_send_frame(avcc, avf) < 0)
    {
        ISX_THROW(isx::ExceptionFileIO, "Failed to send frame for encoding.");
    }

    av_init_packet(&pkt);
    if (avcodec_receive_packet(avcc, &pkt) < 0)
    {
        ISX_LOG_INFO("No more encoded packets.");
        return 1;
    }
    got_packet = 1;
    av_packet_rescale_ts(&pkt, avcc->time_base, vOut->avs->time_base);
    pkt.stream_index = vOut->avs->index;
    int ret = av_interleaved_write_frame(avFmtCnxt, &pkt);

    if (ret < 0)
    {
        ISX_THROW(isx::ExceptionFileIO, "Output write error: ", ret);
    }
    return (avf || got_packet) ? 0 : 1;
}

bool
preLoop(const char *filename, AVFormatContext * & avFmtCnxt, VideoOutput & vOut, const isx::Image *inImg, isx::isize_t frameRate, isx::isize_t bitRate)
{
    vOut = { 0 };
    int ret;
    AVDictionary *opt = NULL;
    avformat_alloc_output_context2(&avFmtCnxt, NULL, NULL, filename);
    if (!avFmtCnxt)
    {
        ISX_LOG_INFO("Cannot infer codec from file extension, using default.");
        avformat_alloc_output_context2(&avFmtCnxt, NULL, "mpeg", filename);
    }
    if (!avFmtCnxt)
    {
        return true;
    }
    AVOutputFormat *avOutFmt = avFmtCnxt->oformat;
    if (avOutFmt->video_codec == AV_CODEC_ID_NONE)
    {
        return true;
    }

    ////

    AVCodec *codec = avcodec_find_encoder(avOutFmt->video_codec);
    if (!(codec))
    {
        ISX_THROW(isx::ExceptionFileIO, "No encoder available for ", avcodec_get_name(avOutFmt->video_codec));
    }
    vOut.avs = avformat_new_stream(avFmtCnxt, NULL);
    if (!vOut.avs)
    {
        ISX_THROW(isx::ExceptionFileIO, "Cannot allocate video output");
    }
    vOut.avs->id = avFmtCnxt->nb_streams - 1;
    AVCodecContext *avcc = avcodec_alloc_context3(codec);
    if (avcc == NULL)
    {
        ISX_THROW(isx::ExceptionFileIO, "Cannot allocate context.");
    }

    if (codec->type != AVMEDIA_TYPE_VIDEO)
    {
        ISX_THROW(isx::ExceptionFileIO, "Codec does not support AV media.");
    }

    avcc->codec_id = avOutFmt->video_codec;
    avcc->bit_rate = bitRate;
    avcc->time_base.num = 1;
    avcc->time_base.den = int(frameRate);

    if (inImg)
    {
        avcc->width = int(inImg->getWidth());
        avcc->height = int(inImg->getHeight());
        avcc->width -= (avcc->width % 2);
        avcc->height -= (avcc->height % 2);
    }
    else
    {
        ISX_LOG_ERROR("No image provided.");
        return true;
    }

    avcc->gop_size = 10;
    avcc->pix_fmt = AV_PIX_FMT_YUV420P;
    if (avcc->codec_id == AV_CODEC_ID_MPEG2VIDEO)
    {
        avcc->max_b_frames = 2;
    }
    if (avcc->codec_id == AV_CODEC_ID_MPEG1VIDEO)
    {
        avcc->mb_decision = 2;
    }

    if (avFmtCnxt->oformat->flags & AVFMT_GLOBALHEADER)
    {
        avcc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    vOut.avcc = avcc;
    vOut.avs->time_base = vOut.avcc->time_base;

    ////

    AVDictionary *opt2 = NULL;
    av_dict_copy(&opt2, opt, 0);
    ret = avcodec_open2(vOut.avcc, codec, &opt2);
    av_dict_free(&opt2);
    if (ret < 0)
    {
        ISX_THROW(isx::ExceptionFileIO, "Codec cannot be opened: ", ret);
    }

    vOut.avf = allocateAVFrame(vOut.avcc->pix_fmt, vOut.avcc->width, vOut.avcc->height);
    if (vOut.avf == NULL)
    {
        ISX_THROW(isx::ExceptionFileIO, "Frame cannot be allocated.");
    }

    vOut.avf0 = NULL;
    if (vOut.avcc->pix_fmt != AV_PIX_FMT_YUV420P)
    {
        vOut.avf0 = allocateAVFrame(AV_PIX_FMT_YUV420P, vOut.avcc->width, vOut.avcc->height);
        if (vOut.avf0 == NULL)
        {
            ISX_THROW(isx::ExceptionFileIO, "Cannot allocate buffer.");
        }
    }

    ret = avcodec_parameters_from_context(vOut.avs->codecpar, vOut.avcc);
    if (ret < 0)
    {
        ISX_THROW(isx::ExceptionFileIO, "Parameters cannot be copied.");
    }

    ////

    av_dump_format(avFmtCnxt, 0, filename, 1);
    if (!(avOutFmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&avFmtCnxt->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            ISX_LOG_ERROR("Cannot open file ", filename, ": ", ret);
            return true;
        }
    }
    ret = avformat_write_header(avFmtCnxt, &opt);
    if (ret < 0)
    {
        ISX_LOG_ERROR("Output error: ", ret);
        return true;
    }
    return false;
}

bool
withinLoop(AVFormatContext *avFmtCnxt, VideoOutput *vOut, isx::Image *inImg, const float inMinVal, const float inMaxVal)
{
    AVFrame *avf = NULL;
    AVCodecContext *avcc = vOut->avcc;

    if (av_frame_make_writable(vOut->avf) < 0)
    {
        ISX_THROW(isx::ExceptionFileIO, "Frame not writable.");
    }
    if (vOut->avcc->pix_fmt != AV_PIX_FMT_YUV420P)
    {
        ISX_THROW(isx::ExceptionFileIO, "Unexpected pixel format.");
    }
    populatePixels(vOut->avf, int(vOut->pts), vOut->avcc->width, vOut->avcc->height, inImg, inMinVal, inMaxVal);
    vOut->avf->pts = vOut->pts++;
    avf = vOut->avf;

    convertFrameToPacket(avcc, avf, avFmtCnxt, vOut);
    return false;
}

bool
postLoop(AVFormatContext *avFmtCnxt, VideoOutput & vOut)
{
    int done = 0;
    while (!done)
    {
        done = convertFrameToPacket(vOut.avcc, NULL, avFmtCnxt, &vOut);
    }

    AVOutputFormat *avOutFmt = avFmtCnxt->oformat;

    av_write_trailer(avFmtCnxt);
    avcodec_free_context(&(vOut.avcc));
    av_frame_free(&(vOut.avf));
    av_frame_free(&(vOut.avf0));

    if (!(avOutFmt->flags & AVFMT_NOFILE))
    {
        avio_closep(&(avFmtCnxt->pb));
    }
    avformat_free_context(avFmtCnxt);
    return false;
}

bool
compressedAVIFindMinMax(const std::string & inFileName, const std::vector<isx::SpMovie_t> & inMovies, isx::AsyncCheckInCB_t & inCheckInCB, float & outMinVal, float & outMaxVal, float progressBarStart, float progressBarEnd)
{
    outMinVal = std::numeric_limits<float>::max();
    outMaxVal = -std::numeric_limits<float>::max();

    bool cancelled = false;
    isx::isize_t writtenFrames = 0;
    isx::isize_t numFrames = 0;
    for (auto m : inMovies)
    {
        numFrames += m->getTimingInfo().getNumTimes();
    }

    for (auto m : inMovies)
    {
        for (isx::isize_t i = 0; i < m->getTimingInfo().getNumTimes(); ++i)
        {
            if (m->getTimingInfo().isIndexValid(i))
            {
                auto f = m->getFrame(i);
                auto& img = f->getImage();

                float minValLocal, maxValLocal;
                getImageMinMax(img, minValLocal, maxValLocal);

                outMinVal = std::min(outMinVal, minValLocal);
                outMaxVal = std::max(outMaxVal, maxValLocal);
            }

            cancelled = inCheckInCB(progressBarStart + (progressBarEnd - progressBarStart)*float(++writtenFrames) / float(numFrames));
            if (cancelled)
            {
                break;
            }
        }
        if (cancelled)
        {
            break;
        }
    }
    return cancelled;
}

bool
compressedAVIOutputMovie(const std::string & inFileName, const std::vector<isx::SpMovie_t> & inMovies, isx::AsyncCheckInCB_t & inCheckInCB, float & inMinVal, float & inMaxVal, float progressBarStart, float progressBarEnd)
{
    int tInd = 0;
    bool cancelled = false;
    isx::isize_t writtenFrames = 0;
    isx::isize_t numFrames = 0;
    isx::DurationInSeconds step, stepPrevious, stepFirst;
    double eps = 1e-2;
    isx::isize_t count = 0;
    for (auto m : inMovies)
    {
        numFrames += m->getTimingInfo().getNumTimes();
        step = m->getTimingInfo().getStep();
        if (count == 0)
        {
            stepFirst = step;
        }
        if (fabs(step.toDouble() - stepFirst.toDouble()) > eps)
        {
            ISX_LOG_WARNING("Steps don't match: ", step.toDouble(), " and ", stepFirst.toDouble());
        }
        count++;
    }
    ISX_ASSERT(stepFirst != isx::DurationInSeconds());

    isx::isize_t frameRate = std::lround(stepFirst.getInverse().toDouble());
    isx::isize_t bitRate = 400000; // TODO: possibly connect to front end for user control

    VideoOutput vOut;
    AVFormatContext *avFmtCnxt;

    for (auto m : inMovies)
    {
        for (isx::isize_t i = 0; i < m->getTimingInfo().getNumTimes(); ++i)
        {
            if (m->getTimingInfo().isIndexValid(i))
            {
                auto f = m->getFrame(i);
                auto& img = f->getImage();
                
                if (tInd == 0)
                {
                    if (preLoop(inFileName.c_str(), avFmtCnxt, vOut, &img, frameRate, bitRate))
                    {
                        return true;
                    }
                }
                if (withinLoop(avFmtCnxt, &vOut, &img, inMinVal, inMaxVal))
                {
                    return true;
                }                
                tInd++;
            }

            cancelled = inCheckInCB(progressBarStart + (progressBarEnd - progressBarStart)*float(++writtenFrames) / float(numFrames));
            if (cancelled)
            {
                break;
            }
        }
        if (cancelled)
        {
            break;
        }
    }

    if (postLoop(avFmtCnxt, vOut))
    {
        return true;
    }
    return cancelled;
}

bool
toCompressedAVI(const std::string & inFileName, const std::vector<isx::SpMovie_t> & inMovies, isx::AsyncCheckInCB_t & inCheckInCB)
{
    bool cancelled;
    float minVal = -1;
    float maxVal = -1;

    cancelled = compressedAVIFindMinMax(inFileName, inMovies, inCheckInCB, minVal, maxVal, 0.0f, 0.1f);
    if (cancelled) return true;

    cancelled = compressedAVIOutputMovie(inFileName, inMovies, inCheckInCB, minVal, maxVal, 0.1f, 1.0f);
    if (cancelled) return true;
    
    return false;
}

} // namespace

namespace isx {

std::string
MovieCompressedAviExporterParams::getOpName()
{
    return "Export MPEG1 Movie";
}

AsyncTaskStatus 
runMovieCompressedAviExporter(MovieCompressedAviExporterParams inParams, std::shared_ptr<MovieCompressedAviExporterOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB)
{
    if (inParams.m_compressedAviFilename.empty())
    {
        ISX_THROW(isx::ExceptionUserInput, "No output video file specified.");
    }

    bool cancelled = false;
    auto & srcs = inParams.m_srcs;

    // validate inputs
    if (srcs.empty())
    {
        inCheckInCB(1.f);
        return AsyncTaskStatus::COMPLETE;
    }

    for (auto & cs: srcs)
    {
        if (cs == nullptr)
        {
            ISX_THROW(isx::ExceptionUserInput, "One or more of the sources is invalid.");
        }
    }

    try
    {
        cancelled = toCompressedAVI(inParams.m_compressedAviFilename, inParams.m_srcs, inCheckInCB);
    }
    catch (...)
    {
        std::remove(inParams.m_compressedAviFilename.c_str());
        throw;
    }

    if (cancelled)
    {
        if (!inParams.m_compressedAviFilename.empty())
        {
            std::remove(inParams.m_compressedAviFilename.c_str());
        }

        return AsyncTaskStatus::CANCELLED;
    }

    inCheckInCB(1.f);
    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
