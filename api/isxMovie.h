#ifndef ISX_MOVIE_H
#define ISX_MOVIE_H

#include "isxCoreFwd.h"
#include "isxObject.h"
#include "isxCore.h"
#include "isxVideoFrame.h"
#include "isxTimingInfo.h"
#include "isxMovieDefs.h"

namespace isx {

    ///
    /// An API for a class encapsulating a movie.
    ///

    class Movie : public Object
    {
    public:
        
        /// \return whether this is a valid movie object.
        ///
        virtual bool
            isValid() const = 0;

        /// \return the number of frames in this movie.
        ///
        virtual isize_t
            getNumFrames() const = 0;

        /// \return the width of the frames in this movie.
        ///
        virtual isize_t
            getFrameWidth() const = 0;

        /// \return the height of the frames in this movie.
        ///
        virtual isize_t
            getFrameHeight() const = 0;

        /// \return the size of each frame in bytes.
        ///
        virtual isize_t
            getFrameSizeInBytes() const = 0;

        /// Get the frame data for given frame number.
        /// \param inFrameNumber 0-based index of frame for which to retrieve frame data
        /// \return a shared_ptr to a VideoFrame object containing the
        ///         requested frame data
        ///
        virtual SpU16VideoFrame_t
            getFrame(isize_t inFrameNumber) = 0;

        /// Get the frame data for given time.
        /// \param inTime time of frame for which to retrieve frame data
        /// \return a shared_ptr to a VideoFrame object containing the
        ///         requested frame data
        ///
        virtual SpU16VideoFrame_t
            getFrame(const Time & inTime) = 0;

        /// Get the frame data for given frame number, asynchronously.
        /// \param inFrameNumber 0-based index of frame for which to retrieve frame data
        /// \param inCallback function used to return the retrieved video frame
        ///
        virtual void
            getFrameAsync(size_t inFrameNumber, MovieGetFrameCB_t inCallback) = 0;

        /// Get the frame data for given time.
        /// \param inTime time of frame for which to retrieve frame data
        /// \param inCallback function used to return the retrieved video frame
        ///
        virtual void
            getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback) = 0;

        /// \return the duration of the movie in seconds
        /// 
        virtual double
            getDurationInSeconds() const = 0;


        /// \return     The timing information of a movie.
        ///
        virtual const isx::TimingInfo &
            getTimingInfo() const = 0;

        /// \return     The name of the movie.
        ///
        virtual std::string getName() = 0;

    };

} // namespace isx

#endif

