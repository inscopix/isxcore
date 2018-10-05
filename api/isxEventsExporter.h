#ifndef ISX_EVENTS_EXPORTER_H
#define ISX_EVENTS_EXPORTER_H

#include "isxEvents.h"
#include "isxAsyncTaskHandle.h"
#include "isxExport.h"

namespace isx
{

/// struct that defines EventsExporter's input data, output data and input parameters
struct EventsExporterParams
{
    /// convenience constructor to fill struct members in one shot
    /// \param inSrcs                   input events
    /// \param inFileName               filename for output file
    /// \param inWriteTimeRelativeTo    time reference point
    /// \param inPropertiesFilename  filename for properties output file
    EventsExporterParams(
        const std::vector<SpEvents_t> & inSrcs,
        const std::string & inFileName,
        WriteTimeRelativeTo inWriteTimeRelativeTo,
        const std::string & inPropertiesFilename = "")
    : m_srcs(inSrcs)
    , m_fileName(inFileName)
    , m_writeTimeRelativeTo(inWriteTimeRelativeTo)
    , m_propertiesFilename(inPropertiesFilename)
    {
    }

    /// default constructor
    ///
    EventsExporterParams() :
    EventsExporterParams(std::vector<SpEvents_t>{}, std::string(), WriteTimeRelativeTo(0), "")
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

    std::vector<SpEvents_t> m_srcs;                 ///< input event sets
    std::string             m_fileName;             ///< name of output file
    WriteTimeRelativeTo     m_writeTimeRelativeTo;  ///< how to write time stamps in file
    std::string             m_propertiesFilename;   ///< path of the cell properties file
};

/// There are no output parameters for exporting events.
struct EventsExporterOutputParams
{
};

/// Runs EventsExporter
/// \param inParams parameters for this events export
/// \param outParams a shared pointer for output parameters
/// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
AsyncTaskStatus runEventsExporter(
        EventsExporterParams inParams,
        std::shared_ptr<EventsExporterOutputParams> outParams = nullptr,
        AsyncCheckInCB_t inCheckInCB = [](float){return false;});

} // namespace isx

#endif // ISX_EVENTS_EXPORTER_H
