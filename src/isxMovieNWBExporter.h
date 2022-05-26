#ifndef ISX_MOVIE_NWB_EXPORTER_H
#define ISX_MOVIE_NWB_EXPORTER_H

#include "isxCore.h"
#include "isxAsyncTaskHandle.h"
#include "isxMovieExporter.h"

#include <memory>

namespace isx 
{

/// struct that defines MovieExporter's input data, output data and input parameters
struct MovieNWBExporterParams : MovieExporterParams
{
    /// convenience constructor to fill struct members in one shot
    /// \param inSrcs                   input movies
    /// \param inNwbFilename            filename for Nwb output file
    /// \param inIdentifier             unique identifer, required by NWB (eg. concatenated lab name, file
    ///                                 creation date/time and experimentalist, or a hash of
    ///                                 these and/or other values)
    /// \param inSessionDescription     One or two sentences describing the experiment and data in the file,
    ///                                 required by NWB
    /// \param inComments               Human-readable comments about the TimeSeries.
    /// \param inDescription            Description of TimeSeries
    /// \param inExperimentDescription  General description of the experiment.
    /// \param inExperimenter           Name of person who performed the experiment.
    /// \param inInstitution            Institution(s) where experiment was performed
    /// \param inLab                    Lab where experiment was performed
    /// \param inSessionId              Lab-specific ID for the session.
    MovieNWBExporterParams(
        const std::vector<SpMovie_t> & inSrcs,
        const std::string & inNwbFilename,
        const std::string & inIdentifier,
        const std::string & inSessionDescription,
        const std::string & inComments = std::string(),
        const std::string & inDescription = std::string(),
        const std::string & inExperimentDescription = std::string(),
        const std::string & inExperimenter = std::string(),
        const std::string & inInstitution = std::string(),
        const std::string & inLab = std::string(),
        const std::string & inSessionId = std::string())
    : m_srcs(inSrcs)
    , m_identifier(inIdentifier)
    , m_sessionDescription(inSessionDescription)
    , m_comments(inComments)
    , m_description(inDescription)
    , m_experimentDescription(inExperimentDescription)
    , m_experimenter(inExperimenter)
    , m_institution(inInstitution)
    , m_lab(inLab)
    , m_sessionId(inSessionId)
    , m_filename(inNwbFilename)
    {}

    /// default constructor
    /// 
    MovieNWBExporterParams(){}

    std::string
    getOpName() override;

    std::string
    toString() const override;

    MovieExporterParams::Type
    getType() override;

    std::vector<std::string> getInputFilePaths() const override;

    std::vector<std::string> getOutputFilePaths() const override;

    void
    setOutputFileName(const std::string & inFileName) override;

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

    void
    setWriteDroppedAndCroppedParameter(const bool inWriteDroppedAndCropped) override;

    void
    setBitRateFraction(const double inBitRateFraction) override;

    /// \return id string
    ///
    static
    std::string
    makeIdString(const std::string & inBase);

    static const char *     sNwbVersion;                    ///< NWB spec version string

    std::vector<SpMovie_t>  m_srcs;                         ///< input movies
    std::string             m_identifier;                   ///< unique identifier
    std::string             m_sessionDescription;           ///< session description
    
    std::string             m_comments;                     ///< Human-readable comments about the TimeSeries.
    std::string             m_description;                  ///< Description of TimeSeries
    
    std::string             m_experimentDescription;        ///< General description of the experiment.
    std::string             m_experimenter;                 ///< Name of person who performed the experiment.
    std::string             m_institution;                  ///< Institution(s) where experiment was performed
    std::string             m_lab;                          ///< Lab where experiment was performed
    std::string             m_sessionId;                    ///< Lab-specific ID for the session.

    std::string             m_filename;                     ///< name of output file
};

/// Movie exporter output parameters 
struct MovieNWBExporterOutputParams : MovieExporterOutputParams
{
    // There are no output parameters for exporting movies.
};

/// Runs MovieExporter
/// \param inParams parameters for this Movie export
/// \param inOutputParams a shared pointer for output parameters
/// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
/// \param inProgressAllocation amount of progress to allocate for this operation
/// \param inProgressStart amount of progress to start with for this operation
AsyncTaskStatus runMovieNWBExporter(MovieNWBExporterParams inParams, 
                                    std::shared_ptr<MovieNWBExporterOutputParams> inOutputParams = nullptr, 
                                    AsyncCheckInCB_t inCheckInCB = [](float) {return false; },
                                    const float inProgressAllocation = 1.0f,
                                    const float inProgressStart = 0.0f);

} // namespace isx

#endif // ISX_MOVIE_NWB_EXPORTER_H
