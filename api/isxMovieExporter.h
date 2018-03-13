#ifndef ISX_MOVIE_EXPORTER_H
#define ISX_MOVIE_EXPORTER_H

#include "isxCore.h"
#include "isxAsyncTaskHandle.h"

#include <memory>

namespace isx 
{


/// Base class for parameters used for exporting movies
///
struct MovieExporterParams
{
    /// Type of export to perform
    enum class Type
    {
        NWB = 0,        ///< NWB
        TIFF,       ///< TIFF
        MP4         ///< MP4
    };

    /// \return export operation name to display to user
    virtual
    MovieExporterParams::Type
    getType() = 0;

    /// \return export operation name to display to user
    virtual
    std::string
    getOpName() = 0;    

    /// \return     A string representation of these parameters.
    ///
    virtual
    std::string
    toString() const = 0;

    /// \param inFileName the name of the output file for the export operation
    virtual 
    void
    setOutputFileName(const std::string & inFileName) = 0;

    /// \param inWriteDroppedAndCropped the flag specifies whether invalid frames should be written as zero-frames or not
    virtual
    void
    setWirteDroppedAndCroppedParameter(const bool inWriteDroppedAndCropped) = 0;

    /// \param inSources the input movies to be exported
    virtual
    void 
    setSources(const std::vector<SpMovie_t> & inSources) = 0;

    /// \return The input file paths.
    ///
    virtual std::vector<std::string> getInputFilePaths() const = 0;

    /// \return The output file paths.
    ///
    virtual std::vector<std::string> getOutputFilePaths() const = 0;

    /// Set additional information to be saved in the output file
    /// \param inIdentifierBase         identifer base used for creating a unique ID (eg. concatenated lab name, experimentalist, or a hash of
    ///                                 these and/or other values). Creation date/time and library version will be appended.
    /// \param inSessionDescription     One or two sentences describing the experiment and data in the file
    /// \param inComments               Human-readable comments about the TimeSeries.
    /// \param inDescription            Description of TimeSeries
    /// \param inExperimentDescription  General description of the experiment.
    /// \param inExperimenter           Name of person who performed the experiment.
    /// \param inInstitution            Institution(s) where experiment was performed
    /// \param inLab                    Lab where experiment was performed
    /// \param inSessionId              Lab-specific ID for the session.
    virtual 
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
        const std::string & inSessionId = std::string()) = 0;
};

/// Wrapper for movie export params.
/// Needed for triggering export through the AsyncProcessor 
struct MovieExporterParamsWrapper
{
    /// A pointer to the parameters
    std::shared_ptr<MovieExporterParams> m_params;

    /// \return export operation name to display to user
    inline
    std::string getOpName()
    {
        return m_params->getOpName();
    }

    /// \return The string representation of these parameters.
    inline
    std::string toString() const
    {
        return m_params->toString();
    }

    /// \param inFileName the name of the output file for the export operation
    inline
    void
    setOutputFileName(const std::string & inFileName)
    {
        m_params->setOutputFileName(inFileName);
    }

    /// \param inSources the input movies to be exported
    inline
    void 
    setSources(const std::vector<SpMovie_t> & inSources)
    {
        m_params->setSources(inSources);
    }

    /// \param inWriteDroppedAndCropped the flag specifies whether invalid frames should be written as zero-frames or not
    inline
    void
    setWirteDroppedAndCroppedParameter(const bool inWriteDroppedAndCropped)
    {
        m_params->setWirteDroppedAndCroppedParameter(inWriteDroppedAndCropped);
    }

    /// Set additional information to be saved in the output file
    /// \param inIdentifierBase         identifer base used for creating a unique ID (eg. concatenated lab name, experimentalist, or a hash of
    ///                                 these and/or other values). Creation date/time and library version will be appended.
    /// \param inSessionDescription     One or two sentences describing the experiment and data in the file
    /// \param inComments               Human-readable comments about the TimeSeries.
    /// \param inDescription            Description of TimeSeries
    /// \param inExperimentDescription  General description of the experiment.
    /// \param inExperimenter           Name of person who performed the experiment.
    /// \param inInstitution            Institution(s) where experiment was performed
    /// \param inLab                    Lab where experiment was performed
    /// \param inSessionId              Lab-specific ID for the session.
    inline
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
        const std::string & inSessionId = std::string())
    {
        m_params->setAdditionalInfo(
            inIdentifierBase, 
            inSessionDescription, 
            inComments, 
            inDescription, 
            inExperimentDescription, 
            inExperimenter,
            inInstitution,
            inLab, 
            inSessionId);
    }

    /// \return The input file paths.
    ///
    std::vector<std::string> getInputFilePaths() const
    {
        return m_params->getInputFilePaths();
    }

    /// \return The output file paths.
    ///
    std::vector<std::string> getOutputFilePaths() const
    {
        return m_params->getOutputFilePaths();
    }
};

/// Factory method for export parameters
/// \return the parameters struct of the desired type
/// \param inType the desired type of export parameters
MovieExporterParamsWrapper makeMovieExporterParamsWrapper(MovieExporterParams::Type inType);

/// Output parameters for all movie export operations
///
struct MovieExporterOutputParams
{
    MovieExporterParams::Type m_type;   ///< The type of file that was exported.
};

/// Runs MovieExport
/// \param inParams parameters for this Movie export
/// \param inOutputParams a shared pointer for output parameters
/// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
AsyncTaskStatus runMovieExport(MovieExporterParamsWrapper inParams, 
                               std::shared_ptr<MovieExporterOutputParams> inOutputParams = nullptr, 
                               AsyncCheckInCB_t inCheckInCB = [](float) {return false; });

} // namespace isx

#endif // ISX_MOVIE_EXPORTER_H
