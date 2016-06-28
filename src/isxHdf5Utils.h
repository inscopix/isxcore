#ifndef ISX_HDF5_UTILS_H
#define ISX_HDF5_UTILS_H

#include "isxHdf5FileHandle.h"
#include <vector>

namespace isx
{

namespace internal
{

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

} // namespace internal

} // namespace isx

#endif // ISX_HDF5_UTILS_H
