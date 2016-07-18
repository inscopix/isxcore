#ifndef ISX_MOVIE_INTERFACE_H
#define ISX_MOVIE_INTERFACE_H

#include "isxCoreFwd.h"
#include "isxObject.h"
#include "isxCore.h"
#include "isxVideoFrame.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxMovieDefs.h"

#include <string>

namespace isx
{

///
/// Interface for Movies
///

class MovieInterface : public Object
{
public:
    
    /// \return whether this is a valid movie object.
    ///
    virtual
    bool
    isValid() const = 0;

    /// Get the frame data for given frame number.
    /// Note 1: this is a synchronized version of getFrameAsync. The actual file I/O is done
    ///         on a dedicated I/O thread.
    /// Note 2: this method returns a shared pointer to a video frame containing the actual pixel
    ///         data. Letting the returned shared pointer instance go out of scope means deleting
    ///         the video frame object, so any pointers to the contained pixe data are no longer valid.
    /// \param inFrameNumber 0-based index of frame for which to retrieve frame data
    /// \return a shared_ptr to a VideoFrame object containing the
    ///         requested frame data
    ///
    virtual
    SpU16VideoFrame_t
    getFrame(isize_t inFrameNumber) = 0;

    /// Get the frame data for given time.
    /// Note 1: this is a synchronized version of getFrameAsync. The actual file I/O is done
    ///         on a dedicated I/O thread.
    /// Note 2: this method returns a shared pointer to a video frame containing the actual pixel
    ///         data. Letting the returned shared pointer instance go out of scope means deleting
    ///         the video frame object, so any pointers to the contained pixe data are no longer valid.
    ///         
    /// \param inTime time of frame for which to retrieve frame data
    /// \return a shared_ptr to a VideoFrame object containing the
    ///         requested frame data
    ///
    virtual
    SpU16VideoFrame_t
    getFrame(const Time & inTime) = 0;

    /// Get the frame data for given frame number, asynchronously.
    /// \param inFrameNumber 0-based index of frame for which to retrieve frame data
    /// \param inCallback function used to return the retrieved video frame
    ///
    virtual
    void
    getFrameAsync(size_t inFrameNumber, MovieGetFrameCB_t inCallback) = 0;

    /// Get the frame data for given time.
    /// \param inTime time of frame for which to retrieve frame data
    /// \param inCallback function used to return the retrieved video frame
    ///
    virtual
    void
    getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback) = 0;

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

    /// \return     The timing information of a movie.
    ///
    virtual
    const isx::TimingInfo &
    getTimingInfo() const = 0;

    /// \return     The spacing information of the movie.
    ///
    virtual
    const isx::SpacingInfo &
    getSpacingInfo() const = 0;

    /// \return     The name of the movie.
    ///
    virtual
    std::string
    getName() = 0;

};

} // namespace isx

#endif // def ISX_MOVIE_INTERFACE_H

