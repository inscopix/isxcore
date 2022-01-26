#ifndef ISX_VESSEL_SET_SERIES_H
#define ISX_VESSEL_SET_SERIES_H

#include "isxVesselSet.h"
#include "isxCoreFwd.h"
#include "isxAsync.h"
#include "isxMutex.h"

#include <fstream>
#include <map>
#include <memory>

namespace isx
{

/// A class for a containing vessels extracted from a movie series
///
class VesselSetSeries
    : public VesselSet
    , public std::enable_shared_from_this<VesselSetSeries>
{
public:

    /// Empty constructor.
    ///
    /// This creates an invalid vessel set and is for allocation purposes only.
    VesselSetSeries();

    /// Read constructor.
    ///
    /// This opens an existing vessel set file and reads information from its
    /// header.
    ///
    /// \param  inFileNames  The name of the vessel set file to read.
    /// \param  enableWrite     Set to true to open in read-write mode
    /// \throw  isx::ExceptionFileIO    If reading the vessel set file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the vessel set file fails.
    VesselSetSeries(const std::vector<std::string> & inFileNames, bool enableWrite = false);

    /// Destructor.
    ///
    ~VesselSetSeries();

    // Overrides
    bool
    isValid() const override;

    void
    closeForWriting() override;

    std::string
    getFileName() const override;

    const isize_t
    getNumVessels() override;

    isx::TimingInfo
    getTimingInfo() const override;

    isx::TimingInfos_t
    getTimingInfosForSeries() const override;

    isx::SpacingInfo
    getSpacingInfo() const override;

    SpFTrace_t
    getTrace(isize_t inIndex) override;

    void
    getTraceAsync(isize_t inIndex, VesselSetGetTraceCB_t inCallback) override;

    SpImage_t
    getImage(isize_t inIndex) override;

    void
    getImageAsync(isize_t inIndex, VesselSetGetImageCB_t inCallback) override;

    SpVesselLine_t
    getLineEndpoints(isize_t inIndex) override;

    void
    getLineEndpointsAsync(isize_t inIndex, VesselSetGetLineEndpointsCB_t inCallback) override;

    SpFTrace_t
    getDirectionTrace(isize_t inIndex) override;

    void
    getDirectionTraceAsync(isize_t inIndex, VesselSetGetTraceCB_t inCallback) override;

    SpVesselCorrelations_t
    getCorrelations(isize_t inIndex, isize_t inFrameNumber) override;

    void
    getCorrelationsAsync(isize_t inIndex, isize_t inFrameNumber, VesselSetGetCorrelationsCB_t inCallback) override;

    void
    writeImageAndLineAndTrace(
        isize_t inIndex,
        const SpImage_t & inProjectionImage,
        const SpVesselLine_t & inLineEndpoints,
        SpFTrace_t & inTrace,
        const std::string & inName= std::string(),
        const SpFTrace_t & inDirectionTrace = nullptr,
        const SpVesselCorrelationsTrace_t & inCorrTrace = nullptr) override;

    VesselSet::VesselStatus
    getVesselStatus(isize_t inIndex) override;

    Color
    getVesselColor(isize_t inIndex) override;

    void
    setVesselStatus(isize_t inIndex, VesselSet::VesselStatus inStatus) override;

    void
    setVesselColor(isize_t inIndex, const Color& inColor) override;

    void
    setVesselColors(const IdColorPairs &inColors) override;

    std::string
    getVesselStatusString(isize_t inIndex) override;

    std::string
    getVesselName(isize_t inIndex) override;

    void
    setVesselName(isize_t inIndex, const std::string & inName) override;

    std::vector<bool>
    getVesselActivity(isize_t inIndex) const override;

    void
    setVesselActive(isize_t inIndex, const std::vector<bool> & inActive) override;

    void
    cancelPendingReads() override;

    std::vector<uint16_t>
    getEfocusValues() override;

    void
    setEfocusValues(const std::vector<uint16_t> & inEfocus) override;

    std::string
    getExtraProperties() const override;

    void
    setExtraProperties(const std::string & inProperties) override;

    SpacingInfo
    getOriginalSpacingInfo() const override;

    VesselSetType_t
    getVesselSetType() const override;

    SizeInPixels_t
    getCorrelationSize(size_t inIndex) const override;

    float
    getMaxVelocity(size_t inIndex) override;

    bool
    isCorrelationSaved() const override;

private:

    /// True if the vessel set is valid, false otherwise.
    bool m_valid = false;

    TimingInfo                          m_gaplessTimingInfo; ///< only really useful for global number of times
    std::vector<SpVesselSet_t>          m_vesselSets;
};

}
#endif // ISX_VESSEL_SET_SERIES_H
