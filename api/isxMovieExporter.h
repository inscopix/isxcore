#ifndef ISX_MOVIE_EXPORTER_H
#define ISX_MOVIE_EXPORTER_H

#include "isxCore.h"
#include "isxAsyncTaskHandle.h"
#include "isxException.h"
#include "isxExport.h"

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

    /// Format to export frame rate metadata
    /// Applicable for MP4 export
    enum class FrameRateFormat
    {
        FLOATING_PRECISE = 0,
        INTEGER_ROUNDED
    };

    /// \return The type of export based on a string representing the type
    static isx::MovieExporterParams::Type convertStringToType(const std::string inStr)
    {
        if (inStr == "NWB")
        {
            return isx::MovieExporterParams::Type::NWB;
        }
        else if (inStr == "TIFF")
        {
            return isx::MovieExporterParams::Type::TIFF;
        }
        else if (inStr == "MP4")
        {
            return isx::MovieExporterParams::Type::MP4;
        }
        else
        {
            ISX_THROW(isx::Exception, "Unsupported movie export type: ", inStr);
        }
    }

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
    setWriteDroppedAndCroppedParameter(const bool inWriteDroppedAndCropped) = 0;

    /// \param  inBitRateFraction    The compression quality in (0, 1]. This will only be used with
    ///                                 exporters that allow lossy compression.
    virtual
    void
    setBitRateFraction(const double inBitRateFraction) = 0;

    /// \param  inFrameRateFormat    The format to export the frame rate
    virtual
    void
    setFrameRateFormat(const FrameRateFormat inFrameRateFormat) = 0;

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


    /// The default bit-rate fraction for lossy exporters.
    constexpr static double s_defaultBitRateFraction = 0.1;
};

/// struct that defines MovieTimestampExporterParams's input data, output data and input parameters
struct MovieTimestampExporterParams
{
    /// convenience constructor to fill struct members in one shot
    /// \param inOutputFilename       filename for csv output file
    /// \param inFormat               format of timestamps
    MovieTimestampExporterParams(
        const std::string & inOutputFilename,
        const WriteTimeRelativeTo inFormat)
    : m_outputFilename(inOutputFilename)
    , m_format(inFormat)
    {}

    /// \param inSources the input movies to be exported
    void 
    setSources(const std::vector<SpMovie_t> & inSources);

    /// \return     A string representation of these parameters.
    ///
    std::string
    toString() const;

    /// \return The input file paths.
    ///
    std::vector<std::string> getInputFilePaths() const;

    /// \return The output file paths.
    ///
    std::vector<std::string> getOutputFilePaths() const;

    std::vector<SpMovie_t> m_srcs;                          ///< input movies
    std::string              m_outputFilename;              ///< name of csv output file for timestamps
    WriteTimeRelativeTo              m_format;       ///< export format for timestamps
};

/// Wrapper for movie export params.
/// Needed for triggering export through the AsyncProcessor 
struct MovieExporterParamsWrapper
{
    /// A pointer to the mpvie frame export parameters
    std::shared_ptr<MovieExporterParams> m_params;

    /// A pointer to the movie timestamp export parameters
    std::shared_ptr<MovieTimestampExporterParams> m_timestampParams;

    /// \return export operation name to display to user
    std::string getOpName();

    /// \return The string representation of these parameters.
    std::string toString() const;

    /// \param inFileName the name of the output file for the export operation
    void
    setOutputFileName(const std::string & inFileName);

    /// \param inSources the input movies to be exported
    void 
    setSources(const std::vector<SpMovie_t> & inSources);

    /// \param inWriteDroppedAndCropped the flag specifies whether invalid frames should be written as zero-frames or not
    void
    setWriteDroppedAndCroppedParameter(const bool inWriteDroppedAndCropped);

    /// \param  inBitRateFraction    The compression quality in (0, 1]. This will only be used with
    ///                                 exporters that allow lossy compression.
    void
    setBitRateFraction(const double inBitRateFraction);

    /// \param  inFrameRateFormat    The format to export the frame rate
    void
    setFrameRateFormat(const MovieExporterParams::FrameRateFormat inFrameRateFormat);

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
        const std::string & inSessionId = std::string());

    /// \return The input file paths.
    ///
    std::vector<std::string> getInputFilePaths() const;

    /// \return The output file paths.
    ///
    std::vector<std::string> getOutputFilePaths() const;
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


/// Runs MovieTimestampExport
/// \param inParams parameters for this Movie timestamp export
/// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
/// \param inProgressAllocation amount of progress to allocate for this operation
/// \param inProgressStart amount of progress to start with for this operation 
AsyncTaskStatus
runMovieTimestampExport(const MovieTimestampExporterParams inParams,
                        AsyncCheckInCB_t inCheckInCB = [](float) {return false; },
                        const float inProgressAllocation = 1.0f,
                        const float inProgressStart = 0.0f);

/// Runs MovieExport
/// \param inParams parameters for this Movie export
/// \param inOutputParams a shared pointer for output parameters
/// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
AsyncTaskStatus runMovieExport(MovieExporterParamsWrapper inParams, 
                               std::shared_ptr<MovieExporterOutputParams> inOutputParams = nullptr, 
                               AsyncCheckInCB_t inCheckInCB = [](float) {return false; });

} // namespace isx

#endif // ISX_MOVIE_EXPORTER_H
