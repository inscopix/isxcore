#ifndef ISX_MOSAICMOVIE_H
#define ISX_MOSAICMOVIE_H

#include "isxTimingInfo.h"
#include "isxTime.h"
#include "isxCoreFwd.h"
#include "isxMovie.h"

#include <string>
#include <memory>


namespace isx {

    ///
    /// A class encapsulating a Mosaic movie file.
    ///

    class MosaicMovie : public Movie
    {
    public:


        /// Default constructor.  Is a valid C++ object but not a valid Movie.
        ///
        MosaicMovie();

        /// Construct movie to be read from a dataset.
        /// \param inHdf5FileHandle opaque HDF5 file handle from Recording.
        /// \param inPath Path to dataset in Recording.
        /// \throw isx::ExceptionFileIO     If the file cannot be read.
        /// \throw isx::ExceptionDataIO     If the dataset cannot be read.
        MosaicMovie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath);

        /// Construct movie to be written to a dataset.
        /// \param inHdf5FileHandle opaque HDF5 file handle from ProjectFile.
        /// \param inPath the path for the movie within the file. It will be created if it doesn't exist
        /// \param inNumFrames number of frames
        /// \param inFrameWidth number of columns in the frame
        /// \param inFrameHeight number of rows in the frame
        /// \param inFrameRate default frame rate
        /// \throw isx::ExceptionFileIO     If the file cannot be written.
        /// \throw isx::ExceptionDataIO     If the dataset cannot be written.
        MosaicMovie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight, isx::Ratio inFrameRate);

        /// Construct movie to be written to a dataset.
        /// \param inHdf5FileHandle opaque HDF5 file handle from ProjectFile.
        /// \param inPath the path for the movie within the file. It will be created if it doesn't exist
        /// \param inTimingInfo     The timing information associated with the frames of the movie.
        /// \param inSpacingInfo    The spacing information associated with each frame of the movie.
        /// \throw isx::ExceptionFileIO     If the file cannot be written.
        /// \throw isx::ExceptionDataIO     If the dataset cannot be written.
        MosaicMovie(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath, const TimingInfo & inTimingInfo, const SpacingInfo & inSpacingInfo);

        /// Destructor
        /// 
        ~MosaicMovie();

        /// \return whether this is a valid movie object.
        ///
        bool
        isValid() const override;

        /// \return the number of frames in this movie.
        ///
        isize_t
        getNumFrames() const override;

        /// \return the width of the frames in this movie.
        ///
        isize_t
        getFrameWidth() const override;

        /// \return the height of the frames in this movie.
        ///
        isize_t
        getFrameHeight() const override;

        /// \return the size of each frame in bytes.
        ///
        isize_t
        getFrameSizeInBytes() const override;

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

        /// \return the duration of the movie in seconds
        /// 
        double
        getDurationInSeconds() const override;

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
        writeFrame(isize_t inFrameNumber, void * inBuffer, isize_t inBufferSize);

        /// \return     The timing information of a movie.
        ///
        const isx::TimingInfo &
        getTimingInfo() const override;

        /// \return     The spacing information of the movie.
        ///
        const isx::SpacingInfo &
        getSpacingInfo() const override;

        /// Serialize the object into an output stream.
        ///
        /// \param   strm    The output stream.
        void
        serialize(std::ostream& strm) const override;

        /// Get movie name
        ///
        std::string
        getName() override;


    private:
        class Impl;
        std::shared_ptr<Impl> m_pImpl;
    };

} // namespace isx

#endif

