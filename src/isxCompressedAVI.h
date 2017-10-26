#ifndef ISX_COMPRESSED_AVI_H
#define ISX_COMPRESSED_AVI_H

#include <array>

extern "C" {
#include "libavformat/avformat.h"
}

void compressedAVI_encode1(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *outfile);

int compressedAVI_encode2(AVCodecContext *avctx, AVPacket *pkt, int *got_packet, AVFrame *frame);

int compressedAVI_preLoop(bool & useSimpleEncoder, const char * & filename, FILE * & fp, AVFrame * & frame, AVPacket * & pkt, AVCodecContext * & avcc, AVCodecID & codec_id, AVCodec * & codec, isx::Image *img = NULL);

int compressedAVI_withinLoop(int tInd, FILE *fp, AVPacket *pkt, AVFrame *frame, AVCodecContext *avcc, bool useSimpleEncoder, isx::Image *img = NULL);

int compressedAVI_postLoop(bool useSimpleEncoder, FILE * fp, AVFrame * & frame, AVPacket * & pkt, AVCodecContext * & avcc, const std::array<uint8_t, 4> & endcode);

int compressedAVI_outputSampleMovie();

void compressedAVI_listAllCodecs();

#endif // ISX_COMPRESSED_AVI_H
