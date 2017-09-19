#include "isxTiffMovie.h"
#include "isxException.h"
#include "isxTiffBuffer.h"
#include "isxVideoFrame.h"
#include <tiffio.h>
#include <cstring>

namespace isx
{

TiffMovie::TiffMovie(const std::string & inFileName)
: m_fileName(inFileName)
{
    m_tif = TIFFOpen(inFileName.c_str(), "r");

    if(!m_tif)
    {
        ISX_THROW(ExceptionFileIO, "Failed to open TIFF file: ", m_fileName);
    }

    uint16_t bits;
    TIFFGetField(m_tif, TIFFTAG_BITSPERSAMPLE, &bits);
    if (bits != sizeof(uint16_t) * 8)
    {
        ISX_THROW(ExceptionDataIO, "Unsupported number of bits (", bits, "). Only 16 bit images are supported.");
    }

    uint32_t width, height;
    TIFFGetField(m_tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(m_tif, TIFFTAG_IMAGELENGTH, &height);

    m_frameWidth = isize_t(width);
    m_frameHeight = isize_t(height);

    m_numFrames = getNumDirectories();
}

TiffMovie::~TiffMovie()
{
    TIFFClose(m_tif);
}

void 
TiffMovie::getFrame(isize_t inFrameNumber, const SpVideoFrame_t & vf)
{
    // Seek to the right directory
    if(1 != TIFFSetDirectory(m_tif, tdir_t(inFrameNumber)))
    {
        ISX_THROW(ExceptionDataIO, "The requested frame number doesn't exist.");
    }

    // Read the image
    tsize_t size = TIFFStripSize(m_tif); 
    isize_t nbytes = m_frameWidth * m_frameHeight * sizeof(uint16_t);
    TIFFBuffer buf(nbytes);
    char * pBuf = (char *)buf.get();

    for (tstrip_t strip = 0; strip < TIFFNumberOfStrips(m_tif); strip++)
    {
        if (TIFFReadEncodedStrip(m_tif, strip, pBuf, size) == -1)
        {
            ISX_THROW(ExceptionFileIO, "Failed to read strip from TIFF file: ", m_fileName);
        }
        pBuf += size;
    }

    // Copy the data to the frame buffer
    std::memcpy(vf->getPixels(), (char *)buf.get(), nbytes);
}

isize_t 
TiffMovie::getNumFrames() const
{
    return m_numFrames;
}

isize_t
TiffMovie::getFrameWidth() const
{
    return m_frameWidth;
}

isize_t
TiffMovie::getFrameHeight() const
{
    return m_frameHeight;
}

isize_t 
TiffMovie::getNumDirectories()
{
    int dircount = 0;
    do 
    {
        dircount++;
    } while (TIFFReadDirectory(m_tif));

    return isize_t(dircount);
}


}
