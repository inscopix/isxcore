#ifndef ISX_SPATIAL_POINT_H
#define ISX_SPATIAL_POINT_H

#include "isxObject.h"
#include "isxCore.h"
#include "isxSpatialVector.h"

namespace isx
{

/// A point in 2D space defined by x and y coordindates w.r.t. an origin.
///
/// A point contains a SpatialVector, but does not derive from because
/// operators on points should be more restricted than operators on vectors.
template <typename T>
class SpatialPoint : public Object
{
public:

    /// Empty constructor.
    ///
    /// Initially the coordinates are (0, 0).
    SpatialPoint();

    /// Constructor that allows for specification of (x, y) coordinates.
    ///
    /// \param   x  The x coordinate.
    /// \param   y  The y coordinate.
    SpatialPoint(T x, T y);

    /// Constructor from a vector.
    ///
    /// \param   vector The vector containing the x and y.
    SpatialPoint(const SpatialVector<T> & vector);

    /// \return         The x coordinate.
    ///
    T getX() const;

    /// \return         The y coordinate.
    ///
    T getY() const;

    /// \return         The vector containing the x and y coordinates.
    ///
    SpatialVector<T> getVector() const;

    /// Addition with a vector.
    ///
    /// \param   other  The point with which to multiply.
    /// \return         The result of multiplying this with other.
    SpatialPoint<T> operator +(const SpatialVector<T> & other) const;

    /// Subtract another point from this.
    ///
    /// \param   other  The point to subtract from this.
    /// \return         The vector result of subtract the other point from this.
    SpatialVector<T> operator -(const SpatialPoint<T> & other) const;

    /// Exact comparison.
    ///
    /// \param   other  The point with which to compare.
    /// \return         True if this is exactly equal to the other point.
    bool operator ==(const SpatialPoint<T> & other) const;

    // Overrides
    void serialize(std::ostream & strm) const override;

private:

    /// This handles most of the implementation.
    SpatialVector<T> m_vector;

}; // class

// Implementation
template <typename T>
SpatialPoint<T>::SpatialPoint()
    : m_vector(SpatialVector<T>())
{
}

template <typename T>
SpatialPoint<T>::SpatialPoint(T x, T y)
    : m_vector(SpatialVector<T>(x, y))
{
}

template <typename T>
SpatialPoint<T>::SpatialPoint(const SpatialVector<T> & vector)
    : m_vector(vector)
{
}

template <typename T>
T
SpatialPoint<T>::getX() const
{
    return m_vector.getX();
}

template <typename T>
T
SpatialPoint<T>::getY() const
{
    return m_vector.getY();
}

template <typename T>
SpatialVector<T>
SpatialPoint<T>::getVector() const
{
    return m_vector;
}

template <typename T>
SpatialPoint<T>
SpatialPoint<T>::operator +(const SpatialVector<T> & other) const
{
    return SpatialPoint<T>(getVector() + other);
}

template <typename T>
SpatialVector<T>
SpatialPoint<T>::operator -(const SpatialPoint<T> & other) const
{
    return getVector() - other.getVector();
}

template <typename T>
bool
SpatialPoint<T>::operator ==(const SpatialPoint<T> & other) const
{
    return getVector() == other.getVector();
}

template <typename T>
void
SpatialPoint<T>::serialize(std::ostream & strm) const
{
    strm << "(" << getX() << ", " << getY() << ")";
}

} // namespace

#endif // ISX_SPATIAL_POINT_H
