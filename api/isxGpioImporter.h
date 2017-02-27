#ifndef GPIO_IMPORTER_H
#define GPIO_IMPORTER_H


#include "isxAsyncTaskHandle.h"
#include <string>

namespace isx
{
    /// A structure containing import parameters for GPIO data
    ///
    struct GpioDataParams
    {
        /// Ctor
        GpioDataParams(
            const std::string & inOutputDir,
            const std::string & inFileName)
        : outputDir(inOutputDir)
        , fileName(inFileName)
        {

        }

        /// \return export operation name to display to user
        static
        std::string
        getOpName();

        std::string outputDir;                      ///< The output directory for the output files
        std::string fileName;                       ///< The filename of the .raw file to process
    };

    /// GPIO data output parameters
    struct GpioDataOutputParams
    {
        std::vector<std::string> filenames;         ///< Output filenames
    };

    /// Imports a GPIO data file
    /// \param inParams parameters for the import
    /// \param inOutputParams a shared pointer for output parameters
    /// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
    AsyncTaskStatus runGpioDataImporter(GpioDataParams inParams, std::shared_ptr<GpioDataOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB);
}

#endif // GPIO_IMPORTER_H