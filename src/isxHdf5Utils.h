#ifndef ISX_HDF5_UTILS_H
#define ISX_HDF5_UTILS_H

#include "isxHdf5FileHandle.h"
#include <vector>

namespace isx
{

namespace internal
{

/// The type of the size of the array.
typedef std::vector<hsize_t> HSizeVector_t;

/// Create a data set in an HDF5 file or open it if it already exists.
///
/// \param  file        The HDF5 file in which to create the data set.
/// \param  name        The name of the data set to create.
/// \param  dataType    The data type of the data set to create (e.g. uint16)
/// \param  dataSpace   The data space of the data set to create (e.g. 1440x1080).
/// \return dataSet     The created data set.
///
/// \throw  isx::ExceptionFileIO     If the file cannot be opened.
/// \throw  isx::ExceptionDataIO     If the data set cannot be created.
H5::DataSet createHdf5DataSet(
    SpH5File_t & file,
    const std::string & name,
    const H5::DataType & dataType,
    const H5::DataSpace & dataSpace
);

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

/// Creates an HDF5 subspace as a hyperslab selected from an existing data space.
///
/// \param  space   The data space in which to select the hyperslab.
/// \param  offset  The offset of the hyperslab in the data space.
/// \param  size    The size of the hyperslab in the data space.
/// \return         The subspace selected from the existing data space.
H5::DataSpace createHdf5SubSpace(
    const H5::DataSpace & space,
    const HSizeVector_t & offset,
    const HSizeVector_t & size);

/// Creates an HDF5 space for a data buffer.
///
/// \param  size    The size of the hyperslab in the data space.
/// \return         The subspace selected from the existing data space.
H5::DataSpace createHdf5BufferSpace(
    const HSizeVector_t & size);

/// Gets the dimensions of an HDF5 data space.
///
/// \param  space   The HDF5 data space of which to get dimensions.
/// \param  dims    This is updated to store the dimensions of the space.
/// \param  maxDims This is updated to store the maximum dimensions of the space.
void getHdf5SpaceDims(
    const H5::DataSpace & space,
    HSizeVector_t & dims,
    HSizeVector_t & maxDims);

} // namespace internal

} // namespace isx

#endif // ISX_HDF5_UTILS_H
