#ifndef ISX_CELL_SET_EXPORTER_H
#define ISX_CELL_SET_EXPORTER_H

#include "isxCellSet.h"
#include "isxAsyncTaskHandle.h"

namespace isx {

/// struct that defines CellSetExporter's input data, output data and input parameters
struct CellSetExporterParams
{
    /// Enum to select how to write time stamps in exported file
    enum class WriteTimeRelativeTo
    {
        FIRST_DATA_ITEM,    ///< write time stamps relative to start time of data set
        UNIX_EPOCH          ///< write time stamps relative to unix epoch
    };
    
    /// convenience constructor to fill struct members in one shot
    /// \param inSrcs                input cellsets
    /// \param inFilenameForOutput   filename for output file
    /// \param inWriteTimeRelativeTo number of decimals written per cell value
    CellSetExporterParams(
        const std::vector<SpCellSet_t> & inSrcs, 
        const std::string & inFilenameForOutput,
        WriteTimeRelativeTo inWriteTimeRelativeTo)
    : m_srcs(inSrcs)
    , m_outputFilename(inFilenameForOutput)
    , m_writeTimeRelativeTo(inWriteTimeRelativeTo)
    {}

    /// default constructor
    /// 
    CellSetExporterParams() :
    CellSetExporterParams(std::vector<SpCellSet_t>{}, std::string(), WriteTimeRelativeTo(0))
    {}
    
    /// \return export operation name to display to user
    static
    std::string
    getOpName();

    std::vector<SpCellSet_t> m_srcs;                ///< input cellsets
    std::string              m_outputFilename;      ///< name of output file
    WriteTimeRelativeTo      m_writeTimeRelativeTo; ///< how to write time stamps in file
};

/// Runs CellSetExporter
/// \param inParams parameters for this CellSet export
/// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
AsyncTaskStatus runCellSetExporter(CellSetExporterParams inParams, AsyncCheckInCB_t inCheckInCB);

} // namespace isx

#endif // ISX_CELL_SET_EXPORTER_H
