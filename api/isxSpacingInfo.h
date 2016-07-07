#ifndef ISX_SPACING_INFO_H
#define ISX_SPACING_INFO_H

#include "isxCore.h"
#include "isxObject.h"
#include "isxSpatialVector.h"
#include "isxSpatialSize.h"
#include "isxSpatialPoint.h"
#include "isxRatio.h"

namespace isx
{

// Vectors, sizes and points in pixels.
typedef SpatialVector<int64_t> VectorInPixels_t;
typedef SpatialSize<isize_t> SizeInPixels_t;
typedef SpatialPoint<isize_t> PointInPixels_t;

// Vectors, sizes and points in microns.
typedef SpatialVector<Ratio> VectorInMicrons_t;
typedef SpatialPoint<Ratio> PointInMicrons_t;
typedef SpatialSize<Ratio> SizeInMicrons_t;

/// The spacing info associated with spatial samples.
///
/// This class is used to store spacing info about samples associated with
/// movies and images.
/// It is assumed that samples are 2-dimensional, that their coordinates are
/// defined w.r.t. the top left corner as a origin and that coordinates
/// increase from left to right and top to bottom.
/// It also contains some utility methods to convert continuous points to
/// pixel indices so that samples can be retrieved with respect to continuous
/// points.
class SpacingInfo : public Object
{
public:

    /// Default constructor.
    ///
    /// Initially the number of pixels is (1440, 1080), the pixel size is
    /// (22/10, 22/10) and the top left corner is (0, 0).
    /// These defaults correspond to those of the nVista sensor.
    SpacingInfo();

    /// Fully specified constructor.
    ///
    /// The default pixel size is the same as that of the nVista sensor.
    /// The default top left corner is (0, 0) which represents the top left
    /// corner or origin of the nVista sensor.
    ///
    /// \param numPixels    The number of pixels in each dimension.
    /// \param pixelSize    The size of a pixel in each dimension in microns.
    /// \param topLeft      The top left corner in microns.
    SpacingInfo(
        const SizeInPixels_t & numPixels,
        const SizeInMicrons_t & pixelSize = SizeInMicrons_t(Ratio(22, 10), Ratio(22, 10)),
        const PointInMicrons_t & topLeft = PointInMicrons_t(0, 0));

    /// \return The top left corner of the top left pixel in microns.
    ///
    PointInMicrons_t getTopLeft() const;

    /// \return The bottom right corner of the bottom right corner in microns.
    ///
    PointInMicrons_t getBottomRight() const;

    /// \return The size of a pixel in each dimension in microns.
    ///
    SizeInMicrons_t getPixelSize() const;

    /// \return The number of pixels in each dimension.
    ///
    SizeInPixels_t getNumPixels() const;

    /// \return The number of rows of pixels.
    ///
    isize_t getNumRows() const;

    /// \return The number of columns of pixels.
    isize_t getNumColumns() const;

    /// \return The total number of pixels.
    isize_t getTotalNumPixels() const;

    /// \return The total size in each dimension of the field of view in microns.
    ///
    SizeInMicrons_t getTotalSize() const;

    /// Converts a point in pixel indices within these samples to a point in microns.
    ///
    /// The converted point in microns represents the center of the input pixel.
    /// The conversion uses clamping so that the converted point always corresponds
    /// to exactly one sample.
    /// I.e. any pixel index that exceeds the number of pixels in that dimension
    /// will set to that pixel index.
    ///
    /// \param  inPoint     The point in pixels to convert.
    /// \return             The point in microns.
    PointInMicrons_t convertPointInPixelsToMicrons(const PointInPixels_t & inPoint) const;

    /// Converts a point in microns to a point in pixel indices within these samples.
    ///
    /// The converted point in pixels is the pixel whose center is closest to the
    /// input point in microns.
    /// If the input point precedes or exceeds the bounds of these spatial samples
    /// it will be effectively be clamped so that the converted point always
    /// represents a pixel within these samples.
    ///
    /// \param  inPoint     The point in microns to convert.
    /// \return             The point in pixels.
    PointInPixels_t convertPointInMicronsToPixels(const PointInMicrons_t & inPoint) const;

    /// \param  other   The other spacing information with which to compare.
    /// \return         True if this is exactly equal to other, false otherwise.
    bool operator ==(const SpacingInfo& other) const;

    // Overrides
    virtual void serialize(std::ostream& strm) const;

private:

    /// The number of pixels in each dimension.
    SizeInPixels_t m_numPixels;

    /// The size of a pixel in each dimension in microns.
    SizeInMicrons_t m_pixelSize;

    /// The top left corner of the field of view.
    PointInMicrons_t m_topLeft;

}; // class

} // namespace

#endif // ISX_SPACING_INFO_H
