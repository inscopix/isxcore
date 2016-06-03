#ifndef ISX_MOVIE_H
#define ISX_MOVIE_H

#include "isxCoreFwd.h"

#include <string>
#include <vector>
#include <memory>

namespace isx {

///
/// A class encapsulating an nVista movie file.
///
class Movie
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


    /// Set the size for a new movie to be written to the file
    /// The file needs to be opened with write permission and the defined path for the 
    /// the movie needs to exist within the file structure for this to succeed
    /// \param inNumFrames total number of frames for the movie, use non-positive numbers
    /// to indicate that this is an expandible movie where the total number of frames is unknown
    /// \param inFrameWidth total number of columns in each frame
    /// \param inFrameHeight total number of rows in each frame
    /// \return true if it succeeds
    bool setMovieSize(int inNumFrames, int inFrameWidth, int inFrameHeight);

    /// Writes a new frame to the movie dataset
    /// The file needs to be opened with write permission and the defined path for the 
    /// the movie needs to exist within the file structure for this to succeed
    /// \param inFrameNumber the frame number to insert
    /// \param inBuffer the buffer containing frame data
    /// \param inBufferSize size of inBuffer
    /// \return true if it succeeds
    bool writeFrame(size_t inFrameNumber, void * inBuffer, size_t inBufferSize);

private:
    class Impl;
    std::unique_ptr<Impl> m_pImpl;
};

} // namespace isx

#endif