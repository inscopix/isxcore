#ifndef ISX_MOVIE_H
#define ISX_MOVIE_H

#include "isxObject.h"
#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxVideoFrame.h"
#include "isxAsyncTaskResult.h"

namespace isx
{

/// The type of callback for getting a frame asynchronously
using MovieGetFrameCB_t = std::function<void(AsyncTaskResult<SpVideoFrame_t>)>;
using GetFrameCB_t = std::function<SpVideoFrame_t()>;

// It's difficult to define a hard cut-off but
// since we're using this in our recommended
// ffmpeg command line to convert unsupported
// movies it seems appropriate to use it here
// as a hard limit also.
static const int64_t sMaxSupportedGopSize = 10;    

/// Interface for Movies
///
class Movie : public Object
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
    ///         the video frame object, so any pointers to the contained pixel data are no longer valid.
    ///
    /// \param  inFrameNumber   0-based index of frame for which to retrieve frame data
    /// \return                 shared_ptr to a VideoFrame object containing the
    ///                         requested frame data
    virtual
    SpVideoFrame_t
    getFrame(isize_t inFrameNumber) = 0;

    /// Get the frame data for given frame number, asynchronously.
    /// \param inFrameNumber 0-based index of frame for which to retrieve frame data
    /// \param inCallback function used to return the retrieved video frame
    ///
    virtual
    void
    getFrameAsync(size_t inFrameNumber, MovieGetFrameCB_t inCallback) = 0;

    /// cancel all pending read requests (scheduled via getFrameAsync) for this movie
    ///
    virtual
    void
    cancelPendingReads() = 0;

    /// \return     The timing information of a movie.
    ///
    virtual
    const isx::TimingInfo &
    getTimingInfo() const = 0;

    /// \return     The TimingInfos_t of a MovieSeries.
    ///             For a regular movie this will contain one TimingInfo object
    ///             matching getTimingInfo.
    ///
    virtual
    const isx::TimingInfos_t &
    getTimingInfosForSeries() const = 0;

    /// \return     The spacing information of the movie.
    ///
    virtual
    const isx::SpacingInfo &
    getSpacingInfo() const = 0;

    /// \return     The name of the movie data file.
    ///
    virtual
    std::string
    getFileName() const = 0;

    /// \return     The data type of a pixel value.
    ///
    virtual
    DataType
    getDataType() const = 0;
};

} // namespace isx

#endif // ifndef ISX_MOVIE_H
