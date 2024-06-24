#ifndef ISX_GPIO_IMPORTER_H
#define ISX_GPIO_IMPORTER_H

#include "isxAsyncTaskHandle.h"
#include <string>

namespace isx
{
    /// A structure containing import parameters for GPIO data from
    /// nVista 2 (in HDF5 files), nVoke 1 (.raw files) and nVista 3 (.gpio files).
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

        /// The path of the directory where the output files are stored.
        std::string outputDir;

        /// The path of the GPIO file to import.
        std::string fileName;
    };

    /// GPIO data output parameters
    struct GpioDataOutputParams
    {
        std::vector<std::string> filenames; ///< The paths of the output files.
                                            ///< This should only contain one path now, but leaving alone as we are close to release.
    };

    /// Imports a GPIO data file
    /// \param inParams parameters for the import
    /// \param inOutputParams a shared pointer for output parameters
    /// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
    AsyncTaskStatus runGpioDataImporter(GpioDataParams inParams, std::shared_ptr<GpioDataOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB);

} // namespace isx

#endif // ISX_GPIO_IMPORTER_H
