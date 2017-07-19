#include "isxImage.h"
#include "isxAssert.h"
#include "isxException.h"

namespace isx
{

Image::Image()
    : m_spacingInfo(SizeInPixels_t())
{
}

Image::Image(  const SpacingInfo & inSpacingInfo,
        isize_t inRowBytes,
        isize_t inNumChannels,
        DataType inDataType)
    : m_spacingInfo(inSpacingInfo)
    , m_rowBytes(inRowBytes)
    , m_numChannels(inNumChannels)
    , m_dataType(inDataType)
{
    ISX_ASSERT(inRowBytes > 0);
    ISX_ASSERT(inNumChannels > 0);
    ISX_ASSERT(m_rowBytes >= getWidth() * getPixelSizeInBytes());
    m_pixels.reset(new char[getImageSizeInBytes()]);
}

Image::Image(Image && inOther)
: m_pixels(std::move(inOther.m_pixels))
, m_spacingInfo(inOther.m_spacingInfo)
, m_rowBytes(inOther.m_rowBytes)
, m_numChannels(inOther.m_numChannels)
, m_dataType(inOther.m_dataType)
{
}

Image & Image::operator=(Image && inOther)
{
    if (this != &inOther)
    {
        m_pixels = std::move(inOther.m_pixels);
        m_spacingInfo = inOther.m_spacingInfo;
        m_rowBytes = inOther.m_rowBytes;
        m_numChannels = inOther.m_numChannels;
        m_dataType = inOther.m_dataType;
    }
    return *this;
}
    
const SpacingInfo &
Image::getSpacingInfo() const
{
    return m_spacingInfo;
}

isize_t
Image::getWidth() const
{
    return m_spacingInfo.getNumColumns();
}

isize_t
Image::getHeight() const
{
    return m_spacingInfo.getNumRows();
}

DataType
Image::getDataType() const
{
    return m_dataType;
}

isize_t
Image::getNumChannels() const
{
    return m_numChannels;
}

isize_t
Image::getPixelSizeInBytes() const
{
    return m_numChannels * getDataTypeSizeInBytes(m_dataType);
}

isize_t
Image::getRowBytes() const
{
    return m_rowBytes;
}

isize_t
Image::getImageSizeInBytes() const
{
    return getRowBytes() * getHeight();
}

char *
Image::getPixels()
{
    return const_cast<char *>(const_cast<const Image *>(this)->getPixels());
}

const
char *
Image::getPixels() const
{
    if (m_pixels)
    {
        return &(m_pixels[0]);
    }
    return 0;
}
    
uint8_t *
Image::getPixelsAsU8()
{
    if (m_dataType != DataType::U8)
    {
        ISX_THROW(isx::ExceptionDataIO,
                "U8 pixels requested, but underlying data type is ", m_dataType);
    }
    return reinterpret_cast<uint8_t *>(getPixels());
}

uint16_t *
Image::getPixelsAsU16()
{
    return const_cast<uint16_t *>(const_cast<const Image *>(this)->getPixelsAsU16());
}

const
uint16_t *
Image::getPixelsAsU16() const
{
    if (m_dataType != DataType::U16)
    {
        ISX_THROW(isx::ExceptionDataIO,
                "U16 pixels requested, but underlying data type is ", m_dataType);
    }
    return reinterpret_cast<const uint16_t *>(getPixels());
}

float *
Image::getPixelsAsF32()
{
    return const_cast<float *>(const_cast<const Image *>(this)->getPixelsAsF32());
}

const
float *
Image::getPixelsAsF32() const
{
    if (m_dataType != DataType::F32)
    {
        ISX_THROW(isx::ExceptionDataIO,
                "F32 pixels requested, but underlying data type is ", m_dataType);
    }
    return reinterpret_cast<const float *>(getPixels());
}


float 
Image::getPixelValueAsF32(isize_t row, isize_t col)
{
    if(row > m_spacingInfo.getNumRows() || col > m_spacingInfo.getNumColumns())
    {
        ISX_THROW(ExceptionUserInput, "Bad row and/or column indices requested");
    }

    float pixVal = 0.0f;
    isize_t idx = row * m_rowBytes / getPixelSizeInBytes() + col;

    switch(m_dataType)
    {
        case DataType::U8:
        {
            uint8_t * pixels = getPixelsAsU8();
            pixVal = (float)pixels[idx];
            break;
        }
        case DataType::F32:
        {
            float * pixels = getPixelsAsF32();
            pixVal = pixels[idx];
            break;
        }
        case DataType::U16:
        {
            uint16_t * pixels = getPixelsAsU16();
            pixVal = (float)pixels[idx];
            break;
        }
        case DataType::RGB888:
        {
            // TODO: fix this bogus value that's being returned.
            // If this is only called to show the value under the mouse cursor then we should
            // probably expand this and return a string instead.
            idx = row * m_rowBytes + getPixelSizeInBytes() * col;
            auto pixels = reinterpret_cast<const uint8_t *>(getPixels());
            pixVal = float(pixels[idx + 0]) * 1000000.f + float(pixels[idx + 1]) * 1000.f + float(pixels[idx + 2]);
        }
    }

    return pixVal;
}

} // namespace isx
