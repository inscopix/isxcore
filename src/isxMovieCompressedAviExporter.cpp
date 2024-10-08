// Reference for understanding this code: https://www.ffmpeg.org/doxygen/trunk/muxing_8c-example.html

#include "isxMovieCompressedAviExporter.h"
#include "isxMovie.h"
#include "isxPathUtils.h"
#include "isxCellSetUtils.h"
#include "isxException.h"
#include "isxNVisionTracking.h"

#include <array>
#include <fstream>
#include <algorithm>
#include <cmath>

#include "json.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"

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

template<typename T>
void
normalizeAndPopulatePixels(AVFrame *avf, int tIndex, int width, int height, int imageLineSize, const T * pixels, const float inMinVal, const float inMaxVal, const bool isValid)
{
    if (inMinVal == -1 && inMaxVal == -1)
    {
        ISX_THROW(isx::ExceptionDataIO, "Cannot rescale pixels to range 0-255. No min and max found.");
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            uint8_t pixelValue = 0;
            if (isValid)
            {
                pixelValue = uint8_t(255 * ((pixels[y * imageLineSize + x] - inMinVal) / (inMaxVal - inMinVal)));
            }
            avf->data[0][y * avf->linesize[0] + x] = pixelValue;
        }
    }
}

void
populatePixels(AVFrame *avf, int tIndex, int width, int height, isx::Image *inImg, const float inMinVal, const float inMaxVal, const bool isValid)
{
    // Y
    const isx::DataType dataType = inImg->getDataType();
    const int imageLineSize = static_cast<int>(inImg->getRowBytes() / inImg->getPixelSizeInBytes());
    switch(dataType)
    {
        case isx::DataType::U8:
        {
            // populate pixels directly to buffer
            const uint8_t * pixels = inImg->getPixelsAsU8();
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    avf->data[0][y * avf->linesize[0] + x] = pixels[y * imageLineSize + x];
                }
            }
            break;
        }
        case isx::DataType::F32:
        {
            const float * pixels = inImg->getPixelsAsF32();
            normalizeAndPopulatePixels(avf, tIndex, width, height, imageLineSize, pixels, inMinVal, inMaxVal, isValid);
            break;
        }
        case isx::DataType::U16:
        {
            const uint16_t * pixels = inImg->getPixelsAsU16();
            normalizeAndPopulatePixels(avf, tIndex, width, height, imageLineSize, pixels, inMinVal, inMaxVal, isValid);
            break;
        }
        default:
        {
            ISX_THROW(isx::Exception, "Unsupported data type.");
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
        ISX_LOG_DEBUG("No more encoded packets.");
        return 1;
    }
    got_packet = 1;
    pkt.duration = 1;
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
preLoop(const char *filename, AVFormatContext * & avFmtCnxt, VideoOutput & vOut, const isx::Image *inImg, isx::DurationInSeconds framePeriod, isx::isize_t bitRate, const bool roundFrameRate)
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
    
    int framePeriodNum, framePeriodDen;
    if (roundFrameRate)
    {
        // Round the frame rate to the closest integer.
        // Multiply the frame rate by 1000 to set the timescale of the encoder to 1000..
        // This ensures that decoders interpret the frame rate correctly -
        // using a timescale of 1 leads to decoders inferring a slightly different fps than what's encoded in the file.
        // See function convertFrameToPacket for more info
        framePeriodNum = 1;
        framePeriodDen = int(std::round(framePeriod.getInverse().toDouble()));
    }
    else
    {
        // The MPEG4 codec only supports frame period denominators of up to 2^16 - 1.
        // Use libav util function to reduce the frampPeriod ratio so that both components are less than 2^16 - 1
        // while still maintaining high enough precision with the average sampling rate stored in the exported file.
        av_reduce(&framePeriodNum, &framePeriodDen, int64_t(framePeriod.getNum()), int64_t(framePeriod.getDen()), 65535);
    }
    avcc->time_base.num = framePeriodNum;
    avcc->time_base.den = framePeriodDen;
    avcc->framerate.num = framePeriodDen;
    avcc->framerate.den = framePeriodNum;

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
withinLoop(AVFormatContext *avFmtCnxt, VideoOutput *vOut, isx::Image *inImg, const float inMinVal, const float inMaxVal, const bool isValid)
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
    populatePixels(vOut->avf, int(vOut->pts), vOut->avcc->width, vOut->avcc->height, inImg, inMinVal, inMaxVal, isValid);
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
compressedAVIFindMinMax(const std::string & inFileName, const std::vector<isx::SpMovie_t> & inMovies, isx::AsyncCheckInCB_t & inCheckInCB, float & outMinVal, float & outMaxVal, const float inProgressAllocation, const float inProgressStart)
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

            cancelled = inCheckInCB(inProgressStart + (float(++writtenFrames) / float(numFrames)) * inProgressAllocation);
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

// helper function to draw nVision tracking data on movie frames.
void drawTrackingData(
    isx::Image & inImage,
    const isx::SpMovie_t & inMovie,
    const size_t inFrameIndex,
    const isx::NVisionMovieTrackingExporterParams inParams,
    const std::vector<isx::Zone> inZones
)
{

    const auto spacingInfo = inImage.getSpacingInfo();
    const auto numPixels = spacingInfo.getNumPixels();
    const auto size = cv::Size(
        int(numPixels.getWidth()),
        int(numPixels.getHeight())
    );
    const auto dataType = CV_8U; // constant for nVision movies
    auto mat = cv::Mat(size, dataType, inImage.getPixels());

    const auto boundingBox = isx::BoundingBox::fromMetadata(
        inMovie->getFrameMetadata(inFrameIndex)
    );
    const auto rect = boundingBox.isValid() ?
        cv::Rect(
            cv::Point(
                int(std::round(boundingBox.getLeft())),
                int(std::round(boundingBox.getTop()))
            ),
            cv::Point(
                int(std::round(boundingBox.getRight())),
                int(std::round(boundingBox.getBottom()))
            )
        ) :
        cv::Rect();

    // TODO: make line color visible.
    const auto color = cv::Scalar(255, 255, 255);
    const int lineThickness = 2;
    
    if (inParams.m_drawBoundingBox && rect.area())
    {
        cv::rectangle(
            mat,
            rect,
            color,
            lineThickness
        );

    }

    if (inParams.m_drawBoundingBoxCenter && rect.area())
    {
        cv::Point center(
            int(boundingBox.getCenter().getX()),
            int(boundingBox.getCenter().getY())
        );
        cv::circle(
            mat,
            center,
            3,
            color,
            -1
        );
    }

    if (inParams.m_drawZones)
    {
        const bool isClosed = true;
        for (const auto & zone : inZones)
        {
            std::vector<cv::Point> coordinates;
            for (const auto & coordinate : zone.getCoordinates())
            {
                coordinates.push_back(cv::Point(
                    int(std::round(coordinate.getX())),
                    int(std::round(coordinate.getY()))
                ));
            }

            if (zone.getType() == isx::Zone::Type::ELLIPSE)
            {
                const double startAngle = 0.0;
                const double endAngle = 360.0;
                cv::ellipse(
                    mat,
                    coordinates.front(),
                    cv::Size(
                        int(std::round(zone.getMajorAxis() / 2.0f)),
                        int(std::round(zone.getMinorAxis() / 2.0f))
                    ),
                    zone.getAngle(),
                    startAngle,
                    endAngle,
                    color,
                    lineThickness
                );
            }
            else
            {
                cv::polylines(
                    mat,
                    coordinates,
                    isClosed,
                    color,
                    lineThickness
                );
            }

        }
    }
}

bool
compressedAVIOutputMovie(
    const std::string & inFileName,
    const std::vector<isx::SpMovie_t> & inMovies,
    isx::AsyncCheckInCB_t & inCheckInCB,
    float & inMinVal,
    float & inMaxVal,
    const float inProgressAllocation,
    const float inProgressStart,
    const isx::isize_t inBitRate,
    const bool inWriteInvalidFrames,
    const bool inRoundFrameRate,
    const std::shared_ptr<isx::NVisionMovieTrackingExporterParams> inTrackingParams
)
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

    VideoOutput vOut;
    AVFormatContext *avFmtCnxt;

    for (auto m : inMovies)
    {
        std::vector<isx::Zone> zones;
        if (inTrackingParams)
        {
            zones = isx::getZonesFromMetadata(m->getExtraProperties());
        }

        for (isx::isize_t i = 0; i < m->getTimingInfo().getNumTimes(); ++i)
        {
            const bool isValid = m->getTimingInfo().isIndexValid(i);
            if (inWriteInvalidFrames || isValid)
            {
                auto f = m->getFrame(i);
                auto& img = f->getImage();

                if (inTrackingParams)
                {
                    drawTrackingData(
                        img,
                        m,
                        i,
                        *inTrackingParams,
                        zones
                    );
                }
                
                if (tInd == 0)
                {
                    if (preLoop(inFileName.c_str(), avFmtCnxt, vOut, &img, stepFirst, inBitRate, inRoundFrameRate))
                    {
                        return true;
                    }
                }
                if (withinLoop(avFmtCnxt, &vOut, &img, inMinVal, inMaxVal, isValid))
                {
                    return true;
                }                
                tInd++;
            }

            cancelled = inCheckInCB(inProgressStart + (float(++writtenFrames) / float(numFrames)) * inProgressAllocation);
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
toCompressedAVI(
    const std::string & inFileName,
    const std::vector<isx::SpMovie_t> & inMovies,
    isx::AsyncCheckInCB_t & inCheckInCB,
    const isx::isize_t inBitRate,
    const bool inWriteInvalidFrames,
    const bool inRoundFrameRate,
    const float inProgressAllocation,
    const float inProgressStart,
    const std::shared_ptr<isx::NVisionMovieTrackingExporterParams> inTrackingParams
)
{
    bool cancelled;
    float minVal = -1;
    float maxVal = -1;
    float progressStart = inProgressStart;
    float progressAllocation = inProgressAllocation;

    if (inMovies.front()->getDataType() != isx::DataType::U8)
    {
        // allocate 10% of this operation to finding the min/max
        progressAllocation = inProgressAllocation * 0.1f;
        cancelled = compressedAVIFindMinMax(inFileName, inMovies, inCheckInCB, minVal, maxVal, progressAllocation, progressStart);
        if (cancelled) return true;

        progressStart += progressAllocation;
        progressAllocation = inProgressAllocation * 0.9f;
    }

    cancelled = compressedAVIOutputMovie(
        inFileName,
        inMovies,
        inCheckInCB,
        minVal,
        maxVal,
        progressAllocation,
        progressStart,
        inBitRate,
        inWriteInvalidFrames,
        inRoundFrameRate,
        inTrackingParams
    );

    if (cancelled) return true;
    
    return false;
}

} // namespace

namespace isx {

std::string
MovieCompressedAviExporterParams::getOpName()
{
    return "Export MP4 Movie";
}

std::string
MovieCompressedAviExporterParams::toString() const
{
    using json = nlohmann::json;
    json j;
    return j.dump(4);
}

MovieExporterParams::Type
MovieCompressedAviExporterParams::getType()
{
    return MovieExporterParams::Type::MP4;
}

void
MovieCompressedAviExporterParams::setOutputFileName(const std::string & inFileName)
{
    m_filename = inFileName;
}

void
MovieCompressedAviExporterParams::setSources(const std::vector<SpMovie_t> & inSources)
{
    m_srcs = inSources;
}

void
MovieCompressedAviExporterParams::setBitRate(const isize_t inBitRate)
{
    m_bitRate = inBitRate;
    m_bitRateFraction = 0;
}

void
MovieCompressedAviExporterParams::setBitRateFraction(const double inBitRateFraction)
{
    m_bitRateFraction = inBitRateFraction;
}

void
MovieCompressedAviExporterParams::setFrameRateFormat(const FrameRateFormat inFrameRateFormat)
{
    m_frameRateFormat = inFrameRateFormat;
}

void
MovieCompressedAviExporterParams::updateBitRateBasedOnFraction()
{
    isize_t bitPerByte = 8;
    SpMovie_t & firstSrc = m_srcs[0];
    SpacingInfo si = firstSrc->getSpacingInfo();
    isize_t bytesPerPixel = getDataTypeSizeInBytes(firstSrc->getDataType());
    double frameRate = 1 / firstSrc->getTimingInfo().getStep().toDouble();
    // explicit enforcement of left-to-right precedence rule
    // purpose: sub-product will be a double, thus avoiding any possibility of "wrapping-around" of unsigned int's
    double bitRateNoCompression = ((frameRate * si.getTotalNumPixels()) * bytesPerPixel) * bitPerByte;
    m_bitRate = isize_t(m_bitRateFraction * bitRateNoCompression);
}

void
MovieCompressedAviExporterParams::setAdditionalInfo(
        const std::string & inIdentifierBase,
        const std::string & inSessionDescription,
        const std::string & inComments,
        const std::string & inDescription,
        const std::string & inExperimentDescription,
        const std::string & inExperimenter,
        const std::string & inInstitution,
        const std::string & inLab,
        const std::string & inSessionId)
{
    // Do nothing - MP4 cannot contains these details
}

void
MovieCompressedAviExporterParams::setWriteDroppedAndCroppedParameter(const bool inWriteDroppedAndCropped)
{
    m_writeInvalidFrames = inWriteDroppedAndCropped;
}

std::vector<std::string>
MovieCompressedAviExporterParams::getInputFilePaths() const
{
    std::vector<std::string> inputFilePaths;
    for (const auto & s : m_srcs)
    {
        inputFilePaths.push_back(s->getFileName());
    }
    return inputFilePaths;
}

std::vector<std::string>
MovieCompressedAviExporterParams::getOutputFilePaths() const
{
    return {m_filename};
}

isx::isize_t
MovieCompressedAviExporterParams::getBitRate() const
{
    return m_bitRate;
}

double
MovieCompressedAviExporterParams::getBitRateFraction() const
{
    return m_bitRateFraction;
}

AsyncTaskStatus 
runMovieCompressedAviExporter(MovieCompressedAviExporterParams inParams, std::shared_ptr<MovieCompressedAviExporterOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB, const float inProgressAllocation, const float inProgressStart)
{
    if (inParams.m_filename.empty())
    {
        ISX_THROW(isx::ExceptionUserInput, "No output video file specified.");
    }

    bool cancelled = false;
    auto & srcs = inParams.m_srcs;

    // validate inputs
    if (srcs.empty())
    {
        inCheckInCB(inProgressStart + inProgressAllocation);
        return AsyncTaskStatus::COMPLETE;
    }

    for (auto & cs: srcs)
    {
        if (cs == nullptr)
        {
            ISX_THROW(isx::ExceptionUserInput, "One or more of the sources is invalid.");
        }
    }

    if (inParams.getBitRate() == 0)
    {
        inParams.updateBitRateBasedOnFraction();
    }

    const bool inRoundFrameRate = inParams.m_frameRateFormat == MovieExporterParams::FrameRateFormat::INTEGER_ROUNDED;
    try
    {
        cancelled = toCompressedAVI(
            inParams.m_filename,
            inParams.m_srcs,
            inCheckInCB,
            inParams.getBitRate(),
            inParams.m_writeInvalidFrames,
            inRoundFrameRate,
            inProgressAllocation,
            inProgressStart,
            inParams.m_trackingParams
        );
    }
    catch (...)
    {
        std::remove(inParams.m_filename.c_str());
        throw;
    }

    if (cancelled)
    {
        if (!inParams.m_filename.empty())
        {
            std::remove(inParams.m_filename.c_str());
        }

        return AsyncTaskStatus::CANCELLED;
    }

    inCheckInCB(inProgressStart + inProgressAllocation);
    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
