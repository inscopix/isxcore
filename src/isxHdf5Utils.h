#ifndef ISX_HDF5_UTILS_H
#define ISX_HDF5_UTILS_H

#include <vector>

namespace isx
{

namespace internal
{

/// Gets the names of the objects immediately under a group in an HDF5 file.
///
/// \param  inFileName  The name of the HDF5 file.
/// \param  inPath      The path of the parent group in the HDF5 file.
/// \param  outNames    The names of the objects immediately under the group.
void getHdf5ObjNames(
    const std::string & inFileName,
    const std::string & inPath,
    std::vector<std::string> & outNames
);

} // namespace internal

} // namespace isx

#endif // ISX_HDF5_UTILS_H
