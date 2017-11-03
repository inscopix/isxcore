#include "isxExportCompressedAVI.h"
#include "isxImage.h"
#include "isxException.h"
#include "isxCore.h"
#include "isxLogicalTrace.h"
#include "isxCellSet.h"
#include "isxTime.h"
#include "isxMovie.h"
#include "isxPathUtils.h"
#include "isxCompressedAVI.h"
#include <cstring>
#include <fstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <array>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <limits>
#include <fstream>

namespace isx
{

bool
toCompressedAVIUtility(const std::string & inFileName, const std::vector<SpMovie_t> & inMovies, const isize_t& inMaxFrameIndex, AsyncCheckInCB_t & inCheckInCB, float & minVal, float & maxVal)
{
    bool firstPass = (minVal == -1 && maxVal == -1);
    if (firstPass)
    {
        minVal = std::numeric_limits<float>::max();
        maxVal = -std::numeric_limits<float>::max();
    }
    float minValLocal = -1;
    float maxValLocal = -1;

    FILE *fp;
    AVFrame *frame;
    AVPacket *pkt;
    const std::array<uint8_t, 4> endcode = {{0, 0, 1, 0xb7}};
    bool useSimpleEncoder = true;

    AVCodecID codec_id;
    AVCodec *codec;

    // Initialize codec. 
    AVCodecContext *avcc;

    int tInd = 0;

    ////////

    const std::string dirname = getDirName(inFileName);
    const std::string basename = getBaseName(inFileName);
    const std::string extension = getExtension(inFileName);

    bool cancelled = false;
    isize_t writtenFrames = 0;
    isize_t numFrames = 0;
    DurationInSeconds step, stepPrevious;
    for (auto m : inMovies)
    {
        numFrames += m->getTimingInfo().getNumTimes();
        step = m->getTimingInfo().getStep();
        //ISX_ASSERT(stepPrevious == DurationInSeconds() || step == stepPrevious);
        stepPrevious = step;
    }

    isize_t frameRate;
    if (step == DurationInSeconds())
    {
        frameRate = 25;
    }
    else
    {
        frameRate = std::lround(step.getInverse().toDouble());
    }
    frameRate = 25; // eventually remove this: needed because otherwise does not work, do not know why :(  See avcc->time_base and avcc->framerate

    size_t width = (numFrames > 10) ? (size_t(std::floor(std::log10(numFrames - 1)) + 1)) : (1);

    isize_t frame_index = 0; // frame index of current movie
    isize_t mv_counter = 0; // movie counter for each 2^16-1 frames

    for (auto m : inMovies)
    {
        for (isize_t i = 0; i < m->getTimingInfo().getNumTimes(); ++i)
        {
            if (m->getTimingInfo().isIndexValid(i))
            {
                if (frame_index == inMaxFrameIndex) // if number of frames larger inMaxFrameIndex - increase file name and dump to new one
                {
                    mv_counter++;
                    frame_index = 0;
                    std::string fn = dirname + "/" + basename + "_" + convertNumberToPaddedString(mv_counter, width) + "." + extension;
                }

                auto f = m->getFrame(i);
                auto& img = f->getImage();
                int numPixels = int(img.getWidth() * img.getHeight());
                
                if (firstPass)
                {
                    DataType dt = img.getDataType();
                    switch (dt)
                    {
                        case DataType::U16:
                            minValLocal = float(*(std::min_element<const uint16_t *>(img.getPixelsAsU16(), img.getPixelsAsU16() + numPixels)));
                            maxValLocal = float(*(std::max_element<const uint16_t *>(img.getPixelsAsU16(), img.getPixelsAsU16() + numPixels)));
                            break;
                        case DataType::F32:
                            minValLocal = *(std::min_element<const float *>(img.getPixelsAsF32(), img.getPixelsAsF32() + numPixels));
                            maxValLocal = *(std::max_element<const float *>(img.getPixelsAsF32(), img.getPixelsAsF32() + numPixels));
                            break;
                        case DataType::U8:
                            minValLocal = float(*(std::min_element<const uint8_t *>(img.getPixelsAsU8(), img.getPixelsAsU8() + numPixels)));
                            maxValLocal = float(*(std::max_element<const uint8_t *>(img.getPixelsAsU8(), img.getPixelsAsU8() + numPixels)));
                            break;
                        default:
                            break;
                    }
                    minVal = std::min(minVal, minValLocal);
                    maxVal = std::max(maxVal, maxValLocal);
                }
                else
                {
                    if (tInd == 0)
                    {
                        if (compressedAVI_preLoop(useSimpleEncoder, inFileName, fp, frame, pkt, avcc, codec_id, codec, &img, frameRate))
                        {
                            return true;
                        }
                    }
                    if (compressedAVI_withinLoop(tInd, fp, pkt, frame, avcc, useSimpleEncoder, &img, minVal, maxVal))
                    {
                        return true;
                    }
                }
                tInd++;

                frame_index++;
            }

            cancelled = inCheckInCB(float(++writtenFrames) / float(numFrames));
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
    if (!firstPass)
    {
        if (compressedAVI_postLoop(useSimpleEncoder, fp, frame, pkt, avcc, endcode))
        {
            return true;
        }
    }
    return cancelled;
}

bool
toCompressedAVI(const std::string & inFileName, const std::vector<SpMovie_t> & inMovies, const isize_t& inMaxFrameIndex, AsyncCheckInCB_t & inCheckInCB)
{
    bool cancelled;
    float minVal = -1;
    float maxVal = -1;

    cancelled = toCompressedAVIUtility(inFileName, inMovies, inMaxFrameIndex, inCheckInCB, minVal, maxVal);
    if (cancelled) return true;

    cancelled = toCompressedAVIUtility(inFileName, inMovies, inMaxFrameIndex, inCheckInCB, minVal, maxVal);
    if (cancelled) return true;
    
    return false;
}

} // namespace isx
