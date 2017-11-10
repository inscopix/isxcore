#include "isxMovie.h"
#include "isxPathUtils.h"
#include "isxCompressedAVI.h"
#include <fstream>
#include <cfloat>
#include <algorithm>

namespace isx
{

bool
compressedAVIFindMinMax(const std::string & inFileName, const std::vector<SpMovie_t> & inMovies, AsyncCheckInCB_t & inCheckInCB, float & minVal, float & maxVal)
{
	minVal = std::numeric_limits<float>::max();// 0;// FLT_MAX;// 
	maxVal = -std::numeric_limits<float>::max();//0;// -FLT_MAX;// 

    bool cancelled = false;
    isize_t writtenFrames = 0;
    isize_t numFrames = 0;
    for (auto m : inMovies)
    {
        numFrames += m->getTimingInfo().getNumTimes();
    }

    for (auto m : inMovies)
    {
        for (isize_t i = 0; i < m->getTimingInfo().getNumTimes(); ++i)
        {
            if (m->getTimingInfo().isIndexValid(i))
            {
                auto f = m->getFrame(i);
                auto& img = f->getImage();
                int numPixels = int(img.getWidth() * img.getHeight());

				float minValLocal = -1;
				float maxValLocal = -1;
                DataType dt = img.getDataType();
                switch (dt)
                {
                case DataType::U16:
					minValLocal = float(*(std::min_element<const uint16_t *>(img.getPixelsAsU16(), img.getPixelsAsU16() + numPixels)));
                    maxValLocal = 0;//float(*(std::max_element<const uint16_t *>(img.getPixelsAsU16(), img.getPixelsAsU16() + numPixels)));
                    break;
                case DataType::F32:
                    minValLocal = 0;//*(std::min_element<const float *>(img.getPixelsAsF32(), img.getPixelsAsF32() + numPixels));
                    maxValLocal = 0;//*(std::max_element<const float *>(img.getPixelsAsF32(), img.getPixelsAsF32() + numPixels));
                    break;
                case DataType::U8:
                    minValLocal = 0;//float(*(std::min_element<const uint8_t *>(img.getPixelsAsU8(), img.getPixelsAsU8() + numPixels)));
                    maxValLocal = 0;//float(*(std::max_element<const uint8_t *>(img.getPixelsAsU8(), img.getPixelsAsU8() + numPixels)));
                    break;
                default:
                    break;
                }
                minVal = std::min(minVal, minValLocal);
                maxVal = std::max(maxVal, maxValLocal);
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
    return cancelled;
	//return true;
}

bool
compressedAVIOutputMovie(const std::string & inFileName, const std::vector<SpMovie_t> & inMovies, AsyncCheckInCB_t & inCheckInCB, float & minVal, float & maxVal)
{
    std::fstream fp;
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
    DurationInSeconds step, stepPrevious, stepFirst;
    double eps = 1e-2;
    isize_t count = 0;
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
    ISX_ASSERT(stepFirst != DurationInSeconds());
    isize_t frameRate = 25; // placeholder: use std::lround(stepFirst.getInverse().toDouble()); // TODO: REMOVE THIS AFTER BRINGING IN MP4
    isize_t frame_index = 0; // frame index of current movie

    for (auto m : inMovies)
    {
        for (isize_t i = 0; i < m->getTimingInfo().getNumTimes(); ++i)
        {
            if (m->getTimingInfo().isIndexValid(i))
            {
                auto f = m->getFrame(i);
                auto& img = f->getImage();
                
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
    if (compressedAVI_postLoop(useSimpleEncoder, fp, frame, pkt, avcc, endcode))
    {
        return true;
    }
    return cancelled;
}

bool
toCompressedAVI(const std::string & inFileName, const std::vector<SpMovie_t> & inMovies, AsyncCheckInCB_t & inCheckInCB)
{
    bool cancelled;
    float minVal = -1;
    float maxVal = -1;

    cancelled = compressedAVIFindMinMax(inFileName, inMovies, inCheckInCB, minVal, maxVal);
    if (cancelled) return true;

    cancelled = compressedAVIOutputMovie(inFileName, inMovies, inCheckInCB, minVal, maxVal);
    if (cancelled) return true;
    
    return false;
}

} // namespace isx
