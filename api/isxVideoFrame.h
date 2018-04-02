#ifndef ISX_VIDEO_FRAME_H
#define ISX_VIDEO_FRAME_H

#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxTime.h"
#include "isxImage.h"

namespace isx
{
/// A class representing a VideoFrame in-memory
/// This basically uses an Image and adds a timestamp and a frameIndex
///
class VideoFrame
{
public:
    /// Enumerator that indicates what kind of video frame this is
    ///
    enum class Type
    {
        VALID,      ///< A valid frame
        DROPPED,    ///< A dropped frame
        CROPPED,    ///< A frame that has been cropped out of the movie
        INGAP,       ///< A frame in between movie segments
        NOTIMESTAMP  ///< No data for the requested timestamp
    };

    /// Empty constructor for allocation only.
    ///
    VideoFrame();

    /// Constructor.
    ///
    /// \param inSpacingInfo    Spacing info of the image.
    /// \param inRowBytes       Number of bytes between column 0 of any two
    ///                         subsequent rows
    /// \param inNumChannels    Number of data channels per pixel (e.g.
    ///                         RGBA would be 4)
    /// \param inDataType       The data type of a pixel.
    /// \param inTimeStamp      The timestamp of this frame in its movie.
    /// \param inFrameIndex     The index of this frame in its movie.
    VideoFrame(
            const SpacingInfo & inSpacingInfo,
            isize_t inRowBytes,
            isize_t inNumChannels,
            DataType inDataType,
            Time inTimeStamp,
            isize_t inFrameIndex);

    /// \return the image data for this videoframe
    ///
    Image &
    getImage();

    /// \return the timestamp for this videoframe
    ///
    const Time &
    getTimeStamp() const;

    /// \return the frame index for this videoframe
    ///
    isize_t
    getFrameIndex() const;

    /// \return the width of this image
    ///
    isize_t
    getWidth() const;

    /// \return the height of this image
    ///
    isize_t
    getHeight() const;

    /// \return     The data type of each pixel in the frame.
    ///
    DataType
    getDataType() const;

    /// \return the type of video frame this is
    ///
    VideoFrame::Type
    getFrameType() const;

    /// Sets the type of video frame
    /// \param inType the type to be set
    void
    setFrameType(VideoFrame::Type inType);

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

    /// Move the image of a compatible frame into this and also use
    /// its frame type.
    /// Will fail if the new image does not match the old image's
    /// SpacingInfo, rowbytes, channels, DataType.
    ///
    /// Can be used to effectively change a VideoFrame's timestamp and frameIndex.
    ///
    /// \param inFrame  Frame from which image and type are taken from. Its image
    ///         will be invalid after successful completion of this call
    /// \throw isx::ExceptionDataIO If inImage is not compatible
    ///        with the current image in this VideoFrame
    void
    moveFrameContent(SpVideoFrame_t inFrame);

private:

    /// The image data and meta data of this frame.
    Image       m_image;

    /// The time stamp of this frame in its movie, which should correspond
    /// to the beginning of its time window.
    Time        m_timeStamp;

    /// The index of this frame in its movie.
    isize_t     m_frameIndex = 0;

    /// The type of frame for this object
    VideoFrame::Type        m_type = Type::VALID;

};

} // namespace isx

#endif // ISX_VIDEO_FRAME_H
