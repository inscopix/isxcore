#ifndef ISX_IMAGE_H
#define ISX_IMAGE_H

#include <memory>

#include "isxSpacingInfo.h"
#include "isxCore.h"
#include "isxException.h"

namespace isx
{

/// A class implementing storage for an image along with some attribbutes
///
class Image
{
public:

    /// Empty constructor for allocation only.
    ///
    Image();

    /// Constructor.
    ///
    /// \param inSpacingInfo    Spacing info of the image.
    /// \param inRowBytes       Number of bytes between column 0 of any two
    ///                         subsequent rows
    /// \param inNumChannels    Number of data channels per pixel (e.g.
    ///                         RGBA would be 4)
    /// \param inDataType       The data type of a pixel.
    Image(  const SpacingInfo & inSpacingInfo,
            isize_t inRowBytes,
            isize_t inNumChannels,
            DataType inDataType);

    /// \return the spacing information of this image
    ///
    const SpacingInfo &
    getSpacingInfo() const;

    /// \return the width of this image
    ///
    isize_t
    getWidth() const;

    /// \return the height of this image
    ///
    isize_t
    getHeight() const;

    /// \return     The data type of each pixel in the image.
    ///
    DataType
    getDataType() const;

    /// \return the number of bytes between the first pixels of two neighboring rows
    /// note that this could be different from getPixelSizeInBytes() * getWidth()
    ///
    isize_t
    getNumChannels() const;

    /// \return the size of one pixel in bytes
    ///
    isize_t
    getPixelSizeInBytes() const;

    /// \return the number of bytes between the first pixels of two neighboring rows
    /// note that this could be different from getPixelSizeInBytes() * getWidth()
    ///
    isize_t
    getRowBytes() const;

    /// \return the size of this image in bytes
    ///
    isize_t
    getImageSizeInBytes() const;

    /// Get the address of the first byte of image data.
    ///
    /// This should always succeed as long as the image is valid.
    ///
    /// \return     The address of the first byte of image data.
    char *
    getPixels();

    /// Get the address of the first pixel of image data as uint8_t.
    ///
    /// This will fail if the underlying data type is not uint8_t.
    ///
    /// \return     The address of the first pixel of image data.
    ///
    /// \throw  isx::ExceptionDataIO    If the underlying data type is
    ///                                 not uint16_t.
    uint8_t *
    getPixelsAsU8();

    /// Get the address of the first pixel of image data as uint16_t.
    ///
    /// This will fail if the underlying data type is not uint16_t.
    ///
    /// \return     The address of the first pixel of image data.
    ///
    /// \throw  isx::ExceptionDataIO    If the underlying data type is
    ///                                 not uint16_t.
    uint16_t *
    getPixelsAsU16();

    /// Get the address of the first pixel of image data as float.
    ///
    /// This will fail if the underlying data type is not float.
    ///
    /// \return     The address of the first pixel of image data.
    ///
    /// \throw  isx::ExceptionDataIO    If the underlying data type is
    ///                                 not float.
    float *
    getPixelsAsF32();

    /// \return the value of the requested pixel as float
    /// \param row row index
    /// \param col column index
    float 
    getPixelValueAsF32(isize_t row, isize_t col);


private:

    /// The byte array used to store data.
    std::unique_ptr<char []> m_pixels = 0;

    /// The spacing information of the image.
    SpacingInfo m_spacingInfo;

    /// The number of bytes between column 0 of two consecutive rows.
    isize_t m_rowBytes = 0;

    /// The number of channels in the image.
    isize_t m_numChannels = 0;

    /// The pixel data type.
    DataType m_dataType;
};

} // namespace isx

#endif // def ISX_IMAGE_H
