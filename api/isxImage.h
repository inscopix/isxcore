#ifndef ISX_IMAGE_H
#define ISX_IMAGE_H

#include "isxTime.h"

#include <vector>
#include <assert.h>

namespace isx
{

/// A class implementing storage for an image along with some attribbutes
///
template <typename T>
class Image
{
public:
    /// Default constructor
    ///
    Image(){}

    /// Constructor for an image
    ///
    Image(int32_t inWidth, int32_t inHeight, int32_t inRowBytes, int32_t inNumChannels)
        : m_width(inWidth)
        , m_height(inHeight)
        , m_rowBytes(inRowBytes)
        , m_numChannels(inNumChannels)
    {
        assert(inWidth > 0 && inHeight > 0 && inRowBytes > 0 && inNumChannels > 0);
        assert(size_t(m_rowBytes) >= m_width * getPixelSizeInBytes());
        m_pixels.resize(getImageSizeInBytes());
    }

    /// Constructor for an image with a timestamp
    ///
    Image(int32_t inWidth, int32_t inHeight, int32_t inRowBytes, int32_t inNumChannels, const Time & inTimeStamp)
        : m_width(inWidth)
        , m_height(inHeight)
        , m_rowBytes(inRowBytes)
        , m_numChannels(inNumChannels)
        , m_hasTimeStamp(true)
        , m_timeStamp(inTimeStamp)
    {
        assert(inWidth > 0 && inHeight > 0 && inRowBytes > 0 && inNumChannels > 0);
        assert(size_t(m_rowBytes) >= m_width * getPixelSizeInBytes());
        m_pixels.resize(getImageSizeInBytes());
    }

    /// \return the width of this image
    ///
    int32_t
    getWidth() const
    {
        return m_width;
    }

    /// \return the height of this image
    ///
    int32_t
    getHeight() const
    {
        return m_height;
    }

    /// \return the number of bytes between the first pixels of two neighboring rows
    /// note that this could be different from getPixelSizeInBytes() * getWidth()
    ///
    int32_t
    getRowBytes() const
    {
        return m_rowBytes;
    }

    /// \return the number of bytes between the first pixels of two neighboring rows
    /// note that this could be different from getPixelSizeInBytes() * getWidth()
    ///
    int32_t
    getNumChannels() const
    {
        return m_numChannels;
    }

    /// \return the size of one pixel in bytes
    ///
    size_t
    getPixelSizeInBytes() const
    {
        return sizeof(T) * m_numChannels;
    }

    /// \return the size of this image in bytes
    ///
    size_t
    getImageSizeInBytes() const
    {
        return getRowBytes() * getHeight();
    }

    /// \return whether this image has a timestamp
    ///
    bool
    hasTimeStamp() const
    {
        return m_hasTimeStamp;
    }

    /// \return timestamp of this image
    ///
    const Time &
    getTimeStamp() const
    {
        return m_timeStamp;
    }

    /// \return the address of the first pixel in memory
    ///
    T *
    getPixels()
    {
        if (m_pixels.size() > 0)
        {
            return &m_pixels[0];
        }
        return 0;
    }

private:
    std::vector<T> m_pixels;
    int32_t m_width = 0;
    int32_t m_height = 0;
    int32_t m_rowBytes = 0;
    int32_t m_numChannels = 0;
    bool m_hasTimeStamp = false;
    Time m_timeStamp;
};

} // namespace isx

#endif // def ISX_IMAGE_H
