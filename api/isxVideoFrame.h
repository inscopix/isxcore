#ifndef ISX_VIDEOFRAME_H
#define ISX_VIDEOFRAME_H

#include "isxTime.h"
#include "isxImage.h"

#include <vector>
#include <assert.h>
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
        int32_t inWidth, int32_t inHeight,
        int32_t inRowBytes, int32_t inNumChannels,
        Time inTimeStamp, size_t inFrameIndex)
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
    size_t
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

private:
    Image<T>    m_image;
    Time        m_timeStamp;
    size_t      m_frameIndex = 0;

};
} // namespace isx
#endif // def ISX_VIDEOFRAME_H
