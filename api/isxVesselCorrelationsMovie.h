#ifndef ISX_VESSEL_CORRELATION_MOVIE_H
#define ISX_VESSEL_CORRELATION_MOVIE_H

#include "isxMovie.h"
#include "isxVesselSet.h"
#include "isxDataSet.h"

namespace isx
{
    template <typename T> class IoTaskTracker;

/// Class for managing and interfacing with a vessel set correlation movie
/// Used in the VesselImageVisualizer to display a vessel correlation movie in single vessel mode.
///
class VesselCorrelationsMovie : public Movie
                  , public std::enable_shared_from_this<VesselCorrelationsMovie>
{
public:

    /// Empty constructor.
    ///
    /// This creates a valid c++ object but an invalid movie.
    VesselCorrelationsMovie();

    /// Constructor.
    ///
    /// This creates a VesselCorrelationsMovie from a VesselSet
    ///
    /// \param  inVesselSet                 The vessel set containing the correlation movie
    /// \param  inVesselId                  The id of the vessel to play
    ///
    /// \throw  isx::ExceptionFileIO        If reading from a vessel set with no correlation heatmaps
    VesselCorrelationsMovie(
        const SpVesselSet_t & inVesselSet,
        const size_t inVesselId);

    // Overrides - see base classes for documentation
    bool
    isValid()
    const override;

    SpVideoFrame_t
    getFrame(isize_t inFrameNumber)
    override;

    void getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback) override;

    void cancelPendingReads() override;

    const TimingInfo & getTimingInfo() const override;

    const TimingInfos_t & getTimingInfosForSeries() const override;

    const SpacingInfo & getSpacingInfo() const override;

    DataType getDataType() const override;

    std::string getFileName() const override;

    void serialize(std::ostream & strm) const override;

    std::string getExtraProperties() const override;

private:
    SpVesselSet_t           m_vesselSet;                ///< The vessel set containing the correlation movoie         
    size_t                  m_vesselId;                 ///< The vessel id to play
    TimingInfo              m_gaplessTimingInfo;        ///< Representation of vessel set series timing info as a single timeline
    TimingInfos_t           m_timingInfos;              ///< Timing info of each element in the vessel set series
    SpacingInfo             m_spacingInfo;              ///< Spacing info of the vessel correlation movie
};

} // namespace isx

#endif // def ISX_VESSEL_CORRELATION_MOVIE_H
