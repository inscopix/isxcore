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
    /// \param inRecording Recording from which to retrieve movie dataset.
    /// \param inPath Path to dataset in Recording.
    ///
    Movie(const tRecording_SP & inRecording, const std::string & inPath);

    /// Destructor
    /// 
    ~Movie();

    ///
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

private:
    class Impl;
    std::unique_ptr<Impl> m_pImpl;
};

} // namespace isx

#endif