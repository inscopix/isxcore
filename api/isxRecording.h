#ifndef ISX_RECORDING_H
#define ISX_RECORDING_H

#include "isxCoreFwd.h"
#include "isxObject.h"
#include "isxMovie.h"
#include <vector>
#include <string>
#include <memory>

namespace isx { 

/// A class for nvista recordings
/// This is considered immutable - we will never write to it
///
class Recording : public Object
{
public:
    
    /// Default constructor.  Is a valid C++ object but not a valid Recording.
    ///
    Recording();

    /// Destructor.
    ///
    ~Recording();

    /// Construct recording from a given file.
    ///
    /// \param inPath Path to recording file (can be an XML or a HDF5).
    /// \throw isx::ExceptionFileIO     If the file cannot be read.
    /// \throw isx::ExceptionDataIO     If the dataset cannot be read.
    Recording(const std::string & inPath);

    /// \return whether this is a valid recording object.
    ///
    bool
    isValid() const;

    /// Accessor for opaque HDF5 file handle
    ///
    /// \return Opaque HDF5 file handle
    ///
    SpHdf5FileHandle_t getHdf5FileHandle();

    /// Accessor for opaque HDF5 file handles
    /// Can be used to create an isx::NVistaMovie instance
    ///
    /// \return Opaque HDF5 file handles
    ///
    std::vector<SpHdf5FileHandle_t> getHdf5FileHandles() const;

    /// Serialize the object into an output stream.
    ///
    /// \param   strm    The output stream.
    void serialize(std::ostream& strm) const override;

    /// \return a pointer to a movie inside the recording
    ///
    SpMovie_t getMovie();

    /// Get file name
    ///
    std::string getName();

private:
    /// Do not copy Recordings
    ///
    Recording(const Recording &) = delete;

    /// Do not assign Recordings
    ///
    const Recording & operator=(const Recording &) = delete;

    class Impl;
    /// Internal implementation of Recording class
    ///
    std::unique_ptr<Impl> m_pImpl;
};


} // namespace isx

#endif // ISX_RECORDING_H
