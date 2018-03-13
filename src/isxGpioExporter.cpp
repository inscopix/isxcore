#include "isxGpioExporter.h"
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
GpioExporterParams::getOpName()
{
    return "Export GPIO";
}

std::string
GpioExporterParams::toString() const
{
    using json = nlohmann::json;
    json j;
    j["writeTimeRelativeTo"] = int(m_writeTimeRelativeTo);
    return j.dump(4);
}

std::vector<std::string>
GpioExporterParams::getInputFilePaths() const
{
    std::vector<std::string> inputFilePaths;
    for (const auto & s : m_srcs)
    {
        inputFilePaths.push_back(s->getFileName());
    }
    return inputFilePaths;
}

std::vector<std::string>
GpioExporterParams::getOutputFilePaths() const
{
    return {m_fileName};
}

AsyncTaskStatus
runGpioExporter(
        GpioExporterParams inParams,
        std::shared_ptr<GpioExporterOutputParams> inOutputParams,
        AsyncCheckInCB_t inCheckInCB)
{
    bool cancelled = false;
    auto & gpios = inParams.m_srcs;

    // validate inputs
    if (gpios.empty())
    {
        inCheckInCB(1.f);
        return AsyncTaskStatus::COMPLETE;
    }

    const size_t numSegments = gpios.size();
    const SpGpio_t & refGpio = gpios.front();
    const std::vector<std::string> channelNames = refGpio->getChannelList();

    Time baseTime;
    switch (inParams.m_writeTimeRelativeTo)
    {
        case WriteTimeRelativeTo::FIRST_DATA_ITEM:
        {
            baseTime = refGpio->getTimingInfo().getStart();
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
    
    
    const size_t numChannels = channelNames.size();
    std::vector<std::vector<SpLogicalTrace_t>> traces(numChannels);  

    for (size_t s = 0; s < numSegments; ++s)
    {
        std::vector<SpLogicalTrace_t> logicalTraces;
        std::vector<SpFTrace_t> continuousTraces;
        gpios[s]->getAllTraces(continuousTraces, logicalTraces);

        for (size_t c = 0; c < numChannels; ++c)
        {
            traces[c].push_back(logicalTraces[c]);
        }
    }

    try
    {
        cancelled = writeLogicalTraces(strm, traces, channelNames, "Channel Name", baseTime, inCheckInCB);
    }
    catch (...)
    {
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
