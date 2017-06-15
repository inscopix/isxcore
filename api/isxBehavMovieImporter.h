#ifndef ISX_BEHAV_MOVIE_IMPORTER_H
#define ISX_BEHAV_MOVIE_IMPORTER_H

#include "isxAsyncTaskHandle.h"
#include "isxDataSet.h"

namespace isx
{
    /// A structure containing parameters for importing behavioral movies
    ///
    struct BehavMovieImportParams
    {
        /// Ctor
        BehavMovieImportParams(
            const std::string & inFileName)
        : fileName(inFileName)
        {

        }

        /// \return export operation name to display to user
        static
        std::string
        getOpName();

        std::string fileName;                           ///< The filename of the behavioral movie to import
    };

    /// Behavioral movie inmport output parameters
    struct BehavMovieImportOutputParams
    {
        DataSet::Properties     m_dataSetProperties;    ///< DataSet properties of output dataset, will set numFrames, gopSize
    };

    /// Imports a behavioral movie
    /// \param inParams parameters for the import
    /// \param inOutputParams a shared pointer for output parameters
    /// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
    AsyncTaskStatus runBehavMovieImporter(BehavMovieImportParams inParams, std::shared_ptr<BehavMovieImportOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB);
}

#endif // ISX_BEHAV_MOVIE_IMPORTER_H
