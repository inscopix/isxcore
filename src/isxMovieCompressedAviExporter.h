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
    /// \param inSrcs                   input movies
    /// \param inCompressedAviFilename           filename for CompressedAvi output file
    MovieCompressedAviExporterParams(
        const std::vector<SpMovie_t> & inSrcs,
        const std::string & inCompressedAviFilename)
    : m_srcs(inSrcs)
    , m_filename(inCompressedAviFilename)
    {}

    /// default constructor
    /// 
    MovieCompressedAviExporterParams(){}

    std::string
    getOpName() override;

    std::string
    toString() const override;

    MovieExporterParams::Type
    getType() override;

    void
    setOutputFileName(const std::string & inFileName) override;

    void
    setWirteDroppedAndCroppedParameter(const bool inWriteDroppedAndCropped) override;

    void 
    setSources(const std::vector<SpMovie_t> & inSources) override;

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
