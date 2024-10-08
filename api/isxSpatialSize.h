#ifndef ISX_SPATIAL_SIZE_H
#define ISX_SPATIAL_SIZE_H

#include "isxObject.h"
#include "isxCore.h"
#include "isxSpatialVector.h"
#include "isxAssert.h"

namespace isx
{

/// A size in 2D space defined by positive x and y coordinates.
///
/// This class inherits from SpatialVector because a size really is a
/// positive vector.
template <typename T>
class SpatialSize : public SpatialVector<T>
{
public:

    /// Empty constructor.
    ///
    /// Initially the sizes are (0, 0).
    SpatialSize();

    /// Constructor that allows for specification of (x, y) sizes.
    ///
    /// \param  x       The x size or width.
    /// \param  y       The y size or height.
    SpatialSize(T x, T y);

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

    // Overrides
    void serialize(std::ostream & strm) const override;

}; // class

// Implementation
template <typename T>
SpatialSize<T>::SpatialSize()
    : SpatialVector<T>(0, 0)
{
}

template <typename T>
SpatialSize<T>::SpatialSize(T x, T y)
    : SpatialVector<T>(x, y)
{
    ISX_ASSERT(x >= 0, "Width must be non-negative.");
    ISX_ASSERT(y >= 0, "Height must be non-negative.");
}

template <typename T>
SpatialSize<T>::SpatialSize(const SpatialVector<T> & vector)
    : SpatialVector<T>(vector)
{
}

template <typename T>
T
SpatialSize<T>::getWidth() const
{
    return this->getX();
}

template <typename T>
T
SpatialSize<T>::getHeight() const
{
    return this->getY();
}

template <typename T>
void
SpatialSize<T>::serialize(std::ostream & strm) const
{
    strm << this->getX() << " x " << this->getY();
}

} // namespace

#endif // ISX_SPATIAL_SIZE_H
