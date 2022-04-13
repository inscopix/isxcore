#ifndef ISX_NVISION_MOVIE_H
#define ISX_NVISION_MOVIE_H

#include "isxMovie.h"
#include "isxNVisionMovieFile.h"

namespace isx
{
template <typename T> class IoTaskTracker;

/// Encapsulates nVision behavioural movie information and data.
/// All data IO operations are performed by the IoQueue thread.
class NVisionMovie : public Movie
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
    NVisionMovie(const std::string & inFileName);

    // Overrides - see base classes for documentation
    bool isValid() const override;

    SpVideoFrame_t getFrame(isize_t inFrameNumber) override;

    void getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback) override;

    void cancelPendingReads() override;

    const isx::TimingInfo & getTimingInfo() const override;

    const isx::TimingInfos_t &
    getTimingInfosForSeries() const override;

    const isx::SpacingInfo & getSpacingInfo() const override;

    DataType getDataType() const override;

    std::string getFileName() const override;

    void serialize(std::ostream & strm) const override;

private:
    /// True if the movie file is valid, false otherwise.
    bool m_valid;

    /// The shared pointer to the movie file that stores data.
    std::shared_ptr<NVisionMovieFile>             m_file;
    std::shared_ptr<IoTaskTracker<VideoFrame>>  m_ioTaskTracker;
};

} // namespace isx

#endif // ISX_NVISION_MOVIE_H
