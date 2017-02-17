#include "isxExport.h"
#include "isxImage.h"
#include "isxException.h"
#include "isxCore.h"
#include <tiffio.h>
#include <cstring>

#if ISX_OS_MACOS
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace isx {

class TIFFBuffer
{
public:
    TIFFBuffer(isize_t inBytes)
    {
        m_buf = _TIFFmalloc(tsize_t(inBytes));
    }
    TIFFBuffer(const TIFFBuffer &) = delete;
    TIFFBuffer & operator=(const TIFFBuffer &) = delete;
    ~TIFFBuffer()
    {
        if (m_buf != nullptr)
        {
            _TIFFfree(m_buf);
        }
    }
    void * get() const
    {
        return m_buf;
    };

private:
    void * m_buf = nullptr;
};

    
void toTiff(const std::string & inFileName, const SpImage_t & inImage)
{
    if (!inImage)
    {
        ISX_THROW(isx::ExceptionUserInput, "The image is invalid.");
    }

    if (inImage->getDataType() != DataType::F32)
    {
        ISX_THROW(isx::ExceptionUserInput, "Only floating point images are supported.");
    }

    uint32_t width = uint32_t(inImage->getSpacingInfo().getNumColumns());
    uint32_t height = uint32_t(inImage->getSpacingInfo().getNumRows()); 
    uint16_t bitsPerSample = uint16_t(getDataTypeSizeInBytes(inImage->getDataType()) * 8);

#if ISX_OS_MACOS
    const auto fd = creat(inFileName.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    TIFF *out = TIFFFdOpen(fd, inFileName.c_str(), "w");
#else
    TIFF *out = TIFFOpen(inFileName.c_str(), "w");
#endif

    if (!out)
    {
        ISX_THROW(isx::ExceptionFileIO, "Unable to open file for writing.");
    }

    TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);                             // set the width of the image
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);                           // set the height of the image
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, inImage->getNumChannels());    // set number of channels per pixel
    TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);             // how to interpret each data sample in a pixel
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, bitsPerSample);                  // set the size of the channels
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);              // set the origin of the image.
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);             // set how the components of each pixel are stored (i.e. RGBRGBRGB or R plane, then G plane, then B plane ) 
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );          // set the color space of the image data

    isize_t linebytes = inImage->getRowBytes();

    // Allocating memory to store the pixels of current row
    TIFFBuffer buf(linebytes);

    // We set the strip size of the file to be size of one row of pixels
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, 1);

    char * pixels = inImage->getPixels();
    for (uint32_t row = 0; row < height; row++)
    {
        std::memcpy(buf.get(), &pixels[row*linebytes], linebytes);
        if (TIFFWriteScanline(out, buf.get(), row, 0) < 0)
        {
            ISX_THROW(isx::ExceptionFileIO, "Error writing to output file.");
        }
        
    }

#if ISX_OS_MACOS
    TIFFCleanup(out);
    close(fd);
#else
    TIFFClose(out);
#endif
}
    
} // namespace isx
