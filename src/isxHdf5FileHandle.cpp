#include "isxHdf5FileHandle.h"
#include "isxCoreFwd.h"
#include <memory>

namespace isx
{

Hdf5FileHandle::Hdf5FileHandle()
: m_H5File(0)
{

}

Hdf5FileHandle::Hdf5FileHandle(const SpH5File_t & inFile, unsigned int accessMode)
: m_H5File(inFile)
, m_isValid(true)
, m_accessMode(accessMode)
{}

const SpH5File_t & 
Hdf5FileHandle::get() const
{
    return m_H5File;
}

bool 
Hdf5FileHandle::isValid() const
{
    return m_isValid;
}


bool 
Hdf5FileHandle::isReadOnly() const
{
    return m_accessMode == H5F_ACC_RDONLY;
}

bool 
Hdf5FileHandle::isReadWrite() const
{
    return ((m_accessMode == H5F_ACC_RDWR) || (m_accessMode == H5F_ACC_TRUNC));
}

} // namespace isx
