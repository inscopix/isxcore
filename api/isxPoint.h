#ifndef ISX_POINT_H
#define ISX_POINT_H

#include <string>
#include "isxObject.h"

namespace isx
{

/// A point in 2D space defined by x and y coorindates.
///
/// This class is templated so that it can also be used to store sizes
/// and numbers of pixels, but still consistently refer to the x and
/// y dimensions.
template <typename T>
class Point : public Object
{
public:

    /// Default constructor.
    ///
    /// Initially the coordinates are (0, 0).
    Point();

    /// Constructor that allows for specification of (x, y) coordinates.
    ///
    /// \param   x      The x coordinate.
    /// \param   y      The y coordinate.
    Point(T x, T y);

    /// \return         The x coordinate.
    ///
    T getX() const;

    /// \return         The y coordinate.
    ///
    T getY() const;

    /// Point addition where coordinate type must be the same as this.
    ///
    /// \param   other  The point to add.
    /// \return         The result of adding other to this.
    Point<T> operator +(const Point<T>& other) const;

    /// Point multiplication where coordinate type can be different to this.
    ///
    /// \param   other  The point with which to multiply.
    /// \return         The result of multiplying this with other.
    template <typename TOther>
    Point<T> operator *(const Point<TOther>& other) const;

    /// Exact comparison.
    ///
    /// param   other   The point with which to compare.
    /// \return         True if this is exactly equal to the other point.
    bool operator ==(const Point<T>& other) const;

    // Overrides
    virtual void serialize(std::ostream& strm) const;

private:

    /// The x coordinate.
    T m_x;

    /// The y coordinate.
    T m_y;

}; // class

// Implementation
template <typename T>
Point<T>::Point()
    : m_x(0)
    , m_y(0)
{
}

template <typename T>
Point<T>::Point(T x, T y)
    : m_x(x)
    , m_y(y)
{
}

template <typename T>
T
Point<T>::getX() const
{
    return m_x;
}

template <typename T>
T
Point<T>::getY() const
{
    return m_y;
}

template <typename T>
Point<T>
Point<T>::operator +(const Point<T>& other) const
{
    return isx::Point<T>(m_x + other.m_x, m_y + other.m_y);
}

template <typename T>
template <typename TOther>
Point<T>
Point<T>::operator *(const Point<TOther>& other) const
{
    return isx::Point<T>(m_x * other.getX(), m_y * other.getY());
}

template <typename T>
bool
Point<T>::operator ==(const Point<T>& other) const
{
    return (m_x == other.m_x) && (m_y == other.m_y);
}

template <typename T>
void
Point<T>::serialize(std::ostream& strm) const
{
    strm << "(" << m_x << ", " << m_y << ")";
}

} // namespace

#endif // ISX_POINT_H
