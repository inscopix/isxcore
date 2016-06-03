#ifndef ISX_IMAGE_H
#define ISX_IMAGE_H

#include "isxTime.h"

#include <vector>

namespace isx
{

/// A class implementing storage for an image along with some attribbutes
///
class Image
{
public:
    /// enum describing the format of this image
    enum Format
    {
        FORMAT_UNDEFINED,       ///< undefined format
        FORMAT_UINT16_1         ///< each pixel is represented by a single (1 channel) 16-bit unsigned int
    };

    /// Default constructor
    ///
    Image();

    /// Constructor for an image
    ///
    Image(Format inFormat, int32_t inWidth, int32_t inHeight, int32_t inRowBytes);

    /// Constructor for an image with a timestamp
    ///
    Image(Format inFormat, int32_t inWidth, int32_t inHeight, int32_t inRowBytes, const Time & inTimeStamp);

    /// \return the number of bytes between the first pixels of two neighboring rows
    /// note that this could be different from getPixelSizeInBytes() * getWidth()
    ///
    Format getFormat() const;

    /// \return the number of bytes between the first pixels of two neighboring rows
    /// note that this could be different from getPixelSizeInBytes() * getWidth()
    ///
    int32_t getRowBytes() const;

    /// \return the width of this image
    ///
    int32_t getWidth() const;

    /// \return the height of this image
    ///
    int32_t getHeight() const;

    /// \return the size of one pixel in bytes
    ///
    size_t getPixelSizeInBytes() const;

    /// \return the size of this image in bytes
    /// convenience function returning getRowBytes() & getHeight()
    ///
    size_t getImageSizeInBytes() const;

    /// \return whether this image has a timestamp
    ///
    bool hasTimeStamp() const;

    /// \return timestamp of this image
    ///
    const Time & getTimeStamp() const;

    /// \return the address of the first pixel in memory
    ///
    void * getPixels() const;

private:
    std::vector<uint8_t> m_pixels;
    Format m_format = FORMAT_UNDEFINED;
    int32_t m_rowBytes = 0;
    int32_t m_width = 0;
    int32_t m_height = 0;
    bool m_hasTimeStamp = false;
    Time m_timeStamp;
};

} // namespace isx

#endif // def ISX_IMAGE_H
