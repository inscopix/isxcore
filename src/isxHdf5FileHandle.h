#ifndef ISX_HDF5_FILE_WRAPPER_H
#define ISX_HDF5_FILE_WRAPPER_H

#include "H5Cpp.h"

#include <memory>

namespace isx
{
typedef std::shared_ptr<H5::H5File> SpH5File_t;

/// wrapper for SpH5File_t objects so they can be opaque in our public API
class Hdf5FileHandle
{
public:
    /// constructor
    /// \ param inFile the SpH5File_t object to wrap
    ///
    explicit Hdf5FileHandle(const SpH5File_t & inFile);

    /// accessor
    /// \return the wrapped SpH5File_t object
    const SpH5File_t & 
    get() const;

private:
    SpH5File_t m_H5File;
};

} // namespace isx

#endif // def ISX_HDF5_FILE_WRAPPER_H
