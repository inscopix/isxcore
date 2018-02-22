#ifndef ISX_WRITABLE_MOVIE_H
#define ISX_WRITABLE_MOVIE_H

#include "isxMovie.h"

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
    writeFrame(const SpVideoFrame_t & inVideoFrame) = 0;
    
    /// Close this file for writing.  This writes the header containing
    /// metadata at the end of the file.  Any attempts to write frames after
    /// this is called will result in an exception.
    ///
    /// \param inTimingInfo  a new timing information object to be set in the file
    /// before finishing writing it
    virtual
    void
    closeForWriting(const TimingInfo & inTimingInfo = TimingInfo()) = 0;

    /// Creates a new frame indexed within this movie.
    ///
    /// This does not set the frame type (valid, dropped, etc.) for performance reasons.
    ///
    /// \param  inIndex     The index of the frame in the given movie.
    /// \return             The created frame.
    virtual
    SpVideoFrame_t
    makeVideoFrame(isize_t inIndex) = 0;

    /// Creates a new frame indexed within this movie with a specific time stamp.
    ///
    /// This does not set the frame type (valid, dropped, etc.) for performance reasons.
    ///
    /// \param  inIndex     The index of the frame in the given movie.
    /// \param  inTimeStamp The time stamp to associate with this frame.
    /// \return             The created frame.
    virtual
    SpVideoFrame_t
    makeVideoFrame(isize_t inIndex, const Time & inTimeStamp) = 0;
};

} // namespace isx

#endif // ifndef ISX_WRITABLE_MOVIE_H
