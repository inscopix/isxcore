#ifndef ISX_MOVIE_EXPORTER_H
#define ISX_MOVIE_EXPORTER_H

#include "isxCore.h"
#include "isxAsyncTaskHandle.h"

namespace isx 
{

/// struct that defines MovieExporter's input data, output data and input parameters
struct MovieExporterParams
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
    MovieExporterParams(
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
    , m_nwbFilename(inNwbFilename)
    , m_identifier(inIdentifier)
    , m_sessionDescription(inSessionDescription)
    , m_comments(inComments)
    , m_description(inDescription)
    , m_experimentDescription(inExperimentDescription)
    , m_experimenter(inExperimenter)
    , m_institution(inInstitution)
    , m_lab(inLab)
    , m_sessionId(inSessionId)
    {}

    /// default constructor
    /// 
    MovieExporterParams(){}
    
    /// \return export operation name to display to user
    static
    std::string
    getOpName();

    /// \return id string
    ///
    static
    std::string
    makeIdString(const std::string & inBase);

    static const char *     sNwbVersion;                    ///< NWB spec version string

    std::vector<SpMovie_t>  m_srcs;                         ///< input movies
    std::string             m_nwbFilename;                  ///< name of output nwb file
    std::string             m_tiffFilename;                 ///< name of output tiff file
    std::string             m_identifier;                   ///< unique identifier
    std::string             m_sessionDescription;           ///< session description
    
    std::string             m_comments;                     ///< Human-readable comments about the TimeSeries.
    std::string             m_description;                  ///< Description of TimeSeries
    
    std::string             m_experimentDescription;        ///< General description of the experiment.
    std::string             m_experimenter;                 ///< Name of person who performed the experiment.
    std::string             m_institution;                  ///< Institution(s) where experiment was performed
    std::string             m_lab;                          ///< Lab where experiment was performed
    std::string             m_sessionId;                    ///< Lab-specific ID for the session.
};

/// Movie exporter output parameters 
struct MovieExporterOutputParams
{
    // There are no output parameters for exporting movies.
};

/// Runs MovieExporter
/// \param inParams parameters for this Movie export
/// \param inOutputParams a shared pointer for output parameters
/// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
AsyncTaskStatus runMovieExporter(MovieExporterParams inParams, std::shared_ptr<MovieExporterOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB);

} // namespace isx

#endif // ISX_MOVIE_EXPORTER_H
