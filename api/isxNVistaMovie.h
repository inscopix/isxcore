#ifndef ISX_NVISTAMOVIE_H
#define ISX_NVISTAMOVIE_H

#include "isxTimingInfo.h"
#include "isxTime.h"
#include "isxCoreFwd.h"
#include "isxMovie.h"

#include <string>
#include <vector>
#include <memory>


namespace isx {


///
/// A class encapsulating an nVista movie file.
///

class NVistaMovie : public Movie
{
public:

    /// Default constructor.  Is a valid C++ object but not a valid Movie.
    ///
    NVistaMovie();

    NVistaMovie(const std::vector<SpHdf5FileHandle_t> & inHdf5FileHandles, const std::vector<std::string> & inPaths);

    /// Construct movie from a dataset in a Recording.
    /// \param inHdf5FileHandle opaque HDF5 file handle from Recording.
    /// \param inPath Path to dataset in Recording.
    /// \throw isx::ExceptionFileIO     If the file cannot be read.
    /// \throw isx::ExceptionDataIO     If the dataset cannot be read.
    NVistaMovie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath);   
    

    /// Destructor
    /// 
    ~NVistaMovie();

    /// \return whether this is a valid movie object.
    ///
    bool
    isValid() const;

    /// \return the number of frames in this movie.
    ///
    isize_t
    getNumFrames() const;

    /// \return the width of the frames in this movie.
    ///
    isize_t
    getFrameWidth() const;

    /// \return the height of the frames in this movie.
    ///
    isize_t
    getFrameHeight() const;

    /// \return the size of each frame in bytes.
    ///
    isize_t
    getFrameSizeInBytes() const;

    /// Get the frame data for given frame number.
    /// \param inFrameNumber 0-based index of frame for which to retrieve frame data
    /// \return a shared_ptr to a VideoFrame object containing the
    ///         requested frame data
    ///
    SpU16VideoFrame_t
    getFrame(isize_t inFrameNumber);

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
    ///
    void
    getFrameAsync(size_t inFrameNumber, MovieGetFrameCB_t inCallback);

    /// Get the frame data for given time.
    /// \param inTime time of frame for which to retrieve frame data
    /// \param inCallback function used to return the retrieved video frame
    ///
    void
    getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback);

    /// \return the duration of the movie in seconds
    /// 
    double 
    getDurationInSeconds() const;

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
