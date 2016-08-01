#ifndef ISX_WRITABLE_MOVIE_H
#define ISX_WRITABLE_MOVIE_H

#include "isxMovie.h"

#include <string>

namespace isx
{

/// Interface for writable Movies
///
class WritableMovie : public Movie
{
public:

    /// Writes a new frame to the movie dataset
    /// Note: this method synchronizes with the dedicated I/O thread.
    ///
    /// The file needs to be opened with write permission and the defined path for the 
    /// the movie needs to exist within the file structure for this to succeed
    ///
    /// \param inVideoFrame VideoFrame to write to this movie
    ///
    /// \throw isx::ExceptionFileIO     If the movie is invalid.
    /// \throw isx::ExceptionUserInput  If the arguments are not compatible with the movie.
    /// \throw isx::ExceptionDataIO     If write access to the dataset fails (eg when trying to write to nvista recordings).
    virtual
    void
    writeFrame(const SpU16VideoFrame_t & inVideoFrame) = 0;

};

} // namespace isx

#endif // ifndef ISX_WRITABLE_MOVIE_H
