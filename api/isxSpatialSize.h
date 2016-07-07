#ifndef ISX_SPATIAL_SIZE_H
#define ISX_SPATIAL_SIZE_H

#include "isxObject.h"
#include "isxCore.h"
#include "isxSpatialVector.h"
#include "isxAssert.h"

namespace isx
{

/// A size in 2D space defined by positive widths and heights.
///
template <typename T>
class SpatialSize : public Object
{
public:

    /// Default constructor.
    ///
    /// Initially the sizes are (1440, 1080).
    SpatialSize();

    /// Constructor that allows for specification of (x, y) coordinates.
    ///
    /// \param   width  The width.
    /// \param   height The height.
    SpatialSize(T width, T height);

    /// Constructor from a vector.
    ///
    /// \param   vector The vector containing the width and height.
    SpatialSize(const SpatialVector<T> & vector);

    /// \return         The width.
    ///
    T getWidth() const;

    /// \return         The height.
    ///
    T getHeight() const;

    /// \return         The vector containing the width and height.
    ///
    SpatialVector<T> getVector() const;

    /// Multiplication with another size of any type.
    ///
    /// \param   other  The size with which to multiply.
    /// \return         The result of multiplying this with other.
    template <typename TOther>
    SpatialSize<T> operator *(const SpatialSize<TOther> & other) const;

    /// Division with another size of any type.
    ///
    /// \param   other  The size with which to divide.
    /// \return         The result of dividing this with other.
    template <typename TOther>
    SpatialSize<T> operator /(const SpatialSize<TOther> & other) const;

    /// Exact comparison.
    ///
    /// \param   other  The size with which to compare.
    /// \return         True if this is exactly equal to the other size.
    bool operator ==(const SpatialSize<T> & other) const;

    // Overrides
    virtual void serialize(std::ostream & strm) const;

private:

    /// This handles most of the implementation.
    SpatialVector<T> m_vector;

}; // class

// Implementation
template <typename T>
SpatialSize<T>::SpatialSize()
    : m_vector(SpatialVector<T>())
{
}

template <typename T>
SpatialSize<T>::SpatialSize(T width, T height)
    : m_vector(SpatialVector<T>(width, height))
{
    ISX_ASSERT(width > 0, "Width must be positive.");
    ISX_ASSERT(height > 0, "Height must be positive.");
}

template <typename T>
SpatialSize<T>::SpatialSize(const SpatialVector<T> & vector)
    : m_vector(vector)
{
}

template <typename T>
T
SpatialSize<T>::getWidth() const
{
    return m_vector.getX();
}

template <typename T>
T
SpatialSize<T>::getHeight() const
{
    return m_vector.getY();
}

template <typename T>
SpatialVector<T>
SpatialSize<T>::getVector() const
{
    return m_vector;
}

template <typename T>
template <typename TOther>
SpatialSize<T>
SpatialSize<T>::operator *(const SpatialSize<TOther> & other) const
{
    return SpatialSize<T>(getVector() * other.getVector());
}

template <typename T>
template <typename TOther>
SpatialSize<T>
SpatialSize<T>::operator /(const SpatialSize<TOther> & other) const
{
    return SpatialSize<T>(getVector() / other.getVector());
}

template <typename T>
bool
SpatialSize<T>::operator ==(const SpatialSize<T> & other) const
{
    return getVector() == other.getVector();
}

template <typename T>
void
SpatialSize<T>::serialize(std::ostream & strm) const
{
    strm << getWidth() << " x " << getHeight();
}

} // namespace

#endif // ISX_SPATIAL_SIZE_H
