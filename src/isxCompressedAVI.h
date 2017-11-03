#ifndef ISX_COMPRESSED_AVI_H
#define ISX_COMPRESSED_AVI_H

#include <array>

extern "C" {
#include "libavformat/avformat.h"
}

int compressedAVI_encode1(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, std::fstream & outfile);

int compressedAVI_encode2(AVCodecContext *avctx, AVPacket *pkt, int *got_packet, AVFrame *frame);

int compressedAVI_preLoop(bool useSimpleEncoder, const std::string & inFileName, std::fstream & fp, AVFrame * & frame, AVPacket * & pkt, AVCodecContext * & avcc, AVCodecID & codec_id, AVCodec * & codec, isx::Image *img = NULL, isx::isize_t inFrameRate = 25);

int compressedAVI_withinLoop(int tInd, std::fstream & fp, AVPacket *pkt, AVFrame *frame, AVCodecContext *avcc, bool useSimpleEncoder, isx::Image *img = NULL, const float minVal = -1, const float maxVal = -1);

int compressedAVI_postLoop(bool useSimpleEncoder, std::fstream & fp, AVFrame * & frame, AVPacket * & pkt, AVCodecContext * & avcc, const std::array<uint8_t, 4> & endcode);

void compressedAVI_listAllCodecs(std::vector<std::string> & supported, std::vector<std::string> & unSupported);

#endif // ISX_COMPRESSED_AVI_H
