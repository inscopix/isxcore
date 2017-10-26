#include "isxImage.h"
extern "C" {
#include "libavformat/avformat.h"
}

#include <iostream>

void compressedAVI_encode1(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *outfile)
{
    static int count = 0;
    count++;
    int ret;

    /* send the frame to the encoder */
    //if (frame) printf("Send frame %3"PRId64"\n", frame->pts);

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            std::cout << "looky: " << count << " " << (ret == AVERROR(EAGAIN)) << " " << (ret == AVERROR_EOF) << std::endl;
            return;
            //exit(1);
        }
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

        //printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
}

int compressedAVI_encode2(AVCodecContext *avctx, AVPacket *pkt, int *got_packet, AVFrame *frame)
{
    int ret = 0;

    *got_packet = 0;

    ret = avcodec_send_frame(avctx, frame);
    if (ret < 0)
        return ret;

    ret = avcodec_receive_packet(avctx, pkt);
    if (!ret)
        *got_packet = 1;
    if (ret == AVERROR(EAGAIN))
        return 0;

    return ret;
}

/*void fillFramePixelsOrig(int i, AVFrame *frame, AVCodecContext* c)
{
    // prepare a dummy image
    // Y
    for (int y = 0; y < c->height; y++) {
        for (int x = 0; x < c->width; x++) {
            frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
        }
    }

    // Cb and Cr
    for (int y = 0; y < c->height / 2; y++) {
        for (int x = 0; x < c->width / 2; x++) {
            frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
            frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
        }
    }
}*/

int compressedAVI_preLoop(bool & useSimpleEncoder, char * & filename, FILE * & fp, AVFrame * & frame, AVPacket * & pkt, AVCodecContext * & avcc, AVCodecID & codec_id, AVCodec * & codec, isx::Image *img)
{
    useSimpleEncoder = true;
    filename = "sample.avi";

    codec_id = AV_CODEC_ID_MPEG1VIDEO; //;AV_CODEC_ID_MJPEG
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
        avcc->bit_rate = 400000;
        // resolution must be a multiple of two
        if (img == NULL)
        {
            avcc->width = 352;
            avcc->height = 288;
        }
        else
        {
            avcc->width = int(img->getWidth());
            avcc->height = int(img->getHeight());
        }
        // frames per second

        //avcc->time_base = (AVRational) { 1, 25 };
        avcc->time_base = av_make_q(1, 25);

        //avcc->framerate = (AVRational) { 25, 1 };
        avcc->framerate = av_make_q(25, 1);

        // emit one intra frame every ten frames
        // check frame pict_type before passing frame
        // to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
        // then gop_size is ignored and the output of encoder
        // will always be I frame irrespective to gop_size
        //
        avcc->gop_size = 10;
        avcc->max_b_frames = 1;
        avcc->pix_fmt = AV_PIX_FMT_YUV420P; // AV_PIX_FMT_YUV420P;AV_PIX_FMT_RGB8
        break;
    case AV_CODEC_ID_MJPEG:
        avcc->codec_id = codec_id;
        avcc->codec_type = AVMEDIA_TYPE_VIDEO;
        avcc->pix_fmt = AV_PIX_FMT_YUVJ422P;
        avcc->bit_rate = 400000;
        avcc->width = 320;
        avcc->height = 240;
        avcc->time_base.num = 1;
        avcc->time_base.den = 1;
    }

    int ret = avcodec_open2(avcc, codec, NULL);
    // Open the codec. 
    if (ret < 0)
    {
        return 2;
    }

    pkt = av_packet_alloc();
    if (!pkt)
    {
        return 3;
    }

    //fp = fopen(filename, "wb");
    fopen_s(&fp, filename, "wb");
    if (!fp) {
        return 5;
    }

    frame = av_frame_alloc();
    if (!frame) {
        return 6;
    }
    frame->format = avcc->pix_fmt;
    frame->width = avcc->width;
    frame->height = avcc->height;

    if (av_frame_get_buffer(frame, 32) < 0) {
        return 7;
    }
    return 0;
}

int compressedAVI_withinLoop(int tInd, FILE *fp, AVPacket *pkt, AVFrame *frame, AVCodecContext *avcc, bool useSimpleEncoder, isx::Image *img)
{
    fflush(stdout);
    /* make sure the frame data is writable */
    int zzz = av_frame_is_writable(frame);
    if (zzz == 1234) std::cout << "hello";
    if (av_frame_make_writable(frame) < 0) {
        return 8;
    }

    /* prepare a dummy image */
    /* Y */
    for (int y = 0; y < avcc->height; y++) {
        for (int x = 0; x < avcc->width; x++) {
            if (img == NULL)
            {
                frame->data[0][y * frame->linesize[0] + x] = 128 + 64 * (2 * (((x + tInd) / 10) % 2) - 1)*(2 * (((y - tInd) / 10) % 2) - 1);// x + y + i * 3;
            }
            else
            {
                std::vector<float> temp = img->getPixelValuesAsF32(isx::isize_t(y), isx::isize_t(x));
                frame->data[0][y * frame->linesize[0] + x] = uint8_t(255 * temp[0]);
            }
        }
    }

    /* Cb and Cr */
    for (int y = 0; y < avcc->height / 2; y++) {
        for (int x = 0; x < avcc->width / 2; x++) {
            frame->data[1][y * frame->linesize[1] + x] = 128;// 128 + y + i * 2;
            frame->data[2][y * frame->linesize[2] + x] = 128;// 64 + x + i * 5;
        }
    }
    frame->pts = tInd;

    /* encode the image */
    if (useSimpleEncoder)
    {
        int got_packet;
        compressedAVI_encode2(avcc, pkt, &got_packet, frame);
        //std::cout << "GOT PACKET: " << got_packet << std::endl;
        fwrite(pkt->data, 1, pkt->size, fp);
    }
    else
    {
        compressedAVI_encode1(avcc, frame, pkt, fp);
    }
    return 0;
}

int compressedAVI_postLoop(bool useSimpleEncoder, FILE * fp, AVFrame * & frame, AVPacket * & pkt, AVCodecContext * & avcc, uint8_t endcode[])
{
    /* flush the encoder */
    if (useSimpleEncoder)
    {
        int got_packet;
        compressedAVI_encode2(avcc, pkt, &got_packet, NULL);
        //std::cout << "GOT PACKET: " << got_packet << std::endl;
        fwrite(pkt->data, 1, pkt->size, fp);
    }
    else
    {
        compressedAVI_encode1(avcc, NULL, pkt, fp);
    }

    /* add sequence end code to have a real MPEG file */
    fwrite(endcode, 1, sizeof(endcode), fp);
    fclose(fp);

    avcodec_free_context(&avcc);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    return 0;
}

int compressedAVI_outputSampleMovie()
{
    bool useSimpleEncoder;
    char *filename;
    FILE *fp;
    AVFrame *frame;
    AVPacket *pkt;
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };

    AVCodecID codec_id;
    AVCodec *codec;

    // Initialize codec. 
    AVCodecContext *avcc;
    int ret;

    if ((ret = compressedAVI_preLoop(useSimpleEncoder, filename, fp, frame, pkt, avcc, codec_id, codec, NULL)) != 0)
    {
        return ret;
    }

    /* encode 1 second of video */
    for (int tInd = 0; tInd < 250; tInd++) {
        if ((ret = compressedAVI_withinLoop(tInd, fp, pkt, frame, avcc, useSimpleEncoder, NULL)) != 0)
        {
            return ret;
        }
    }

    if ((ret = compressedAVI_postLoop(useSimpleEncoder, fp, frame, pkt, avcc, endcode)) != 0)
    {
        return ret;
    }
    return 0;
}

void compressedAVI_listAllCodecs()
{

    AVCodec *codec;

    std::cout << std::endl;

    std::cout << "Supported codecs: " << std::endl << std::endl;
    codec = NULL;
    while (codec = av_codec_next(codec))
    {
        if (avcodec_find_encoder_by_name(codec->name) != 0)
        {
            std::cout << codec->name << std::endl;
        }
    }

    std::cout << std::endl;

    std::cout << "Unsupported codecs: " << std::endl << std::endl;
    codec = NULL;
    while (codec = av_codec_next(codec))
    {
        if (avcodec_find_encoder_by_name(codec->name) == 0)
        {
            std::cout << codec->name << std::endl;
        }
    }
}