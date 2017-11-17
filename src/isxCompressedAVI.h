#ifndef ISX_COMPRESSED_AVI_H
#define ISX_COMPRESSED_AVI_H

#include <array>

extern "C" {
#include "libavformat/avformat.h"
}

/// Initialization prior to processing each iteration
/// \param  inFileName                    file name
/// \param  outFs                         file stream
/// \param  outFrame                      frame
/// \param  outPkt                        packet
/// \param  outAvcc                       codec context
/// \param  inImg                         first image
/// \param  inFrameRate                   frame rate
/// \param  inBitRate                     bit rate
/// \return  error code
int compressedAVI_preLoop(const std::string & inFileName, std::fstream & outFs, AVFrame * & outFrame, AVPacket * & outPkt, AVCodecContext * & outAvcc, const isx::Image *inImg = NULL, const isx::isize_t inFrameRate = 25, const isx::isize_t inBitRate = 400000);

/// Process each iteration
/// \param  inTInd              iteration
/// \param  inFs                file stream
/// \param  inPkt               packet
/// \param  inFrame             frame
/// \param  inAvcc              codec context
/// \param  inUseSimpleEncoder  select encoder
/// \param  inImg               image
/// \param  inMinVal            minimum pixel value in movie
/// \param  inMaxVal            maximum pixel value in movie
/// \return  error code
int compressedAVI_withinLoop(const int inTInd, std::fstream & inFs, AVPacket *inPkt, AVFrame *inFrame, AVCodecContext *inAvcc, const bool inUseSimpleEncoder, isx::Image *inImg = NULL, const float inMinVal = -1, const float inMaxVal = -1);

/// All iterations done: close file
/// \param  inUseSimpleEncoder  select encoder
/// \param  inFs                file stream
/// \param  inFrame             frame
/// \param  inPkt               packet
/// \param  inAvcc              codec context
/// \param  inEndcode           end code
/// \return  error code
int compressedAVI_postLoop(const bool inUseSimpleEncoder, std::fstream & inFs, AVFrame * & inFrame, AVPacket * & inPkt, AVCodecContext * & inAvcc, const std::array<uint8_t, 4> & inEndcode);

// Creates a list of supported codecs and a list of unsupported codecs
/// \param  outSupported    list of supported codecs
/// \param  outUnSupported  list of unsupported codecs
void compressedAVI_listAllCodecs(std::vector<std::string> & outSupported, std::vector<std::string> & outUnSupported);

#endif // ISX_COMPRESSED_AVI_H
