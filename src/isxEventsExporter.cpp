#include "isxEventsExporter.h"
#include "isxPathUtils.h"
#include "isxExport.h"
#include "isxSeriesUtils.h"
#include "isxException.h"

#include <fstream>
#include <iomanip>
#include <limits>

#include "json.hpp"

namespace
{

bool
writeEventsProperties(
        const std::vector<isx::SpEvents_t> & inEvents,
        const std::string & inFilePath,
        isx::AsyncCheckInCB_t inCheckInCB)
{
    std::vector<std::string> eventsFilePaths;
    for (const auto & e : inEvents)
    {
        eventsFilePaths.push_back(e->getFileName());
    }
    const isx::SpEvents_t eventsSeries = isx::readEventsSeries(eventsFilePaths);
    const std::vector<std::string> cellNames = eventsSeries->getCellNamesList();
    const size_t numCells = cellNames.size();
    const bool hasMetrics = eventsSeries->hasMetrics();

    std::ofstream csv(inFilePath);
    csv << "Name";
    if (hasMetrics)
    {
        csv << ",SNR,EventRate(Hz)";
    }
    csv << std::endl;

    for (size_t c = 0; c < numCells; ++c)
    {
        csv << cellNames.at(c);
        if (hasMetrics)
        {
            const isx::SpTraceMetrics_t traceMetrics = eventsSeries->getTraceMetrics(c);
            csv << "," << traceMetrics->m_snr
                << "," << traceMetrics->m_eventRate;
        }
        csv << std::endl;

        if (inCheckInCB && inCheckInCB(float(c) / float(numCells)))
        {
            return true;
        }
    }

    return false;
}

} // namespace

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

    if (inParams.m_autoOutputProps && inParams.m_propertiesFilename.empty())
    {
        inParams.m_propertiesFilename = makeOutputFilePath(inParams.m_fileName, "-props.csv");
    }

    std::ofstream strm(inParams.m_fileName, std::ios::trunc);
    if (!strm.good())
    {
        ISX_THROW(ExceptionFileIO, "Error writing to output file.");
    }

    const bool outputProps = !inParams.m_propertiesFilename.empty();

    // Event traces to CSV.
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

    AsyncCheckInCB_t tracesCheckInCB = rescaleCheckInCB(inCheckInCB, 0.f, outputProps ? 0.8f : 1.f);
    std::vector<std::string> filesToCleanUp = {inParams.m_fileName};
    try
    {
        cancelled = writeLogicalTraces(strm, traces, cellNames, "Cell Name", baseTime, tracesCheckInCB);
    }
    catch (...)
    {
        strm.close();
        removeFiles(filesToCleanUp);
        throw;
    }

    /// Event properties to CSV.
    if (outputProps)
    {
        AsyncCheckInCB_t propsCheckInCB = rescaleCheckInCB(inCheckInCB, 0.8f, 0.2f);
        filesToCleanUp.push_back(inParams.m_propertiesFilename);
        try
        {
            cancelled = writeEventsProperties(events, inParams.m_propertiesFilename, propsCheckInCB);
        }
        catch (...)
        {
            removeFiles(filesToCleanUp);
            throw;
        }
    }

    if (cancelled)
    {
        strm.flush();
        removeFiles(filesToCleanUp);
        return AsyncTaskStatus::CANCELLED;
    }

    inCheckInCB(1.f);
    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
