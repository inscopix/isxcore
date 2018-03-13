#ifndef ISX_MOVIE_TIFF_EXPORTER_H
#define ISX_MOVIE_TIFF_EXPORTER_H

#include "isxCore.h"
#include "isxAsyncTaskHandle.h"
#include "isxMovieExporter.h"

namespace isx 
{

/// struct that defines MovieExporter's input data, output data and input parameters
struct MovieTiffExporterParams : MovieExporterParams
{
    /// convenience constructor to fill struct members in one shot
    /// \param inSrcs                   input movies
    /// \param inTiffFilename           filename for Tiff output file
    /// \param inWriteInvalidFrames     true if dropped/cropped frames should be written as zeros frames,
    ///                                 false if they should be skipped
    /// \param inNumFramesInMovie       frames number in one splitted part of movie.
    MovieTiffExporterParams(
        const std::vector<SpMovie_t> & inSrcs,
        const std::string & inTiffFilename,
        const bool inWriteInvalidFrames = false,
        const isize_t inNumFramesInMovie = s_defaultNumFramesInMovie)
    : m_srcs(inSrcs)
    , m_filename(inTiffFilename)
    , m_writeInvalidFrames(inWriteInvalidFrames)
    , m_numFramesInMovie(inNumFramesInMovie)
    {}

    /// default constructor
    /// 
    MovieTiffExporterParams(){}

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

    std::vector<SpMovie_t>  m_srcs;                                         ///< input movies
    std::string             m_filename;                                     ///< name of output file
    bool                    m_writeInvalidFrames;                           ///< substitute zero-frames with dropped and cropped
    isize_t                 m_numFramesInMovie = s_defaultNumFramesInMovie; ///< number of frames in one movie
    const static isize_t    s_defaultNumFramesInMovie = 65535;              ///< default number of frames in one movie
};

/// Movie exporter output parameters 
struct MovieTiffExporterOutputParams : MovieExporterOutputParams
{
    // There are no output parameters for exporting movies.
};

/// Runs MovieTiffExporter
/// \param inParams parameters for this Movie Tiff export
/// \param inOutputParams a shared pointer for output parameters
/// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
AsyncTaskStatus runMovieTiffExporter(MovieTiffExporterParams inParams, 
                                     std::shared_ptr<MovieTiffExporterOutputParams> inOutputParams = nullptr, 
                                     AsyncCheckInCB_t inCheckInCB = [](float) {return false; });

} // namespace isx

#endif // ISX_MOVIE_TIFF_EXPORTER_H
