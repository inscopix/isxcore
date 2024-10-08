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

    // nVista seems to write TIFF files with 0 samples per pixel (or at least did at some stage).
    // This check is mainly to catch multi-channel files exported by ImageJ (e.g. RGB 8-bit).
    uint16_t channels = 0;
    TIFFGetField(m_tif, TIFFTAG_SAMPLESPERPIXEL, &channels);
    if (channels > 1)
    {
        ISX_THROW(ExceptionDataIO, "Unsupported number of channels (", channels, "). Only single channel TIFF images are supported.");
    }

    uint16_t bits;
    TIFFGetField(m_tif, TIFFTAG_BITSPERSAMPLE, &bits);
    switch (bits)
    {
        case sizeof(float) * 8:
        {
            m_dataType = DataType::F32;
            break;
        }
        case sizeof(uint16_t) * 8:
        {
            m_dataType = DataType::U16;
            break;
        }
        default:
        {
            ISX_THROW(ExceptionDataIO, "Unsupported number of bits (", bits, "). Only 16 (U16) and 32 (F32) bit images are supported.");
        }
    }

    uint32_t width, height;
    TIFFGetField(m_tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(m_tif, TIFFTAG_IMAGELENGTH, &height);

    m_frameWidth = isize_t(width);
    m_frameHeight = isize_t(height);
}

SpVideoFrame_t 
TiffMovie::getVideoFrame(isize_t inFrameNumber, const SpacingInfo & inSpacingInfo, Time inTimeStamp)
{
    const isx::SpVideoFrame_t vf = std::make_shared<isx::VideoFrame>(
        inSpacingInfo,
        isx::getDataTypeSizeInBytes(m_dataType) * inSpacingInfo.getNumColumns(),
        1, // numChannels
        m_dataType,
        inTimeStamp,
        inFrameNumber);

    getFrame(inFrameNumber, vf);
    return vf;
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
