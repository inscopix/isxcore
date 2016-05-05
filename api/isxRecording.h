#ifndef ISX_RECORDING_H
#define ISX_RECORDING_H

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

    class Impl;
    std::unique_ptr<Impl> m_pImpl;
};


} // namespace isx

#endif // ISX_RECORDING_H
