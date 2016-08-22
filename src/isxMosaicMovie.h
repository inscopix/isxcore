#ifndef ISX_MOSAIC_MOVIE_H
#define ISX_MOSAIC_MOVIE_H

#include "isxWritableMovie.h"
#include "isxMosaicMovieFile.h"
#include "isxAsyncTaskHandle.h"
#include "isxMutex.h"

#include <memory>
#include <map>

namespace isx
{

/// Encapsulates movie information and data.
///
/// All information and data is read from and written to one file.
/// All data IO operations (e.g. read/write frames) on that file are
/// performed by the IoQueue thread.
/// All other IO operations (e.g. read/write header) are performed
/// on the current thread.
class MosaicMovie : public WritableMovie
                  , public std::enable_shared_from_this<MosaicMovie>
{
public:

    /// Empty constructor.
    ///
    /// This creates a valid c++ object but an invalid movie.
    MosaicMovie();

    /// Destructor
    ~MosaicMovie();

    /// Read constructor.
    ///
    /// This opens an existing movie from a file.
    ///
    /// \param  inFileName  The name of the movie file.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the movie file fails.
    MosaicMovie(const std::string & inFileName);

    /// Write constructor.
    ///
    /// This creates a new movie in a file and initializes the data with
    /// zeros.
    ///
    /// \param  inFileName      The name of the movie file.
    /// \param  inTimingInfo    The timing information of the movie.
    /// \param  inSpacingInfo   The spacing information of the movie.
    /// \param  inDataType      The pixel value data type.
    ///
    /// \throw  isx::ExceptionFileIO    If writing the movie file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the movie data fails.
    MosaicMovie(const std::string & inFileName,
                const TimingInfo & inTimingInfo,
                const SpacingInfo & inSpacingInfo,
                DataType inDataType = DataType::U16);

    // Overrides - see base classes for documentation
    bool isValid() const override;

    void getFrame(isize_t inFrameNumber, SpU16VideoFrame_t & outFrame) override;

    void getFrame(isize_t inFrameNumber, SpF32VideoFrame_t & outFrame) override;

    void getFrameAsync(isize_t inFrameNumber, MovieGetU16FrameCB_t inCallback) override;

    void getFrameAsync(isize_t inFrameNumber, MovieGetF32FrameCB_t inCallback) override;

    void cancelPendingReads() override;

    void writeFrame(const SpU16VideoFrame_t & inVideoFrame) override;

    const isx::TimingInfo & getTimingInfo() const override;

    const isx::SpacingInfo & getSpacingInfo() const override;

    DataType getDataType() const override;

    std::string getName() const override;

    void serialize(std::ostream & strm) const override;

private:

    /// remove read request from our pending reads
    /// \param inReadRequestId Id of request to remove
    /// \return AsyncTaskHandle for the removed read request
    SpAsyncTaskHandle_t
    unregisterReadRequest(uint64_t inReadRequestId);
    
    /// True if the movie file is valid, false otherwise.
    bool m_valid;

    /// The shared pointer to the movie file that stores data.
    std::shared_ptr<MosaicMovieFile> m_file;
    uint64_t m_readRequestCount = 0;
    isx::Mutex m_pendingReadsMutex;
    std::map<uint64_t, SpAsyncTaskHandle_t> m_pendingReads;

    /// The shared implementation of getting different frame types synchronously.
    ///
    /// \param  inFrameNumber   The number of the frame to read.
    /// \param  outFrame        The shared pointer in which to store the frame data.
    template <typename FrameType>
    void getFrameTemplate(
            isize_t inFrameNumber,
            std::shared_ptr<FrameType> & outFrame);

    /// The shared implementation of getting different frame types asynchronously.
    ///
    /// \param  inFrameNumber   The number of the frame to read.
    /// \param  inCallback      The function used to return the retrieved video frame.
    template <typename FrameType>
    void getFrameAsyncTemplate(
            isize_t inFrameNumber,
            MovieGetFrameCB_t<FrameType> inCallback);
};

} // namespace isx

#endif // ISX_MOSAIC_MOVIE_H
