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

namespace isx
{

bool
toCompressedAVI(const std::string & inFileName, const std::vector<SpMovie_t> & inMovies, const isize_t& inMaxFrameIndex, AsyncCheckInCB_t & inCheckInCB)
{
    bool useSimpleEncoder;
    const char *filename;
    FILE *fp;
    AVFrame *frame;
    AVPacket *pkt;
    const std::array<uint8_t, 4> endcode = {{0, 0, 1, 0xb7}};

    AVCodecID codec_id;
    AVCodec *codec;

    // Initialize codec. 
    AVCodecContext *avcc;

    int tInd = 0;

    ////////

    const std::string dirname = getDirName(inFileName);
    const std::string basename = getBaseName(inFileName);
    const std::string extension = getExtension(inFileName);

    auto cancelled = false;
    isize_t writtenFrames = 0;
    isize_t numFrames = 0;
    for (auto m : inMovies)
    {
        numFrames += m->getTimingInfo().getNumTimes();
    }

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

                    ////delete out;
                    ////out = new CompressedAVIExporter(fn);
                }

                auto f = m->getFrame(i);
                auto& img = f->getImage();
                ////out->toTiffOut(&img);
                ////out->nextTiffDir();

                if (tInd == 0)
                {
                    compressedAVI_preLoop(useSimpleEncoder, filename, fp, frame, pkt, avcc, codec_id, codec, &img);
                }
                compressedAVI_withinLoop(tInd, fp, pkt, frame, avcc, useSimpleEncoder, &img);
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
    compressedAVI_postLoop(useSimpleEncoder, fp, frame, pkt, avcc, endcode);
    return cancelled;
}

} // namespace isx
