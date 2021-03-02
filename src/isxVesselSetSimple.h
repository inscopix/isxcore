#ifndef ISX_VESSEL_SET_SIMPLE_H
#define ISX_VESSEL_SET_SIMPLE_H

#include "isxVesselSet.h"
#include "isxCoreFwd.h"
#include "isxAsync.h"
#include "isxMutex.h"

#include <fstream>
#include <map>
#include <memory>

namespace isx
{
template <typename T> class IoTaskTracker;

/// Forward declare needed because VesselSetFile is in src.
class VesselSetFile;

/// A class for a containing vessels extracted from a movie
///
/// Currently all reads/writes happen on calling threads and
/// not on the IO thread.
class VesselSetSimple
    : public VesselSet
    , public std::enable_shared_from_this<VesselSetSimple>
{
public:

    /// Empty constructor.
    ///
    /// This creates an invalid vessel set and is for allocation purposes only.
    VesselSetSimple();

    /// Read constructor.
    ///
    /// This opens an existing vessel set file and reads information from its
    /// header.
    ///
    /// \param  inFileName  The name of the vessel set file to read.
    /// \param  enableWrite     Set to true to open in read-write mode
    /// \throw  isx::ExceptionFileIO    If reading the vessel set file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the vessel set file fails.
    VesselSetSimple(const std::string & inFileName, bool enableWrite = false);

    /// Write constructor.
    ///
    /// This opens a new file, writes header information.
    ///
    /// \param  inFileName      The name of the vessel set file to write.
    /// \param  inTimingInfo    The timing information of the vessel set.
    /// \param  inSpacingInfo   The spacing information of the vessel set.
    /// \throw  isx::ExceptionFileIO    If writing the vessel set file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the vessel set data fails.
    VesselSetSimple(const std::string & inFileName,
            const TimingInfo & inTimingInfo,
            const SpacingInfo & inSpacingInfo);

    /// Destructor.
    ///
    ~VesselSetSimple();

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

    void
    writeImageAndLineAndTrace(
        isize_t inIndex,
        const SpImage_t & inProjectionImage,
        const std::pair<PointInPixels_t, PointInPixels_t> & inLineEndpoints,
        SpFTrace_t & inTrace,
        const std::string & inName= std::string()) override;

    VesselSet::VesselStatus
    getVesselStatus(isize_t inIndex) override;

    Color
    getVesselColor(isize_t inIndex) override;

    std::string
    getVesselStatusString(isize_t inIndex) override;

    void
    setVesselStatus(isize_t inIndex, VesselSet::VesselStatus inStatus) override;

    void
    setVesselColor(isize_t inIndex, const Color& inColor) override;

    void
    setVesselColors(const IdColorPairs &inColors) override;

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

private:

    /// True if the vessel set is valid, false otherwise.
    bool m_valid = false;

    /// The vessel set file to read/write from/to.
    std::shared_ptr<VesselSetFile>              m_file;
    std::shared_ptr<IoTaskTracker<FTrace_t>>    m_traceIoTaskTracker;
    std::shared_ptr<IoTaskTracker<Image>>       m_imageIoTaskTracker;
};

}
#endif // ISX_VESSEL_SET_SIMPLE_H
