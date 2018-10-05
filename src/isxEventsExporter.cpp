#include "isxEventsExporter.h"
#include "isxPathUtils.h"
#include "isxExport.h"
#include "isxSeriesUtils.h"
#include "isxException.h"

#include <fstream>
#include <iomanip>
#include <limits>

#include "json.hpp"

namespace isx
{

std::string
EventsExporterParams::getOpName()
{
    return "Export Events";
}

std::string
EventsExporterParams::toString() const
{
    using json = nlohmann::json;
    json j;
    j["writeTimeRelativeTo"] = int(m_writeTimeRelativeTo);
    return j.dump(4);
}

std::vector<std::string>
EventsExporterParams::getInputFilePaths() const
{
    std::vector<std::string> inputFilePaths;
    for (const auto & s : m_srcs)
    {
        inputFilePaths.push_back(s->getFileName());
    }
    return inputFilePaths;
}

std::vector<std::string>
EventsExporterParams::getOutputFilePaths() const
{
    return {m_fileName};
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

    /// Event properties to CSV.
    if (inParams.m_autoOutputProps && inParams.m_propertiesFilename.empty())
    {
        inParams.m_propertiesFilename = makeOutputFilePath(inParams.m_fileName, "-props.csv");
    }

    if (!inParams.m_propertiesFilename.empty())
    {
        std::vector<std::string> eventsFilePaths;
        for (const auto & e : events)
        {
            eventsFilePaths.push_back(e->getFileName());
        }
        const isx::SpEvents_t eventsSeries = isx::readEventsSeries(eventsFilePaths);
        const bool hasMetrics = eventsSeries->hasMetrics();

        std::ofstream csv(inParams.m_propertiesFilename);
        csv << "Name";
        if (hasMetrics)
        {
            csv << ",EventRate(Hz),SNR";
        }
        csv << std::endl;

        const std::vector<std::string> cellNames = eventsSeries->getCellNamesList();
        ISX_ASSERT(cellNames.size() == numCells);
        for (size_t c = 0; c < numCells; ++c)
        {
            csv << cellNames.at(c);
            if (hasMetrics)
            {
                const isx::SpTraceMetrics_t traceMetrics = eventsSeries->getTraceMetrics(c);
                csv << "," << traceMetrics->m_eventRate
                    << "," << traceMetrics->m_snr;
            }
            csv << std::endl;
        }
    }

    if (cancelled)
    {
        strm.flush();
        std::remove(inParams.m_fileName.c_str());
        if (!inParams.m_propertiesFilename.empty())
        {
            std::remove(inParams.m_propertiesFilename.c_str());
        }
        return AsyncTaskStatus::CANCELLED;
    }

    inCheckInCB(1.f);
    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
