#ifndef ISX_SPATIAL_VECTOR_H
#define ISX_SPATIAL_VECTOR_H

#include "isxObject.h"
#include "isxCore.h"

namespace isx
{

/// A vector in 2D space defined by x and y coorindates.
///
/// This class is templated so that it can also be used to store sizes
/// and numbers of pixels, but still consistently refer to the x and
/// y dimensions.
///
/// You should likely not used this class directly, but use one of the
/// public typedefs like SizeInPixels_t, SizeInMicrons_t,
/// PointInMicrons_t, etc.
template <typename T>
class SpatialVector : public Object
{
public:

    /// Default constructor.
    ///
    /// Initially the coordinates are (0, 0).
    SpatialVector();

    /// Constructor that allows for specification of (x, y) coordinates.
    ///
    /// \param   x      The x coordinate.
    /// \param   y      The y coordinate.
    SpatialVector(T x, T y);

    /// \return         The x coordinate.
    ///
    T getX() const;

    /// \return         The y coordinate.
    ///
    T getY() const;

    /// SpatialVector addition where coordinate type must be the same as this.
    ///
    /// \param   other  The point to add.
    /// \return         The result of adding other to this.
    SpatialVector<T> operator +(const SpatialVector<T> & other) const;

    /// SpatialVector multiplication where the coordinate type is an integer.
    ///
    /// \param   other  The point with which to multiply.
    /// \return         The result of multiplying this with other.
    SpatialVector<T> operator *(const SpatialVector<isize_t> & other) const;

    /// Exact comparison.
    ///
    /// param   other   The point with which to compare.
    /// \return         True if this is exactly equal to the other point.
    bool operator ==(const SpatialVector<T> & other) const;

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
SpatialVector<T>::SpatialVector()
    : m_x(0)
    , m_y(0)
{
}

template <typename T>
SpatialVector<T>::SpatialVector(T x, T y)
    : m_x(x)
    , m_y(y)
{
}

template <typename T>
T
SpatialVector<T>::getX() const
{
    return m_x;
}

template <typename T>
T
SpatialVector<T>::getY() const
{
    return m_y;
}

template <typename T>
SpatialVector<T>
SpatialVector<T>::operator +(const SpatialVector<T> & other) const
{
    return isx::SpatialVector<T>(m_x + other.m_x, m_y + other.m_y);
}

template <typename T>
SpatialVector<T>
SpatialVector<T>::operator *(const SpatialVector<isize_t> & other) const
{
    return isx::SpatialVector<T>(m_x * other.getX(), m_y * other.getY());
}

template <typename T>
bool
SpatialVector<T>::operator ==(const SpatialVector<T> & other) const
{
    return (m_x == other.m_x) && (m_y == other.m_y);
}

template <typename T>
void
SpatialVector<T>::serialize(std::ostream & strm) const
{
    strm << "(" << m_x << ", " << m_y << ")";
}

} // namespace

#endif // ISX_SPATIAL_VECTOR_H
