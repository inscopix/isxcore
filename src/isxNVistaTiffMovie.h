#ifndef ISX_NVISTA_TIFF_MOVIE
#define ISX_NVISTA_TIFF_MOVIE

#include "isxMovie.h"
#include "isxCoreFwd.h"
#include "isxVideoFrame.h"
#include "isxMutex.h"
#include "isxVariant.h"

#include <memory>
#include <map>




namespace isx
{
template <typename T> class IoTaskTracker;
class TiffMovie;

/// Encapsulates a movie/recording from nVista stored in at least one TIFF file.
///
class NVistaTiffMovie : public Movie
                      , public std::enable_shared_from_this<NVistaTiffMovie>
{
public:

    /// Empty constructor.
    ///
    /// Creates a valid c++ object, but not a valid movie.
    NVistaTiffMovie();

    /// Construct a new movie from vector of existing TIFF files
    ///
    /// \param inFileName name of the top level file (XML or TIFF)
    /// \param inTiffFileNames names of TIFF files
    /// \param inTimingInfo the timing info for the movie (from external source such as xml)
    /// \param inSpacingInfo the spacing info for the movie (from external source such as xml)
    /// \param inDroppedFrames the list of frame numbers that were dropped (from external source such as xml)
    /// \param inAdditionalProperties the list of additional information for the movie (from external source such as xml)
    /// \param inNumFrames the number of frames in each of the TIFF files, which if specified
    ///                    allows us to avoid expensive IO to get this from the TIFF files
    NVistaTiffMovie(const std::string & inFileName,
        const std::vector<std::string> & inTiffFileNames,
        const TimingInfo & inTimingInfo = TimingInfo(),
        const SpacingInfo & inSpacingInfo = SpacingInfo(),
        const std::vector<isize_t> & inDroppedFrames = std::vector<isize_t>(),
        const std::map<std::string, Variant> & inAdditionalProperties = std::map<std::string, Variant>(),
        const std::vector<isize_t> & inNumFrames = {});

    /// Destructor
    ///
    ~NVistaTiffMovie();

    /// \return additional properties found in XML file (if initialized from XML)
    ///
    const std::map<std::string, Variant> & getAdditionalProperties() const;

    // Overrides
    bool
    isValid() const override;

    SpVideoFrame_t
    getFrame(isize_t inFrameNumber) override;

    void
    getFrameAsync(size_t inFrameNumber, MovieGetFrameCB_t inCallback) override;

    void
    cancelPendingReads() override;

    const isx::TimingInfo &
    getTimingInfo() const override;

    const isx::TimingInfos_t &
    getTimingInfosForSeries() const override;

    const isx::SpacingInfo &
    getSpacingInfo() const override;

    DataType
    getDataType() const override;

    std::string
    getFileName() const override;

    void
    serialize(std::ostream& strm) const override;

private:

    /// True if the movie file is valid, false otherwise.
    bool m_valid;

    /// The timing information of the movie.
    TimingInfos_t m_timingInfos;

    /// The spacing information of the movie.
    SpacingInfo m_spacingInfo;

    /// The vector of pointers to TIFF files.
    std::vector<std::unique_ptr<TiffMovie>> m_movies;

    /// The vector of cumulative frame indices.
    std::vector<isize_t> m_cumulativeFrames;

    /// The top level file name (XML or TIFF)
    std::string m_fileName;

    /// The TIFF file names
    std::vector<std::string> m_tiffFileNames;

    std::map<std::string, Variant> m_additionalProperties;

    std::shared_ptr<IoTaskTracker<VideoFrame>>   m_ioTaskTracker;

    /// Reads a frame directly from the associated file.
    ///
    /// The read occurs on whatever thread calls this function.
    SpVideoFrame_t
    getFrameInternal(isize_t inFrameNumber);

    /// \return The movie file index associated with a frame number.
    ///
    isize_t getMovieIndex(isize_t inFrameNumber);
};

typedef std::shared_ptr<NVistaTiffMovie>  SpNVistaTiffMovie_t;
typedef std::weak_ptr<NVistaTiffMovie>    WpNVistaTiffMovie_t;


} // namespace isx

#endif // ifndef ISX_NVISTA_TIFF_MOVIE
