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
class SpatialSize : public SpatialVector<T>
{
public:

    /// Default constructor.
    ///
    /// Initially the sizes are (1440, 1080).
    SpatialSize();

    /// Constructor that allows for specification of (x, y) sizes.
    ///
    /// \param  x       The x size or width.
    /// \param  y       The y size of height.
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
    virtual void serialize(std::ostream & strm) const;

}; // class

// Implementation
template <typename T>
SpatialSize<T>::SpatialSize()
    : SpatialVector<T>()
{
}

template <typename T>
SpatialSize<T>::SpatialSize(T width, T height)
    : SpatialVector<T>(width, height)
{
    ISX_ASSERT(width > 0, "Width must be positive.");
    ISX_ASSERT(height > 0, "Height must be positive.");
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
