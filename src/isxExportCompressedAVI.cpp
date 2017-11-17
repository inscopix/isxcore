#include "isxMovie.h"
#include "isxPathUtils.h"
#include "isxCompressedAVI.h"
#include "isxCellSetUtils.h"
#include <fstream>
#include <cfloat>
#include <algorithm>

namespace isx
{

bool
compressedAVIFindMinMax(const std::string & inFileName, const std::vector<SpMovie_t> & inMovies, AsyncCheckInCB_t & inCheckInCB, float & outMinVal, float & outMaxVal)
{
    outMinVal = std::numeric_limits<float>::max();
    outMaxVal = -std::numeric_limits<float>::max();

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

                float minValLocal, maxValLocal;
                getImageMinMax(img, minValLocal, maxValLocal);

                outMinVal = std::min(outMinVal, minValLocal);
                outMaxVal = std::max(outMaxVal, maxValLocal);
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
}

bool
compressedAVIOutputMovie(const std::string & inFileName, const std::vector<SpMovie_t> & inMovies, AsyncCheckInCB_t & inCheckInCB, float & minVal, float & maxVal)
{
    std::fstream fs;
    AVFrame *frame;
    AVPacket *pkt;
    const std::array<uint8_t, 4> endcode = {{0, 0, 1, 0xb7}};
    bool useSimpleEncoder = true;

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
                    if (compressedAVI_preLoop(inFileName, fs, frame, pkt, avcc, &img, frameRate))
                    {
                        return true;
                    }
                }
                if (compressedAVI_withinLoop(tInd, fs, pkt, frame, avcc, useSimpleEncoder, &img, minVal, maxVal))
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
    if (compressedAVI_postLoop(useSimpleEncoder, fs, frame, pkt, avcc, endcode))
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
