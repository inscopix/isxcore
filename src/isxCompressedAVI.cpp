#include "isxImage.h"

#include "isxCompressedAVI.h"

#include <iostream>
#include <fstream>
#include <stdio.h>

namespace
{

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

}

int compressedAVI_preLoop(const std::string & inFileName, std::fstream & outFs, AVFrame * & outFrame, AVPacket * & outPkt, AVCodecContext * & outAvcc, const isx::Image *inImg, const isx::isize_t inFrameRate, const isx::isize_t inBitRate)
{
    AVCodecID codec_id = AV_CODEC_ID_MPEG1VIDEO;
    AVCodec *outCodec = avcodec_find_encoder(codec_id);
    if (!outCodec)
    {
        return 1;
    }

    // Initialize codec. 
    outAvcc = avcodec_alloc_context3(outCodec);

    outAvcc->codec_id = codec_id;
    // put sample parameters
    outAvcc->bit_rate = inBitRate;
    // resolution must be a multiple of two
    if (inImg == NULL)
    {
        return 2;
    }
    else
    {
        outAvcc->width = int(inImg->getWidth());
        outAvcc->height = int(inImg->getHeight());
    }
    // frames per second

    outAvcc->time_base = av_make_q(1, int(inFrameRate));

    outAvcc->framerate = av_make_q(int(inFrameRate), 1);

    // emit one intra frame every ten frames
    // check frame pict_type before passing frame
    // to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
    // then gop_size is ignored and the output of encoder
    // will always be I frame irrespective to gop_size
    //
    outAvcc->gop_size = 10;
    outAvcc->max_b_frames = 1;
    outAvcc->pix_fmt = AV_PIX_FMT_YUV420P;

    int ret = avcodec_open2(outAvcc, outCodec, NULL);
    // Open the codec. 
    if (ret < 0)
    {
        return 3;
    }

    outPkt = av_packet_alloc();
    if (!outPkt)
    {
        return 4;
    }

    outFs.open(inFileName.c_str(), std::ios::out | std::ios::binary);
    if (!outFs)
    {
        return 5;
    }

    outFrame = av_frame_alloc();
    if (!outFrame)
    {
        return 6;
    }
    outFrame->format = outAvcc->pix_fmt;
    outFrame->width = outAvcc->width;
    outFrame->height = outAvcc->height;

    if (av_frame_get_buffer(outFrame, 32) < 0)
    {
        return 7;
    }
    return 0;
}

int compressedAVI_withinLoop(const int inTInd, std::fstream & inFs, AVPacket *inPkt, AVFrame *inFrame, AVCodecContext *inAvcc, const bool inUseSimpleEncoder, isx::Image *inImg, const float inMinVal, const float inMaxVal)
{
    const bool rescaleDynamicRange = !(inMinVal == -1 && inMaxVal == -1);

    fflush(stdout);
    /* make sure the frame data is writable */
    
    if (av_frame_make_writable(inFrame) < 0)
    {
        return 1;
    }

    /* Y */
    for (int y = 0; y < inAvcc->height; y++) {
        for (int x = 0; x < inAvcc->width; x++) {
            if (rescaleDynamicRange)
            {
                std::vector<float> val = inImg->getPixelValuesAsF32(isx::isize_t(y), isx::isize_t(x));
                inFrame->data[0][y * inFrame->linesize[0] + x] = uint8_t(255 * ((val[0] - inMinVal) / (inMaxVal - inMinVal)));
            }
            else
            {
                std::vector<float> val = inImg->getPixelValuesAsF32(isx::isize_t(y), isx::isize_t(x));
                inFrame->data[0][y * inFrame->linesize[0] + x] = uint8_t(255 * val[0]);
            }
        }
    }

    /* Cb and Cr */
    for (int y = 0; y < inAvcc->height / 2; y++) {
        for (int x = 0; x < inAvcc->width / 2; x++) {
            inFrame->data[1][y * inFrame->linesize[1] + x] = 128; // no color
            inFrame->data[2][y * inFrame->linesize[2] + x] = 128; // no color
        }
    }
    inFrame->pts = inTInd;

    /* encode the image */
    if (inUseSimpleEncoder)
    {
        int got_packet;
        if (::compressedAVI_encode2(inAvcc, inPkt, &got_packet, inFrame))
        {
            return 2;
        }
        if (got_packet)
        {
            inFs.write((char *) (inPkt->data), 1 * inPkt->size);
        }
    }
    else
    {
        if (::compressedAVI_encode1(inAvcc, inFrame, inPkt, inFs))
        {
            return 3;
        }
    }
    
    return 0;
}

int compressedAVI_postLoop(const bool inUseSimpleEncoder, std::fstream & inFs, AVFrame * & inFrame, AVPacket * & inPkt, AVCodecContext * & inAvcc, const std::array<uint8_t, 4> & inEndcode)
{
    /* flush the encoder */
    if (inUseSimpleEncoder)
    {
        int got_packet;
        if (::compressedAVI_encode2(inAvcc, inPkt, &got_packet, NULL))
        {
            return 1;
        }
        if (got_packet)
        {
            inFs.write((char *) (inPkt->data), 1 * inPkt->size);
        }
    }
    else
    {
        if (::compressedAVI_encode1(inAvcc, NULL, inPkt, inFs))
        {
            return 2;
        }
    }

    /* add sequence end code to have a real MPEG file */
    inFs.write((char *) (inEndcode.data()), 1 * sizeof(inEndcode.front()) * inEndcode.size());
    inFs.close();

    avcodec_free_context(&inAvcc);
    av_frame_free(&inFrame);
    av_packet_free(&inPkt);
    return 0;
}

void compressedAVI_listAllCodecs(std::vector<std::string> & outSupported, std::vector<std::string> & outUnSupported)
{

    AVCodec *codec;

    std::cout << std::endl;

    outSupported.clear();
    codec = NULL;
    while ((codec = av_codec_next(codec)))
    {
        if (avcodec_find_encoder_by_name(codec->name) != 0)
        {
            outSupported.push_back(codec->name);
        }
    }

    outUnSupported.clear();
    codec = NULL;
    while ((codec = av_codec_next(codec)))
    {
        if (avcodec_find_encoder_by_name(codec->name) == 0)
        {
            outUnSupported.push_back(codec->name);
        }
    }
}
