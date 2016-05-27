#include "isxHdf5FileHandle.h"
#include "isxCoreFwd.h"
#include "H5Cpp.h"

#include <memory>

namespace isx
{

Hdf5FileHandle::Hdf5FileHandle(const SpH5File_t & inFile)
: m_H5File(inFile) {}

const SpH5File_t & 
Hdf5FileHandle::get() const
{
    return m_H5File;
}

} // namespace isx
