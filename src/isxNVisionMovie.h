#ifndef ISX_NVISION_MOVIE_H
#define ISX_NVISION_MOVIE_H

#include "isxWritableMovie.h"
#include "isxNVisionMovieFile.h"

namespace isx
{
template <typename T> class IoTaskTracker;

/// Encapsulates nVision behavioural movie information and data.
/// All data IO operations are performed by the IoQueue thread.
class NVisionMovie : public WritableMovie
                 , public std::enable_shared_from_this<NVisionMovie>
{
public:

    /// Empty constructor.
    ///
    /// This creates a valid c++ object but an invalid movie.
    NVisionMovie();

    /// Read constructor.
    ///
    /// \param  inFileName      The name of the movie file.
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the movie file fails.
    /// \param inEnableWrite    Flag indicating whether the file should be open for read & write.
    ///                         This used for updating metadata on existing isxb movies.
    NVisionMovie(
        const std::string & inFileName,
        const bool inEnableWrite = false
    );

    NVisionMovie(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo
    );

    // Overrides - see base classes for documentation
    bool isValid() const override;

    SpVideoFrame_t getFrame(isize_t inFrameNumber) override;

    void getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback) override;

    std::string getFrameMetadata(const size_t inFrameNumber) override;

    void writeFrame(const SpVideoFrame_t & inVideoFrame) override;

    void writeFrameWithHeaderFooter(const uint16_t * inBuffer) override;

    void writeFrameWithHeaderFooter(const uint16_t * inHeader, const uint16_t * inPixels, const uint16_t * inFooter) override;

    void
    writeFrameMetadata(const std::string inMetadata) override;

    void
    closeForWriting(const TimingInfo & inTimingInfo = TimingInfo()) override;

    SpVideoFrame_t makeVideoFrame(isize_t inIndex) override;

    void cancelPendingReads() override;

    const isx::TimingInfo & getTimingInfo() const override;

    const isx::TimingInfos_t &
    getTimingInfosForSeries() const override;

    const isx::SpacingInfo & getSpacingInfo() const override;

    DataType getDataType() const override;

    std::string getFileName() const override;

    void serialize(std::ostream & strm) const override;

    bool hasFrameTimestamps() const override;

    uint64_t getFrameTimestamp(const isize_t inIndex) override;
    
    std::string getExtraProperties() const override;

    void setExtraProperties(const std::string & inProperties) override;

    // void closeForWriting();

private:
    /// True if the movie file is valid, false otherwise.
    bool m_valid;

    /// The shared pointer to the movie file that stores data.
    std::shared_ptr<NVisionMovieFile>             m_file;
    std::shared_ptr<IoTaskTracker<VideoFrame>>  m_ioTaskTracker;

    /// Writes to the movie file and waits for the operation to finished on the I/O thread.
    void writeAndWait(std::function<void()> inCallback, const std::string & inName);
};

} // namespace isx

#endif // ISX_NVISION_MOVIE_H
