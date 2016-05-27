#ifndef ISX_RECORDING_H
#define ISX_RECORDING_H

#include "isxCoreFwd.h"

#include <string>
#include <memory>

namespace isx {


/// A class for nvista recordings
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
    ///
    Recording(const std::string & inPath);

    /// \return whether this is a valid recording object.
    ///
    bool
    isValid() const;

    /// Factory method to create a movie instance for a dataset in this
    /// Recording.
    ///
    /// \return new movie instance
    ///
    tMovie_SP getMovie(const std::string & inDataSetName);

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
