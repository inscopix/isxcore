#include "isxVideoFrame.h"
#include "isxMovie.h"

namespace isx
{

VideoFrame::VideoFrame()
{
}

VideoFrame::VideoFrame(
        const SpacingInfo & inSpacingInfo,
        isize_t inRowBytes,
        isize_t inNumChannels,
        DataType inDataType,
        Time inTimeStamp,
        isize_t inFrameIndex)
        : m_image(inSpacingInfo, inRowBytes, inNumChannels, inDataType)
        , m_timeStamp(inTimeStamp)
        , m_frameIndex(inFrameIndex)
{
}

VideoFrame::VideoFrame(const SpMovie_t & inMovie, isize_t inFrameIndex)
{
    SpacingInfo spacingInfo = inMovie->getSpacingInfo();
    isize_t pixelSizeInBytes = getDataTypeSizeInBytes(inMovie->getDataType());
    isize_t rowSizeInBytes = pixelSizeInBytes * spacingInfo.getNumColumns();
    m_image = Image(spacingInfo, rowSizeInBytes, 1, inMovie->getDataType());

    m_timeStamp = inMovie->getTimingInfo().convertIndexToTime(inFrameIndex);
    m_frameIndex = inFrameIndex;
}

Image &
VideoFrame::getImage()
{
    return m_image;
}

const Time &
VideoFrame::getTimeStamp() const
{
    return m_timeStamp;
}

isize_t
VideoFrame::getFrameIndex() const
{
    return m_frameIndex;
}

isize_t
VideoFrame::getWidth() const
{
    return m_image.getWidth();
}

isize_t
VideoFrame::getHeight() const
{
    return m_image.getHeight();
}

DataType
VideoFrame::getDataType() const
{
    return m_image.getDataType();
}

isize_t
VideoFrame::getRowBytes() const
{
    return m_image.getRowBytes();
}

isize_t
VideoFrame::getNumChannels() const
{
    return m_image.getNumChannels();
}

isize_t
VideoFrame::getPixelSizeInBytes() const
{
    return m_image.getPixelSizeInBytes();
}

isize_t
VideoFrame::getImageSizeInBytes() const
{
    return m_image.getImageSizeInBytes();
}

char *
VideoFrame::getPixels()
{
    return m_image.getPixels();
}

uint16_t *
VideoFrame::getPixelsAsU16()
{
    return m_image.getPixelsAsU16();
}

float *
VideoFrame::getPixelsAsF32()
{
    return m_image.getPixelsAsF32();
}

} // namespace isx
