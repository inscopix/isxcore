#ifndef ISX_MOVIE_H
#define ISX_MOVIE_H

#include "isxTimingInfo.h"
#include "isxTime.h"
#include "isxCoreFwd.h"
#include "isxObject.h"
#include "isxVideoFrame.h"
#include "isxMutex.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace isx {

/// type for an nvista movie video frame
///
typedef VideoFrame<uint16_t> U16VideoFrame_t;

/// shared_ptr type for an nvista movie video frame
///
typedef std::shared_ptr<U16VideoFrame_t> SpU16VideoFrame_t;

///
/// A class encapsulating an nVista movie file.
///

class Movie : public Object
{
public:
    /// Type of callback function to use to return video frames asynchronously
    ///
    typedef std::function<void(const SpU16VideoFrame_t & inVideoFrame)> MovieGetFrameCB_t;

    /// Default constructor.  Is a valid C++ object but not a valid Movie.
    ///
    Movie();

    /// Construct movie from a dataset in a Recording.
    /// \param inHdf5FileHandle opaque HDF5 file handle from Recording.
    /// \param inPath Path to dataset in Recording.
    /// \throw isx::ExceptionFileIO     If the file cannot be read.
    /// \throw isx::ExceptionDataIO     If the dataset cannot be read.
    Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath);
    
    /// Construct movie to be written to a Mosaic Project File.
    /// \param inHdf5FileHandle opaque HDF5 file handle from ProjectFile.
    /// \param inPath the path for the movie within the file. It will be created if it doesn't exist
    /// \param inNumFrames number of frames
    /// \param inFrameWidth number of columns in the frame
    /// \param inFrameHeight number of rows in the frame
	/// \param inFrameRate default frame rate
    /// \throw isx::ExceptionFileIO     If the file cannot be written.
    /// \throw isx::ExceptionDataIO     If the dataset cannot be written.
    Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath, size_t inNumFrames, size_t inFrameWidth, size_t inFrameHeight, isx::Ratio inFrameRate);

    /// Destructor
    /// 
    ~Movie();

    /// \return whether this is a valid movie object.
    ///
    bool
    isValid() const;

    /// \return the number of frames in this movie.
    ///
    size_t
    getNumFrames() const;

    /// \return the width of the frames in this movie.
    ///
    int32_t
    getFrameWidth() const;

    /// \return the height of the frames in this movie.
    ///
    int32_t
    getFrameHeight() const;

    /// \return the size of each frame in bytes.
    ///
    size_t
    getFrameSizeInBytes() const;

    /// Get the frame data for given frame number.
    /// \param inFrameNumber 0-based index of frame for which to retrieve frame data
    /// \return a shared_ptr to a VideoFrame object containing the
    ///         requested frame data
    ///
    SpU16VideoFrame_t
    getFrame(size_t inFrameNumber);

    /// Get the frame data for given time.
    /// \param inTime time of frame for which to retrieve frame data
    /// \return a shared_ptr to a VideoFrame object containing the
    ///         requested frame data
    ///
    SpU16VideoFrame_t
    getFrame(const Time & inTime);

    /// Get the frame data for given frame number, asynchronously.
    /// \param inFrameNumber 0-based index of frame for which to retrieve frame data
    /// \param inCallback function used to return the retrieved video frame
    /// \return a shared_ptr to a VideoFrame object containing the
    ///         requested frame data
    ///
    void
    getFrameAsync(size_t inFrameNumber, MovieGetFrameCB_t inCallback);

    /// Get the frame data for given time.
    /// \param inTime time of frame for which to retrieve frame data
    /// \param inCallback function used to return the retrieved video frame
    /// \return a shared_ptr to a VideoFrame object containing the
    ///         requested frame data
    ///
    void
    getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback);

    /// \return the duration of the movie in seconds
    /// 
    double 
    getDurationInSeconds() const;

    /// Writes a new frame to the movie dataset
    ///
    /// The file needs to be opened with write permission and the defined path for the 
    /// the movie needs to exist within the file structure for this to succeed
    ///
    /// \param inFrameNumber the frame number to insert
    /// \param inBuffer the buffer containing frame data
    /// \param inBufferSize size of inBuffer
    /// \return true if it succeeds
    ///
    /// \throw isx::ExceptionFileIO     If the movie is invalid.
    /// \throw isx::ExceptionUserInput  If the arguments are not compatible with the movie.
    /// \throw isx::ExceptionDataIO     If write access to the dataset fails.
    void 
    writeFrame(size_t inFrameNumber, void * inBuffer, size_t inBufferSize);

    /// \return     The timing information of a movie.
    ///
    const isx::TimingInfo &
    getTimingInfo() const;

    /// Serialize the object into an output stream.
    ///
    /// \param   strm    The output stream.
    virtual 
    void 
    serialize(std::ostream& strm) const;

    /// Get movie name
    ///
    std::string getName();


private:
    class Impl;
    std::shared_ptr<Impl> m_pImpl;
};

} // namespace isx

#endif
