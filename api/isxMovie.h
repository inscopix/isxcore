#ifndef ISX_MOVIE_H
#define ISX_MOVIE_H

#include "isxTimingInfo.h"
#include "isxTime.h"
#include "isxCoreFwd.h"
#include "isxObject.h"
#include "isxVideoFrame.h"

#include <string>
#include <vector>
#include <memory>

namespace isx {

/// type for an nvista movie video frame
///
typedef VideoFrame<uint16_t> NvistaVideoFrame_t;

/// shared_ptr type for an nvista movie video frame
///
typedef std::shared_ptr<NvistaVideoFrame_t> SpNvistaVideoFrame_t;

///
/// A class encapsulating an nVista movie file.
///
class Movie : public Object
{
public:
    /// Default constructor.  Is a valid C++ object but not a valid Movie.
    ///
    Movie();

    /// Construct movie from a dataset in a Recording.
    /// \param inHdf5FileHandle opaque HDF5 file handle from Recording.
    /// \param inPath Path to dataset in Recording.
    ///
    Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath);
    
    /// Construct movie to be written to a Mosaic Project File.
    /// \param inHdf5FileHandle opaque HDF5 file handle from ProjectFile.
    /// \param inPath the path for the movie within the file. It will be created if it doesn't exist
    /// \param inNumFrames number of frames
    /// \param inFrameWidth number of columns in the frame
    /// \param inFrameHeight number of rows in the frame
    ///
    Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath, size_t inNumFrames, size_t inFrameWidth, size_t inFrameHeight);

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

    /// Get the frame data for given frame.
    /// \param inTime time of frame for which to retrieve frame data
    /// \return a shared_ptr to a VideoFrame object containing the
    ///         requested frame data
    ///
    SpNvistaVideoFrame_t
    getFrame(const Time & inTime);

    /// \return the duration of the movie in seconds
    /// 
    double 
    getDurationInSeconds() const;
 
    /// Writes a new frame to the movie dataset
    /// The file needs to be opened with write permission and the defined path for the 
    /// the movie needs to exist within the file structure for this to succeed
    /// \param inFrameNumber the frame number to insert
    /// \param inBuffer the buffer containing frame data
    /// \param inBufferSize size of inBuffer
    /// \return true if it succeeds
    void 
    writeFrame(size_t inFrameNumber, void * inBuffer, size_t inBufferSize);

    /// \return     The timing information of a movie.
    ///
    isx::TimingInfo getTimingInfo() const;

    /// Serialize the object into an output stream.
    ///
    /// \param   strm    The output stream.
    virtual 
    void 
    serialize(std::ostream& strm) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_pImpl;
};

} // namespace isx

#endif
