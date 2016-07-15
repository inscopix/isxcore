#ifndef ISX_MOVIE_H
#define ISX_MOVIE_H

#include "isxMovieInterface.h"
#include "isxCoreFwd.h"
#include "isxObject.h"
#include "isxCore.h"
#include "isxVideoFrame.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxMovieDefs.h"

#include <memory>

namespace isx
{

/// Movie class, implements MovieInterface
///
class Movie : public MovieInterface
{
public:
    
    /// Default constructor
    ///
    Movie();
    
    /// Construct a new movie from vector of existing HDF5 datasets
    /// \param inHdf5FileHandles vector of opaque HDF5 file handles
    /// \param inPaths vecotr of paths to datasets
    ///
    Movie(const std::vector<SpHdf5FileHandle_t> & inHdf5FileHandles, const std::vector<std::string> & inPaths);
    
    /// Construct movie to be read from an existing dataset.
    /// \param inHdf5FileHandle opaque HDF5 file handle from Recording.
    /// \param inPath Path to dataset
    /// \throw isx::ExceptionFileIO     If the file cannot be read.
    /// \throw isx::ExceptionDataIO     If the dataset cannot be read.
    Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath);

    /// Construct movie to be written to a new dataset.
    /// \param inHdf5FileHandle opaque HDF5 file handle from ProjectFile.
    /// \param inPath the path for the movie within the file. It will be created if it doesn't exist
    /// \param inNumFrames number of frames
    /// \param inFrameWidth number of columns in the frame
    /// \param inFrameHeight number of rows in the frame
    /// \param inFrameRate default frame rate
    /// \throw isx::ExceptionFileIO     If the file cannot be written.
    /// \throw isx::ExceptionDataIO     If the dataset cannot be written.
    Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight, isx::Ratio inFrameRate);

    /// Construct movie to be written to a new dataset.
    /// \param inHdf5FileHandle opaque HDF5 file handle from ProjectFile.
    /// \param inPath the path for the movie within the file. It will be created if it doesn't exist
    /// \param inTimingInfo     The timing information associated with the frames of the movie.
    /// \param inSpacingInfo    The spacing information associated with each frame of the movie.
    /// \throw isx::ExceptionFileIO     If the file cannot be written.
    /// \throw isx::ExceptionDataIO     If the dataset cannot be written.
    Movie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath, const TimingInfo & inTimingInfo, const SpacingInfo & inSpacingInfo);

    /// Destructor
    ///
    ~Movie();
    
    /// \return whether this is a valid movie object.
    ///
    bool
    isValid() const override;

    /// Get the frame data for given frame number.
    /// \param inFrameNumber 0-based index of frame for which to retrieve frame data
    /// \return a shared_ptr to a VideoFrame object containing the
    ///         requested frame data
    ///
    SpU16VideoFrame_t
    getFrame(isize_t inFrameNumber) override;

    /// Get the frame data for given time.
    /// \param inTime time of frame for which to retrieve frame data
    /// \return a shared_ptr to a VideoFrame object containing the
    ///         requested frame data
    ///
    SpU16VideoFrame_t
    getFrame(const Time & inTime) override;

    /// Get the frame data for given frame number, asynchronously.
    /// \param inFrameNumber 0-based index of frame for which to retrieve frame data
    /// \param inCallback function used to return the retrieved video frame
    ///
    void
    getFrameAsync(size_t inFrameNumber, MovieGetFrameCB_t inCallback) override;

    /// Get the frame data for given time.
    /// \param inTime time of frame for which to retrieve frame data
    /// \param inCallback function used to return the retrieved video frame
    ///
    void
    getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback) override;

    /// Writes a new frame to the movie dataset
    ///
    /// The file needs to be opened with write permission and the defined path for the 
    /// the movie needs to exist within the file structure for this to succeed
    ///
    /// \param inVideoFrame VideoFrame to write to this movie
    ///
    /// \throw isx::ExceptionFileIO     If the movie is invalid.
    /// \throw isx::ExceptionUserInput  If the arguments are not compatible with the movie.
    /// \throw isx::ExceptionDataIO     If write access to the dataset fails (eg when trying to write to nvista recordings).
    void
    writeFrame(const SpU16VideoFrame_t & inVideoFrame) override;

    /// \return     The timing information of a movie.
    ///
    const isx::TimingInfo &
    getTimingInfo() const override;

    /// \return     The spacing information of the movie.
    ///
    const isx::SpacingInfo &
    getSpacingInfo() const override;

    /// \return     The name of the movie.
    ///
    std::string
    getName() override;
    
    /// Serialize the object into an output stream.
    ///
    /// \param   strm    The output stream.
    void
    serialize(std::ostream& strm) const override;

private:
    class Impl;
    std::shared_ptr<Impl> m_pImpl;
    
};

typedef std::shared_ptr<Movie>  SpMovie_t;
typedef std::weak_ptr<Movie>    WpMovie_t;

} // namespace isx

#endif

