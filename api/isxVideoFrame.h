#ifndef ISX_VIDEOFRAME_H
#define ISX_VIDEOFRAME_H

#include "isxTime.h"
#include "isxImage.h"

#include <vector>
#include <cstdint>
#include <cstddef>

namespace isx
{
/// A class representing a VideoFrame in-memory
/// This basically uses an Image and adds a timestap and a frameIndex
///
template<typename T>
class VideoFrame
{
public:
    /// Default constructor
    ///
    VideoFrame(){}

    /// Constructor
    /// \param inWidth width of the image in pixels
    /// \param inHeight height of the image in pixels
    /// \param inRowBytes number of bytes between 
    ///        column 0 of any two subsequent rows
    /// \param inNumChannels number of data channels
    ///        of type T per pixel (eg. RGBA would be 4)
    /// \param inTimeStamp timestamp of this frame
    /// \param inFrameIndex index of this frame in its movie
    ///
    VideoFrame(
        isize_t inWidth, isize_t inHeight,
        isize_t inRowBytes, isize_t inNumChannels,
        Time inTimeStamp, isize_t inFrameIndex)
        : m_image(inWidth, inHeight, inRowBytes, inNumChannels)
        , m_timeStamp(inTimeStamp)
        , m_frameIndex(inFrameIndex){}

    /// \return the timestamp for this videoframe
    ///
    const Time &
    getTimeStamp() const
    {
        return m_timeStamp;
    }

    /// \return the frame index for this videoframe
    ///
    isize_t
    getFrameIndex() const
    {
        return m_frameIndex;
    }

    /// \return the image data for this videoframe
    ///
    Image<T> &
    getImage()
    {
        return m_image;
    }

    /// \return the width of this image
    ///
    isize_t
    getWidth() const
    {
        return m_image.getWidth();
    }

    /// \return the height of this image
    ///
    isize_t
    getHeight() const
    {
        return m_image.getHeight();
    }

    /// \return the number of bytes between the first pixels of two neighboring rows
    /// note that this could be different from getPixelSizeInBytes() * getWidth()
    ///
    isize_t
    getRowBytes() const
    {
        return m_image.getRowBytes();
    }

    /// \return the number of bytes between the first pixels of two neighboring rows
    /// note that this could be different from getPixelSizeInBytes() * getWidth()
    ///
    isize_t
    getNumChannels() const
    {
        return m_image.getNumChannels();
    }

    /// \return the size of one pixel in bytes
    ///
    isize_t
    getPixelSizeInBytes() const
    {
        return m_image.getPixelSizeInBytes();
    }

    /// \return the size of this image in bytes
    ///
    isize_t
    getImageSizeInBytes() const
    {
        return m_image.getImageSizeInBytes();
    }

    /// \return the address of the first pixel in memory
    ///
    T *
    getPixels()
    {
        return m_image.getPixels();
    }

private:
    Image<T>    m_image;
    Time        m_timeStamp;
    isize_t      m_frameIndex = 0;

};
} // namespace isx
#endif // def ISX_VIDEOFRAME_H
