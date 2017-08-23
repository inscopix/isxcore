#include "isxEventsExporter.h"
#include "isxPathUtils.h"
#include "isxExport.h"
#include "isxSeriesUtils.h"
#include "isxException.h"

#include <fstream>
#include <iomanip>
#include <limits>

namespace isx
{

std::string
EventsExporterParams::getOpName()
{
    return "Export Events";
}

AsyncTaskStatus
runEventsExporter(
        EventsExporterParams inParams,
        std::shared_ptr<EventsExporterOutputParams> outParams,
        AsyncCheckInCB_t inCheckInCB)
{
    bool cancelled = false;
    auto & events = inParams.m_srcs;

    // validate inputs
    if (events.empty())
    {
        inCheckInCB(1.f);
        return AsyncTaskStatus::COMPLETE;
    }

    const SpEvents_t & refEvents = events.front();

    Time baseTime;
    switch (inParams.m_writeTimeRelativeTo)
    {
        case WriteTimeRelativeTo::FIRST_DATA_ITEM:
        {
            baseTime = refEvents->getTimingInfo().getStart();
            break;
        }
        case WriteTimeRelativeTo::UNIX_EPOCH:
        {
            // baseTime is already the unix epoch
            break;
        }
        default:
            ISX_THROW(ExceptionUserInput, "Invalid setting for writeTimeRelativeTo");
    }

    std::ofstream strm(inParams.m_fileName, std::ios::trunc);
    if (!strm.good())
    {
        ISX_THROW(ExceptionFileIO, "Error writing to output file.");
    }

    const std::vector<std::string> cellNames = refEvents->getCellNamesList();
    const size_t numCells = cellNames.size();
    const size_t numSegments = events.size();
    std::vector<std::vector<SpLogicalTrace_t>> traces(numCells);
    for (size_t c = 0; c < numCells; ++c)
    {
        for (size_t s = 0; s < numSegments; ++s)
        {
            traces[c].push_back(events[s]->getLogicalData(cellNames[c]));
        }
    }

    try
    {
        cancelled = writeLogicalTraces(strm, traces, cellNames, "Cell Name", baseTime, inCheckInCB);
    }
    catch (...)
    {
        strm.close();
        std::remove(inParams.m_fileName.c_str());
        throw;
    }
    

    if (cancelled)
    {
        strm.flush();
        std::remove(inParams.m_fileName.c_str());
        return AsyncTaskStatus::CANCELLED;
    }

    inCheckInCB(1.f);
    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
