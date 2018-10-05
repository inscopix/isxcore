#ifndef ISX_CELL_SET_EXPORTER_H
#define ISX_CELL_SET_EXPORTER_H

#include "isxCellSet.h"
#include "isxAsyncTaskHandle.h"
#include "isxExport.h"

namespace isx {

/// struct that defines CellSetExporter's input data, output data and input parameters
struct CellSetExporterParams
{
    /// convenience constructor to fill struct members in one shot
    /// \param inSrcs                input cellsets
    /// \param inTraceFilename       filename for trace output file
    /// \param inImagesFilename      base filename for cell images output file
    /// \param inWriteTimeRelativeTo time reference point
    /// \param inWritePngImage       true if PNG cell map image should be written,
    ///                              false otherwise
    /// \param inPropertiesFilename  filename for properties output file
    /// \param inAutoOutputProps     if true, automatically output a properties file
    CellSetExporterParams(
        const std::vector<SpCellSet_t> & inSrcs, 
        const std::string & inTraceFilename,
        const std::string & inImagesFilename,
        WriteTimeRelativeTo inWriteTimeRelativeTo,
        const bool inWritePngImage = true,
        const std::string & inPropertiesFilename = "",
        const bool inAutoOutputProps = false)
    : m_srcs(inSrcs)
    , m_outputTraceFilename(inTraceFilename)
    , m_outputImageFilename(inImagesFilename)
    , m_writeTimeRelativeTo(inWriteTimeRelativeTo)
    , m_writePngImage(inWritePngImage)
    , m_propertiesFilename(inPropertiesFilename)
    , m_autoOutputProps(inAutoOutputProps)
    {}

    /// default constructor
    /// 
    CellSetExporterParams() :
    CellSetExporterParams(std::vector<SpCellSet_t>{}, std::string(), std::string(), WriteTimeRelativeTo(0), true, "", false)
    {}
    
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

    std::vector<SpCellSet_t> m_srcs;                    ///< input cellsets
    std::string              m_outputTraceFilename;     ///< name of output file for traces
    std::string              m_outputImageFilename;     ///< base filename for output images
    WriteTimeRelativeTo      m_writeTimeRelativeTo;     ///< how to write time stamps in file
    bool                     m_writePngImage;           ///< true if PNG cell map should be written, false otherwise
    std::string              m_propertiesFilename;      ///< path of the cell properties file
    bool                     m_autoOutputProps;         ///< If true and the properties file name is empty,
                                                        ///< automatically output a properties file
                                                        ///< that shares the same base name as the trace file
                                                        ///< but with "-properties" appended before the extension.
                                                        ///< Otherwise, do not output a properties file automatically.
};

/// CellSet exporter output parameters 
struct CellSetExporterOutputParams
{
    // There are no output parameters for exporting cellsets.
};

/// Runs CellSetExporter
/// \param inParams parameters for this CellSet export
/// \param inOutputParams a shared pointer for output parameters
/// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
AsyncTaskStatus runCellSetExporter(CellSetExporterParams inParams, std::shared_ptr<CellSetExporterOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB);

} // namespace isx

#endif // ISX_CELL_SET_EXPORTER_H
