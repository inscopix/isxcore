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
    /// \param inTraceFilename       filename for trace output file
    /// \param inImagesFilename      base filename for cell images output file
    /// \param inWriteTimeRelativeTo number of decimals written per cell value
    CellSetExporterParams(
        const std::vector<SpCellSet_t> & inSrcs, 
        const std::string & inTraceFilename,
        const std::string & inImagesFilename,
        WriteTimeRelativeTo inWriteTimeRelativeTo)
    : m_srcs(inSrcs)
    , m_outputTraceFilename(inTraceFilename)
    , m_outputImageFilename(inImagesFilename)
    , m_writeTimeRelativeTo(inWriteTimeRelativeTo)
    {}

    /// default constructor
    /// 
    CellSetExporterParams() :
    CellSetExporterParams(std::vector<SpCellSet_t>{}, std::string(), std::string(), WriteTimeRelativeTo(0))
    {}
    
    /// \return export operation name to display to user
    static
    std::string
    getOpName();

    std::vector<SpCellSet_t> m_srcs;                    ///< input cellsets
    std::string              m_outputTraceFilename;      ///< name of output file for traces
    std::string              m_outputImageFilename;      ///< base filename for output images
    WriteTimeRelativeTo      m_writeTimeRelativeTo;     ///< how to write time stamps in file
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
