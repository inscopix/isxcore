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

VideoFrame::Type 
VideoFrame::getFrameType() const
{
    return m_type;
}

void 
VideoFrame::setFrameType(VideoFrame::Type inType)
{
    m_type = inType;
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

void
VideoFrame::moveFrameContent(SpVideoFrame_t inFrame)
{
    if (!(inFrame->m_image.getSpacingInfo() == m_image.getSpacingInfo())
      || inFrame->getRowBytes() != getRowBytes()
      || inFrame->getNumChannels() != getNumChannels()
      || inFrame->getDataType() != getDataType())
    {
        ISX_THROW(ExceptionDataIO, "Attemptimg to move incompatible frame.");
    }

    m_image = std::move(inFrame->getImage());
    m_type = inFrame->getFrameType();
}

} // namespace isx
