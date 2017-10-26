#include "isxExportCompressedAVI.h"
#include "isxImage.h"
#include "isxException.h"
#include "isxCore.h"
#include "isxTiffBuffer.h"
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

#include <tiffio.h>

// TODO: Ask Sylvana why specific macros was used for MacOS

//#if ISX_OS_MACOS
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <unistd.h>
//#endif

//#if ISX_OS_MACOS
//    fd = creat(inFileName.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
//    out = TIFFFdOpen(fd, inFileName.c_str(), "w");
//#else
//    out = TIFFOpen(inFileName.c_str(), "w");
//#endif

//#if ISX_OS_MACOS
//    TIFFCleanup(out);
//    close(fd);
//#else
//    TIFFClose(out);
//#endif

namespace isx {


uint16_t
getCompressedAVISampleFormat(DataType type)
{
    switch (type)
    {
    case DataType::F32:
        return SAMPLEFORMAT_IEEEFP;
        break;
    case DataType::U16:
    case DataType::U8:
    case DataType::RGB888:
        return SAMPLEFORMAT_UINT;
        break;
    default:
        ISX_THROW(isx::ExceptionUserInput, "Image's format is not supported.");
    }
}

void 
CompressedAVIExporter::toTiffOut(const Image * inImage)
{
    if (!inImage)
    {
        ISX_THROW(isx::ExceptionUserInput, "The image is invalid.");
    }

    uint32_t width = uint32_t(inImage->getSpacingInfo().getNumColumns());
    uint32_t height = uint32_t(inImage->getSpacingInfo().getNumRows());

    uint16_t sampleFormat = getCompressedAVISampleFormat(inImage->getDataType());
    uint16_t bitsPerSample = uint16_t(getDataTypeSizeInBytes(inImage->getDataType()) * 8);

    TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);                             // set the width of the image
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);                           // set the height of the image
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, inImage->getNumChannels());    // set number of channels per pixel
    TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, sampleFormat);             // how to interpret each data sample in a pixel
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, bitsPerSample);                  // set the size of the channels
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);              // set the origin of the image.
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);             // set how the components of each pixel are stored (i.e. RGBRGBRGB or R plane, then G plane, then B plane ) 
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);          // set the color space of the image data

    isize_t linebytes = inImage->getRowBytes();

    // Allocating memory to store the pixels of current row
    TIFFBuffer buf(linebytes);

    // We set the strip size of the file to be size of one row of pixels
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, 1);

    const char * pixels = inImage->getPixels();
    for (uint32_t row = 0; row < height; row++)
    {
        std::memcpy(buf.get(), &pixels[row*linebytes], linebytes);
        if (TIFFWriteScanline(out, buf.get(), row, 0) < 0)
        {
            ISX_THROW(isx::ExceptionFileIO, "Error writing to output file.");
        }

    }
}

CompressedAVIExporter::CompressedAVIExporter(const std::string & inFileName)
{
    out = TIFFOpen(inFileName.c_str(), "w");

    if (!out)
    {
        ISX_THROW(isx::ExceptionFileIO, "Unable to open file for writing.");
    }
}

CompressedAVIExporter::~CompressedAVIExporter()
{
    TIFFClose(out);
}


void 
CompressedAVIExporter::nextTiffDir()
{
    TIFFWriteDirectory(out);
    TIFFFlush(out);
}

bool 
toCompressedAVI(const std::string & inFileName, const std::vector<SpMovie_t> & inMovies, const isize_t& inMaxFrameIndex, AsyncCheckInCB_t & inCheckInCB)
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

    ////CompressedAVIExporter* out = new CompressedAVIExporter(inFileName); // for one movie - save to selected filess
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
    ////delete out;
    return cancelled;
}

void 
toCompressedAVI(const std::string & inFileName, const SpImage_t & inImage)
{
    ////CompressedAVIExporter out(inFileName);
    ////out.toTiffOut(inImage.get());
}

} // namespace isx
