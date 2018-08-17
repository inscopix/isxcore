#ifndef ISX_MOVIE_COMPRESSED_AVI_EXPORTER_H
#define ISX_MOVIE_COMPRESSED_AVI_EXPORTER_H

#include "isxCore.h"
#include "isxAsyncTaskHandle.h"
#include "isxMovieExporter.h"

namespace isx 
{

/// struct that defines MovieExporter's input data, output data and input parameters
struct MovieCompressedAviExporterParams : MovieExporterParams
{
    /// convenience constructor to fill struct members in one shot
    /// \param inSrcs                      input movies
    /// \param inCompressedAviFilename     filename for CompressedAvi output file
    /// \param inBitRate                   bit-rate in bps
    MovieCompressedAviExporterParams(
        const std::vector<SpMovie_t> & inSrcs,
        const std::string & inCompressedAviFilename,
        const isize_t inBitRate)
    : m_srcs(inSrcs)
    , m_filename(inCompressedAviFilename)
    , m_bitRate(inBitRate)
    {}

    /// convenience constructor to fill struct members in one shot
    /// \param inSrcs                      input movies
    /// \param inCompressedAviFilename     filename for CompressedAvi output file
    /// \param inBitRateFraction           bit-rate as fraction of uncompressed bit-rate
    MovieCompressedAviExporterParams(
        const std::vector<SpMovie_t> & inSrcs,
        const std::string & inCompressedAviFilename,
        const double inBitRateFraction)
        : m_srcs(inSrcs)
        , m_filename(inCompressedAviFilename)
        , m_bitRateFraction(inBitRateFraction)
    { }

    /// default constructor
    /// 
    MovieCompressedAviExporterParams()
    {
        setBitRateFraction(.25);
    }

    std::string
    getOpName() override;

    std::string
    toString() const override;

    MovieExporterParams::Type
    getType() override;

    std::vector<std::string> getInputFilePaths() const override;

    std::vector<std::string> getOutputFilePaths() const override;

    isize_t getBitRate() const;

    double getBitRateFraction() const;

    void
    setOutputFileName(const std::string & inFileName) override;

    void
    setWirteDroppedAndCroppedParameter(const bool inWriteDroppedAndCropped) override;

    void 
    setSources(const std::vector<SpMovie_t> & inSources) override;

    void
    setBitRate(const isize_t inBitRate);

    void
    setBitRateFraction(const double inBitRateFraction);

    void
    updateBitRateBasedOnFraction();

    void
    setAdditionalInfo(
        const std::string & inIdentifierBase,
        const std::string & inSessionDescription,
        const std::string & inComments = std::string(),
        const std::string & inDescription = std::string(),
        const std::string & inExperimentDescription = std::string(),
        const std::string & inExperimenter = std::string(),
        const std::string & inInstitution = std::string(),
        const std::string & inLab = std::string(),
        const std::string & inSessionId = std::string()) override;

    std::vector<SpMovie_t>  m_srcs;                         ///< input movies
    std::string             m_filename;                     ///< name of output file
    isize_t                 m_bitRate;                      ///< bitrate in bps
    double                  m_bitRateFraction;              ///< bitrate as fraction of theoretical uncompressed
};

/// Movie exporter output parameters 
struct MovieCompressedAviExporterOutputParams : MovieExporterOutputParams
{
    // There are no output parameters for exporting movies.
};

/// Runs MovieCompressedAviExporter
/// \param inParams parameters for this Movie CompressedAvi export
/// \param inOutputParams a shared pointer for output parameters
/// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
AsyncTaskStatus runMovieCompressedAviExporter(MovieCompressedAviExporterParams inParams, 
                                              std::shared_ptr<MovieCompressedAviExporterOutputParams> inOutputParams = nullptr, 
                                              AsyncCheckInCB_t inCheckInCB = [](float) {return false; });

} // namespace isx

#endif // ISX_MOVIE_COMPRESSED_AVI_EXPORTER_H
