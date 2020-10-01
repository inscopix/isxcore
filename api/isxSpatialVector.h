#ifndef ISX_SPATIAL_VECTOR_H
#define ISX_SPATIAL_VECTOR_H

#include "isxObject.h"
#include "isxCore.h"

namespace isx
{

/// A vector in 2D space defined by x and y coordinates.
///
/// This class is templated so that it can refer to integral coordinate
/// systems, such as the pixel indices of an image, and continuous coordinate
/// systems, such as absolute coordinates in microns.
template <typename T>
class SpatialVector : public Object
{
public:

    /// Empty constructor.
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

    /// Vector addition with mixed types.
    ///
    /// \param   other  The point to add.
    /// \return         The result of adding other to this.
    template <typename TOther>
    SpatialVector<T> operator +(const SpatialVector<TOther> & other) const;

    /// Vector subtraction with mixed types.
    ///
    /// \param   other  The point to subtract.
    /// \return         The result of subtracting other from this.
    template <typename TOther>
    SpatialVector<T> operator -(const SpatialVector<TOther> & other) const;

    /// Vector multiplication with mixed types.
    ///
    /// \param   other  The point with which to multiply.
    /// \return         The result of multiplying this with other.
    template <typename TOther>
    SpatialVector<T> operator *(const SpatialVector<TOther> & other) const;

    /// Vector division with mixed types.
    ///
    /// \param   other  The point with which to divide.
    /// \return         The result of dividing this with other.
    template <typename TOther>
    SpatialVector<T> operator /(const SpatialVector<TOther> & other) const;

    /// Exact comparison.
    ///
    /// param   other   The point with which to compare.
    /// \return         True if this is exactly equal to the other point.
    bool operator ==(const SpatialVector<T> & other) const;

    /// Exact comparison.
    ///
    /// param   other   The point with which to compare.
    /// \return         False if this is exactly equal to the other point.
    bool operator !=(const SpatialVector<T> & other) const;

    // Overrides
    void serialize(std::ostream& strm) const override;

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
template <typename TOther>
SpatialVector<T>
SpatialVector<T>::operator +(const SpatialVector<TOther> & other) const
{
    return isx::SpatialVector<T>(m_x + other.m_x, m_y + other.m_y);
}

template <typename T>
template <typename TOther>
SpatialVector<T>
SpatialVector<T>::operator -(const SpatialVector<TOther> & other) const
{
    return isx::SpatialVector<T>(m_x - other.m_x, m_y - other.m_y);
}

template <typename T>
template <typename TOther>
SpatialVector<T>
SpatialVector<T>::operator *(const SpatialVector<TOther> & other) const
{
    return isx::SpatialVector<T>(m_x * other.getX(), m_y * other.getY());
}

template <typename T>
template <typename TOther>
SpatialVector<T>
SpatialVector<T>::operator /(const SpatialVector<TOther> & other) const
{
    return isx::SpatialVector<T>(m_x / other.getX(), m_y / other.getY());
}

template <typename T>
bool
SpatialVector<T>::operator ==(const SpatialVector<T> & other) const
{
    return (m_x == other.m_x) && (m_y == other.m_y);
}

template <typename T>
bool
SpatialVector<T>::operator !=(const SpatialVector<T> & other) const
{
    return !(*this == other);
}

template <typename T>
void
SpatialVector<T>::serialize(std::ostream & strm) const
{
    strm << "(" << m_x << ", " << m_y << ")";
}

} // namespace

#endif // ISX_SPATIAL_VECTOR_H
