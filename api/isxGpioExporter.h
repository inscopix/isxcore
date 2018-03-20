#ifndef ISX_GPIO_EXPORTER_H
#define ISX_GPIO_EXPORTER_H

#include "isxGpio.h"
#include "isxAsyncTaskHandle.h"
#include "isxExport.h"

namespace isx
{

/// struct that defines GpioExporter's input data, output data and input parameters
struct GpioExporterParams
{
    /// convenience constructor to fill struct members in one shot
    /// \param inSrcs                   input GPIOs
    /// \param inFileName               filename for output file
    /// \param inWriteTimeRelativeTo    time reference point
    GpioExporterParams(
        const std::vector<SpGpio_t> & inSrcs,
        const std::string & inFileName,
        WriteTimeRelativeTo inWriteTimeRelativeTo)
    : m_srcs(inSrcs)
    , m_fileName(inFileName)
    , m_writeTimeRelativeTo(inWriteTimeRelativeTo)
    {
    }

    /// default constructor
    ///
    GpioExporterParams() :
    GpioExporterParams(std::vector<SpGpio_t>{}, std::string(), WriteTimeRelativeTo(0))
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

    std::vector<SpGpio_t>   m_srcs;                 ///< input GPIO sets
    std::string             m_fileName;             ///< name of output file
    WriteTimeRelativeTo     m_writeTimeRelativeTo;  ///< how to write time stamps in file
};

/// There are no output parameters for exporting GPIO sets.
struct GpioExporterOutputParams
{
};

/// Runs GpioExporter
/// \param inParams parameters for this GPIO export
/// \param inOutputParams a shared pointer for output parameters
/// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
AsyncTaskStatus runGpioExporter(
        GpioExporterParams inParams,
        std::shared_ptr<GpioExporterOutputParams> inOutputParams = nullptr,
        AsyncCheckInCB_t inCheckInCB = [](float){return false;});

} // namespace isx

#endif // ISX_GPIO_EXPORTER_H
