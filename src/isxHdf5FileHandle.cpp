#include "isxHdf5FileHandle.h"
#include "isxCoreFwd.h"
#include "H5Cpp.h"

#include <memory>

namespace isx
{

Hdf5FileHandle::Hdf5FileHandle()
: m_H5File(0)
{

}

Hdf5FileHandle::Hdf5FileHandle(const SpH5File_t & inFile)
: m_H5File(inFile)
, m_isValid(true)
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


} // namespace isx
