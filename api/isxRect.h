#ifndef ISX_RECT_H
#define ISX_RECT_H

#include "isxObject.h"
#include "isxCore.h"
#include "isxSpatialPoint.h"

namespace isx
{

/// A rectangle defined by two points in space, measured from the origin
/// (top left corner of the original movie).
struct Rect
{
    /// convenience constructor 
    /// \param inTopLeft                top left corner in pixels
    /// \param inBottomRight            bottom right corner in pixels
    Rect(const SpatialPoint<int64_t> & inTopLeft, const SpatialPoint<int64_t> & inBottomRight)
        : m_topLeft(inTopLeft)
        , m_bottomRight(inBottomRight)
    {}

    /// \return the width of the cropping rectangle in pixels
    ///
    isize_t width() const;

    /// \return the height of the cropping rectangle in pixels
    ///
    isize_t height() const;

    /// \return the location of the top left corner in the x axis for the cropping rectangle in pixels
    ///
    int64_t x() const;

    /// \return the location of the top left corner in the y axis for the cropping rectangle in pixels
    ///
    int64_t y() const;

    /// Exact comparison with another rectangle.
    ///
    /// \param  inOther     The rectangle with which to compare.
    /// \return             True if this is exactly equal to the other rectangle.
    bool operator ==(const Rect & inOther) const;

    SpatialPoint<int64_t> m_topLeft;        ///< top left point of the rectangle
    SpatialPoint<int64_t> m_bottomRight;    ///< bottom right point of the rectangle
};

} // namespace isx

#endif // ISX_RECT_H
