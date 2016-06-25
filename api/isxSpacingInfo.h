#ifndef ISX_SPACINGINFO_H
#define ISX_SPACINGINFO_H

#include "isxCore.h"
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
    typedef Ratio Coord_t;

    /// Default constructor.
    ///
    SpacingInfo();

    /// Fully specified constructor.
    ///
    /// \param numPixels    The number of pixels in each dimension.
    /// \param pixelSize    The size of a pixel in each dimension in microns.
    /// \param topLeft      The top left corner in microns.
    SpacingInfo(
        const Point<isize_t>& numPixels,
        const Point<Coord_t>& pixelSize,
        const Point<Coord_t>& topLeft);

    /// \return The top left corner of the top left pixel in microns.
    ///
    const Point<Coord_t>& getTopLeft() const;

    /// \return The bottom right corner of the bottom right corner in microns.
    ///
    Point<Coord_t> getBottomRight() const;

    /// \return The size of a pixel in each dimension in microns.
    ///
    const Point<Coord_t>& getPixelSize() const;

    /// \return The number of pixels in each dimension.
    ///
    const Point<isize_t>& getNumPixels() const;

    /// \return The number of rows of pixels.
    ///
    isize_t getNumRows() const;

    /// \return The number of columns of pixels.
    isize_t getNumColumns() const;

    /// \return The total number of pixels.
    isize_t getTotalNumPixels() const;

    /// \return The total size in each dimension of the field of view in microns.
    ///
    Point<Coord_t> getTotalSize() const;

    /// \param  other   The other spacing information with which to compare.
    /// \return         True if this is exactly equal to other, false otherwise.
    bool operator ==(const SpacingInfo& other) const;

    // Overrides
    virtual void serialize(std::ostream& strm) const;

private:

    /// The number of pixels in each dimension.
    Point<isize_t> m_numPixels;

    /// The size of a pixel in each dimension in microns.
    Point<Coord_t> m_pixelSize;

    /// The top left corner of the field of view.
    Point<Coord_t> m_topLeft;

}; // class

} // namespace

#endif // ISX_SPACINGINFO_H
