#include "isxTiffMovie.h"
#include "isxException.h"
#include "isxTiffBuffer.h"
#include "isxVideoFrame.h"
#include <tiffio.h>
#include <cstring>

namespace isx
{

TiffMovie::TiffMovie(const std::string & inFileName, const isize_t inNumDirectories)
{
    initialize(inFileName);
    m_numFrames = inNumDirectories;
}

TiffMovie::TiffMovie(const std::string & inFileName)
{
    initialize(inFileName);
    m_numFrames = isize_t(TIFFNumberOfDirectories(m_tif));
}

TiffMovie::~TiffMovie()
{
    TIFFClose(m_tif);
}

void
TiffMovie::initialize(const std::string & inFileName)
{
    m_fileName = inFileName;

    m_tif = TIFFOpen(inFileName.c_str(), "r");

    if(!m_tif)
    {
        ISX_THROW(ExceptionFileIO, "Failed to open TIFF file: ", m_fileName);
    }

    uint16_t bits;
    TIFFGetField(m_tif, TIFFTAG_BITSPERSAMPLE, &bits);
    switch (bits)
    {
        case sizeof(uint16_t) * 8:
        {
            m_dataType = DataType::U16;
            break;
        }
        case sizeof(uint8_t) * 8:
        {
            m_dataType = DataType::U8;
            //break; // import is not implemented yet, so it will jump to default/exception
        }
        case sizeof(float) * 8:
        {
            m_dataType = DataType::F32;
            //break;
        }
        case sizeof(uint8_t) * 8 * 3:
        {
            m_dataType = DataType::RGB888;
            //break;
        }
        default:
        {
            ISX_THROW(ExceptionDataIO, "Unsupported number of bits (", bits, "). Only 16 bit images are supported.");
        }
    }

    uint32_t width, height;
    TIFFGetField(m_tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(m_tif, TIFFTAG_IMAGELENGTH, &height);

    m_frameWidth = isize_t(width);
    m_frameHeight = isize_t(height);
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

    isize_t nbytes = m_frameWidth * m_frameHeight * getDataTypeSizeInBytes(m_dataType);
    TIFFBuffer buf(nbytes);
    char * pBuf = (char *)buf.get();

    auto numOfStrips = TIFFNumberOfStrips(m_tif);
    for (tstrip_t strip = 0; strip < numOfStrips; strip++)
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

DataType
TiffMovie::getDataType() const
{
    return m_dataType;
}

}
