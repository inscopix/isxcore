#ifndef ISX_MOVIE_H
#define ISX_MOVIE_H

#include "isxTimingInfo.h"
#include "isxCoreFwd.h"
#include "isxObject.h"

#include <string>
#include <vector>
#include <memory>

namespace isx {

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

    /// Destructor
    /// 
    ~Movie();

    /// \return whether this is a valid movie object.
    ///
    bool
    isValid() const;

    /// \return the number of frames in this movie.
    ///
    int getNumFrames() const;

    /// \return the width of the frames in this movie.
    ///
    int getFrameWidth() const;

    /// \return the height of the frames in this movie.
    ///
    int getFrameHeight() const;

    /// \return the size of each frame in bytes.
    ///
    size_t getFrameSizeInBytes() const;

    /// Get the frame data for given frame.
    /// \param inFrameNumber 0-based index of the frame to retrieve
    /// \param outBuffer pointer to memory where this function copies the frame data
    /// \param inBufferSize size of outBuffer in bytes
    ///
    void getFrame(uint32_t inFrameNumber, void * outBuffer, size_t inBufferSize);

    /// \return the duration of the movie in seconds
    /// 
    double getDurationInSeconds() const;

    /// \return     The timing information of a movie.
    ///
    isx::TimingInfo getTimingInfo() const;

    /// Serialize the object into an output stream.
    ///
    /// \param   strm    The output stream.
    virtual void serialize(std::ostream& strm) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_pImpl;
};

} // namespace isx

#endif
