#ifndef ISX_SPACINGINFO_H
#define ISX_SPACINGINFO_H

#include <cstdint>
#include "isxObject.h"
#include "isxPoint.h"
#include "isxRatio.h"

namespace isx
{

/// The spacing info associated with spatial samples.
///
/// This class is used to store spacing info about samples associated with
/// movies and images. It also contains some utility methods to convert
/// continuous points to pixel indices so that samples can be retrieved with
/// respect to continuous points.
class SpacingInfo : public Object
{
public:

    /// Definition of coorindate type.
    typedef isx::Ratio Coord_t;

    /// Default constructor.
    ///
    SpacingInfo();

    /// Fully specified constructor.
    ///
    /// \param topLeft      The top left corner in microns.
    /// \param pixelSize    The length of a pixel in each dimension in microns.
    /// \param numPixels    The number of pixels in each dimension.
    SpacingInfo(
        const Point<Coord_t>& topLeft,
        const Point<Coord_t>& pixelSize,
        const Point<size_t>& numPixels);

    /// \return The start time of the samples.
    ///
    const Point<Coord_t>& getTopLeft() const;

    /// \return The end time of the samples.
    ///
    Point<Coord_t> getBottomRight() const;

    /// \return The duration of one sample in seconds.
    ///
    const Point<Coord_t>& getPixelSize() const;

    /// \return The number of time samples.
    ///
    const Point<size_t>& getNumPixels() const;

    /// \return The total size in each dimension of the field of view in microns.
    ///
    Point<Coord_t> getTotalSize() const;

    // Overrides
    virtual void serialize(std::ostream& strm) const;

private:

    /// The top left corner of the field of view.
    Point<Coord_t> m_topLeft;

    /// The size of a pixel in each dimension in microns.
    Point<Coord_t> m_pixelSize;

    /// The number of pixels in each dimension.
    Point<size_t> m_numPixels;

}; // class

} // namespace

#endif // ISX_SPACINGINFO_H
