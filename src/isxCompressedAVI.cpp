#include "isxImage.h"

#include "isxCompressedAVI.h"

#include <iostream>
#include <fstream>
#include <stdio.h>

int compressedAVI_encode1(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, std::fstream & outfile)
{
    /* send the frame to the encoder */

    int ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) // Error sending a frame for encoding
    {
        return 1;
    }

    while (true)
    {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        }
        if (ret < 0) // Error during encoding
        {
            break;
        }

        outfile.write((char *) (pkt->data), 1 * pkt->size);
        av_packet_unref(pkt);
    }
    return 2;
}

int compressedAVI_encode2(AVCodecContext *avctx, AVPacket *pkt, int *got_packet, AVFrame *frame)
{
    *got_packet = 0;

    if (avcodec_send_frame(avctx, frame) < 0)
    {
        return 1;
    }

    int ret = avcodec_receive_packet(avctx, pkt);
    switch (ret)
    {
        case 0:
            *got_packet = 1;
            return 0;
        case AVERROR(EAGAIN):
            return 0;
        default:
            return 2;
    }
}

int compressedAVI_preLoop(bool useSimpleEncoder, const std::string & inFileName, std::fstream & fp, AVFrame * & frame, AVPacket * & pkt, AVCodecContext * & avcc, AVCodecID & codec_id, AVCodec * & codec, isx::Image *img, isx::isize_t inFrameRate)
{
    codec_id = AV_CODEC_ID_MPEG1VIDEO;
    codec = avcodec_find_encoder(codec_id);
    if (!codec)
    {
        return 1;
    }

    // Initialize codec. 
    avcc = avcodec_alloc_context3(codec);

    switch (codec_id)
    {
        case AV_CODEC_ID_MPEG1VIDEO:
            avcc->codec_id = codec_id;
            // put sample parameters
            avcc->bit_rate = 4000000;
            // resolution must be a multiple of two
            if (img == NULL)
            {
                return 2;
            }
            else
            {
                avcc->width = int(img->getWidth());
                avcc->height = int(img->getHeight());
            }
            // frames per second

            avcc->time_base = av_make_q(1, int(inFrameRate));

            avcc->framerate = av_make_q(int(inFrameRate), 1);

            // emit one intra frame every ten frames
            // check frame pict_type before passing frame
            // to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
            // then gop_size is ignored and the output of encoder
            // will always be I frame irrespective to gop_size
            //
            avcc->gop_size = 10;
            avcc->max_b_frames = 1;
            avcc->pix_fmt = AV_PIX_FMT_YUV420P;
            break;

        default:
            break;
    }

    int ret = avcodec_open2(avcc, codec, NULL);
    // Open the codec. 
    if (ret < 0)
    {
        return 3;
    }

    pkt = av_packet_alloc();
    if (!pkt)
    {
        return 4;
    }

    fp.open(inFileName.c_str(), std::ios::out | std::ios::binary);
    if (!fp)
    {
        return 5;
    }

    frame = av_frame_alloc();
    if (!frame)
    {
        return 6;
    }
    frame->format = avcc->pix_fmt;
    frame->width = avcc->width;
    frame->height = avcc->height;

    if (av_frame_get_buffer(frame, 32) < 0)
    {
        return 7;
    }
    return 0;
}

int compressedAVI_withinLoop(int tInd, std::fstream & fp, AVPacket *pkt, AVFrame *frame, AVCodecContext *avcc, bool useSimpleEncoder, isx::Image *img, const float minVal, const float maxVal)
{
    const bool rescaleDynamicRange = !(minVal == -1 && maxVal == -1);

    fflush(stdout);
    /* make sure the frame data is writable */
    
    if (av_frame_make_writable(frame) < 0)
    {
        return 1;
    }

    /* Y */
    for (int y = 0; y < avcc->height; y++) {
        for (int x = 0; x < avcc->width; x++) {
            if (rescaleDynamicRange)
            {
                std::vector<float> val = img->getPixelValuesAsF32(isx::isize_t(y), isx::isize_t(x));
                frame->data[0][y * frame->linesize[0] + x] = uint8_t(255 * ((val[0] - minVal) / (maxVal - minVal)));
            }
            else
            {
                std::vector<float> val = img->getPixelValuesAsF32(isx::isize_t(y), isx::isize_t(x));
                frame->data[0][y * frame->linesize[0] + x] = uint8_t(255 * val[0]);
            }
        }
    }

    /* Cb and Cr */
    for (int y = 0; y < avcc->height / 2; y++) {
        for (int x = 0; x < avcc->width / 2; x++) {
            frame->data[1][y * frame->linesize[1] + x] = 128; // no color
            frame->data[2][y * frame->linesize[2] + x] = 128; // no color
        }
    }
    frame->pts = tInd;

    /* encode the image */
    if (useSimpleEncoder)
    {
        int got_packet;
        if (compressedAVI_encode2(avcc, pkt, &got_packet, frame))
        {
            return 2;
        }
        if (got_packet)
        {
            fp.write((char *) (pkt->data), 1 * pkt->size);
        }
    }
    else
    {
        if (compressedAVI_encode1(avcc, frame, pkt, fp))
        {
            return 3;
        }
    }
    
    return 0;
}

int compressedAVI_postLoop(bool useSimpleEncoder, std::fstream & fp, AVFrame * & frame, AVPacket * & pkt, AVCodecContext * & avcc, const std::array<uint8_t, 4> & endcode)
{
    /* flush the encoder */
    if (useSimpleEncoder)
    {
        int got_packet;
        if (compressedAVI_encode2(avcc, pkt, &got_packet, NULL))
        {
            return 1;
        }
        if (got_packet)
        {
            fp.write((char *) (pkt->data), 1 * pkt->size);
        }
    }
    else
    {
        if (compressedAVI_encode1(avcc, NULL, pkt, fp))
        {
            return 2;
        }
    }

    /* add sequence end code to have a real MPEG file */
    fp.write((char *) (endcode.data()), 1 * sizeof(endcode.front()) * endcode.size());
    fp.close();

    avcodec_free_context(&avcc);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    return 0;
}

void compressedAVI_listAllCodecs(std::vector<std::string> & supported, std::vector<std::string> & unSupported)
{

    AVCodec *codec;

    std::cout << std::endl;

    supported.clear();
    codec = NULL;
    while ((codec = av_codec_next(codec)))
    {
        if (avcodec_find_encoder_by_name(codec->name) != 0)
        {
            supported.push_back(codec->name);
        }
    }

    unSupported.clear();
    codec = NULL;
    while ((codec = av_codec_next(codec)))
    {
        if (avcodec_find_encoder_by_name(codec->name) == 0)
        {
            unSupported.push_back(codec->name);
        }
    }
}
