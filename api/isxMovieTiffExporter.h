#ifndef ISX_MOVIE_TIFF_EXPORTER_H
#define ISX_MOVIE_TIFF_EXPORTER_H

#include "isxCore.h"
#include "isxAsyncTaskHandle.h"

namespace isx 
{

/// struct that defines MovieExporter's input data, output data and input parameters
struct MovieTiffExporterParams
{
    /// convenience constructor to fill struct members in one shot
    /// \param inSrcs                   input movies
    /// \param inTiffFilename           filename for Tiff output file
    /// \param inNumFramesInMovie       frames number in one splitted part of movie.
    MovieTiffExporterParams(
        const std::vector<SpMovie_t> & inSrcs,
        const std::string & inTiffFilename,
        const bool inWriteInvalidFrames,
        const isize_t inNumFramesInMovie = s_defaultNumFramesInMovie)
    : m_srcs(inSrcs)
    , m_tiffFilename(inTiffFilename)
    , m_writeInvalidFrames(inWriteInvalidFrames)
    , m_numFramesInMovie(inNumFramesInMovie)
    {}

    /// default constructor
    /// 
    MovieTiffExporterParams(){}
    
    /// \return export operation name to display to user
    static
    std::string
    getOpName();

    std::vector<SpMovie_t>  m_srcs;                                         ///< input movies
    std::string             m_tiffFilename;                                 ///< name of output tiff file
    bool                    m_writeInvalidFrames;                           ///< substitute zero-frames with dropped and cropped
    isize_t                 m_numFramesInMovie = s_defaultNumFramesInMovie; ///< number of frames in one movie
    const static isize_t    s_defaultNumFramesInMovie = 65535;              ///< default number of frames in one movie
};

/// Movie exporter output parameters 
struct MovieTiffExporterOutputParams
{
    // There are no output parameters for exporting movies.
};

/// Runs MovieTiffExporter
/// \param inParams parameters for this Movie Tiff export
/// \param inOutputParams a shared pointer for output parameters
/// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
AsyncTaskStatus runMovieTiffExporter(MovieTiffExporterParams inParams, std::shared_ptr<MovieTiffExporterOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB);

} // namespace isx

#endif // ISX_MOVIE_TIFF_EXPORTER_H
