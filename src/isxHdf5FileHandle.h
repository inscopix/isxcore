#ifndef ISX_HDF5_FILE_WRAPPER_H
#define ISX_HDF5_FILE_WRAPPER_H

#include "H5Cpp.h"

#include <memory>
#include <vector>
#include <string>

namespace isx
{
typedef std::shared_ptr<H5::H5File> SpH5File_t;

/// wrapper for SpH5File_t objects so they can be opaque in our public API
class Hdf5FileHandle
{
public:
    /// Default constructor
    ///
    Hdf5FileHandle();

    /// constructor
    /// \ param inFile the SpH5File_t object to wrap
    ///
    explicit Hdf5FileHandle(const SpH5File_t & inFile, unsigned int accessMode);

    /// accessor
    /// \return the wrapped SpH5File_t object
    const SpH5File_t & 
    get() const; 

    /// \return whether this is a valid Hdf5FileHandle object
    ///
    bool isValid() const;
    
    /// \return whether this is a handle to a file opened with Read Only permissions
    ///
    bool isReadOnly() const;
    
    /// \return whether this is a handle to a file opened with Read Write permissions
    ///
    bool isReadWrite() const;

    /// \param outNames a vector of objects under the hdf5 root
    ///
    void getObjNames(std::vector<std::string> & outNames);

private:
    SpH5File_t m_H5File;
    bool m_isValid = false;
    
    unsigned int m_accessMode;
};

} // namespace isx

#endif // def ISX_HDF5_FILE_WRAPPER_H
