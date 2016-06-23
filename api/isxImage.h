#ifndef ISX_IMAGE_H
#define ISX_IMAGE_H

#include <vector>
#include <memory>

#include "isxSpacingInfo.h"
#include "isxCore.h"
#include "isxAssert.h"

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
    Image()
        : m_spacingInfo(Point<Ratio>(0, 0), Point<Ratio>(0, 0), Point<isize_t>(0, 0))
    {
    }

    /// Constructor for an image
    /// \param inWidth width of the image in pixels
    /// \param inHeight height of the image in pixels
    /// \param inRowBytes number of bytes between 
    ///        column 0 of any two subsequent rows
    /// \param inNumChannels number of data channels
    ///        of type T per pixel (eg. RGBA would be 4)
    Image(isize_t inWidth, isize_t inHeight, isize_t inRowBytes, isize_t inNumChannels)
        : m_spacingInfo(Point<Ratio>(0, 0), Point<Ratio>(Ratio(22, 10), Ratio(22, 10)), Point<isize_t>(inWidth, inHeight))
        , m_rowBytes(inRowBytes)
        , m_numChannels(inNumChannels)
    {
        ISX_ASSERT(inWidth > 0);
        ISX_ASSERT(inHeight > 0);
        ISX_ASSERT(inRowBytes > 0);
        ISX_ASSERT(inNumChannels > 0);
        ISX_ASSERT(m_rowBytes >= inWidth * getPixelSizeInBytes());
        m_pixels.reset(new T[getImageSizeInBytes()]);
    }

    /// Constructor for an image
    /// \param inSpacingInfo spacing info of the image
    /// \param inRowBytes number of bytes between 
    ///        column 0 of any two subsequent rows
    /// \param inNumChannels number of data channels
    ///        of type T per pixel (eg. RGBA would be 4)
    Image(const SpacingInfo & inSpacingInfo, isize_t inRowBytes, isize_t inNumChannels)
        : m_spacingInfo(inSpacingInfo)
        , m_rowBytes(inRowBytes)
        , m_numChannels(inNumChannels)
    {
        ISX_ASSERT(inRowBytes > 0);
        ISX_ASSERT(inNumChannels > 0);
        ISX_ASSERT(m_rowBytes >= getWidth() * getPixelSizeInBytes());
        m_pixels.reset(new T[getImageSizeInBytes()]);
    }

    /// \return the spacing information of this image
    ///
    const SpacingInfo &
    getSpacingInfo() const
    {
        return m_spacingInfo;
    }

    /// \return the width of this image
    ///
    isize_t
    getWidth() const
    {
        return m_spacingInfo.getNumColumns();
    }

    /// \return the height of this image
    ///
    isize_t
    getHeight() const
    {
        return m_spacingInfo.getNumRows();
    }

    /// \return the number of bytes between the first pixels of two neighboring rows
    /// note that this could be different from getPixelSizeInBytes() * getWidth()
    ///
    isize_t
    getRowBytes() const
    {
        return m_rowBytes;
    }

    /// \return the number of bytes between the first pixels of two neighboring rows
    /// note that this could be different from getPixelSizeInBytes() * getWidth()
    ///
    isize_t
    getNumChannels() const
    {
        return m_numChannels;
    }

    /// \return the size of one pixel in bytes
    ///
    isize_t
    getPixelSizeInBytes() const
    {
        return sizeof(T) * m_numChannels;
    }

    /// \return the size of this image in bytes
    ///
    isize_t
    getImageSizeInBytes() const
    {
        return getRowBytes() * getHeight();
    }

    /// \return the address of the first pixel in memory
    ///
    T *
    getPixels()
    {
        if (m_pixels)
        {
            return &m_pixels[0];
        }
        return 0;
    }

private:
    std::unique_ptr<T[]> m_pixels = 0;
    SpacingInfo m_spacingInfo;
    isize_t m_rowBytes = 0;
    isize_t m_numChannels = 0;
};

} // namespace isx

#endif // def ISX_IMAGE_H
