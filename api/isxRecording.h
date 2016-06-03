#ifndef ISX_RECORDING_H
#define ISX_RECORDING_H

#include "isxCoreFwd.h"

#include <string>
#include <memory>

namespace isx {

    /// File access flags
    ///
    enum RecordingOpenMode
    {
        RECOPENMODE_RDONLY = 0,
        RECOPENMODE_TRUNC
    };

/// A class for nvista recordings
/// This is considered immutable - we will never write to it
///
class Recording
{
public:
    
    /// Default constructor.  Is a valid C++ object but not a valid Recording.
    ///
    Recording();

    /// Destructor.
    ///
    ~Recording();

    /// Construct recording from a given file.
    /// \param inPath Path to recording file.
    /// \param openMode file access flag
    ///
    Recording(const std::string & inPath, RecordingOpenMode openMode = RECOPENMODE_RDONLY);

    /// \return whether this is a valid recording object.
    ///
    bool
    isValid() const;

    /// Accessor for opaque HDF5 file handle
    /// Can be used to create an isx::Movie instance
    ///
    /// \return Opaque HDF5 file handle
    ///
    SpHdf5FileHandle_t getHdf5FileHandle();

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
