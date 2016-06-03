#include "isxImage.h"

#include <assert.h>

namespace isx
{

Image::Image(){}

Image::Image(Image::Format inFormat, int32_t inWidth, int32_t inHeight, int32_t inRowBytes)
    : m_format(inFormat)
    , m_rowBytes(inRowBytes)
    , m_width(inWidth)
    , m_height(inHeight)
{
    assert(m_rowBytes >= m_width * getPixelSizeInBytes());
    m_pixels.resize(getImageSizeInBytes());
}

Image::Image(Image::Format inFormat, int32_t inWidth, int32_t inHeight, int32_t inRowBytes, const Time & inTimeStamp)
    : m_format(inFormat)
    , m_rowBytes(inRowBytes)
    , m_width(inWidth)
    , m_height(inHeight)
    , m_hasTimeStamp(true)
    , m_timeStamp(inTimeStamp)
{
    assert(m_rowBytes >= m_width * getPixelSizeInBytes());
    m_pixels.resize(getImageSizeInBytes());
}

Image::Format
Image::getFormat() const
{
    return m_format;
}

int32_t
Image::getRowBytes() const
{
    return m_rowBytes;
}

int32_t
Image::getWidth() const
{
    return m_width;
}

int32_t
Image::getHeight() const
{
    return m_height;
}

size_t
Image::getPixelSizeInBytes() const
{
    switch (m_format)
    {
        case FORMAT_UNDEFINED:
            return 0;
            break;
        case FORMAT_UINT16_1:
            return sizeof(uint16_t);
            break;
    }
    return 0;
}

size_t
Image::getImageSizeInBytes() const
{
    return getRowBytes() * getHeight();
}

bool
Image::hasTimeStamp() const
{
    return m_hasTimeStamp;
}

const Time & 
Image::getTimeStamp() const
{
    return m_timeStamp;
}

void * 
Image::getPixels() const
{
    if (m_pixels.size() > 0)
    {
        return (void *)&m_pixels[0];
    }
    return 0;
}

} // namespace isx
