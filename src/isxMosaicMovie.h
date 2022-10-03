#ifndef ISX_MOSAIC_MOVIE_H
#define ISX_MOSAIC_MOVIE_H

#include "isxWritableMovie.h"
#include "isxMosaicMovieFile.h"

#include <memory>

namespace isx
{
template <typename T> class IoTaskTracker;

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

    /// Read constructor.
    ///
    /// This opens an existing movie from a file.
    ///
    /// \param  inFileName  The name of the movie file.
    /// \param  enableWrite     Set to true to open in read-write mode
    ///
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the movie file fails.
    MosaicMovie(const std::string & inFileName, bool enableWrite = false);

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
                DataType inDataType,
                const bool inHasFrameHeaderFooter = false);

    // Overrides - see base classes for documentation
    bool isValid() const override;

    SpVideoFrame_t getFrame(isize_t inFrameNumber) override;

    void getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback) override;

    std::vector<uint16_t> getFrameHeader(const size_t inFrameNumber) override;

    std::string getFrameMetadata(const size_t inFrameNumber) override;

    std::vector<uint16_t> getFrameFooter(const size_t inFrameNumber) override;

    void cancelPendingReads() override;

    void writeFrame(const SpVideoFrame_t & inVideoFrame) override;

    void writeFrameWithHeaderFooter(const uint16_t * inBuffer) override;

    void writeFrameWithHeaderFooter(const uint16_t * inHeader, const uint16_t * inPixels, const uint16_t * inFooter) override;

    void
    closeForWriting(const TimingInfo & inTimingInfo = TimingInfo()) override;

    SpVideoFrame_t makeVideoFrame(isize_t inIndex) override;

    const isx::TimingInfo & getTimingInfo() const override;

    const isx::TimingInfos_t &
    getTimingInfosForSeries() const override;

    const isx::SpacingInfo & getSpacingInfo() const override;

    DataType getDataType() const override;

    std::string getFileName() const override;

    void serialize(std::ostream & strm) const override;

    void setExtraProperties(const std::string & inProperties) override;

    std::string getExtraProperties() const override;

    SpacingInfo getOriginalSpacingInfo() const override;

    bool hasFrameTimestamps() const override;

    uint64_t getFrameTimestamp(const isize_t inIndex) override;

    void closeFileStream() override;

private:

    /// True if the movie file is valid, false otherwise.
    bool m_valid;

    /// The shared pointer to the movie file that stores data.
    std::shared_ptr<MosaicMovieFile>            m_file;
    std::shared_ptr<IoTaskTracker<VideoFrame>>  m_ioTaskTracker;

    /// Writes to the movie file and waits for the operation to finished on the I/O thread.
    void writeAndWait(std::function<void()> inCallback, const std::string & inName);
};

} // namespace isx

#endif // ISX_MOSAIC_MOVIE_H
