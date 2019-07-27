#include "isxGpioExporter.h"

#include "isxException.h"
#include "isxExport.h"
#include "isxIMUFile.h"
#include "isxPathUtils.h"
#include "isxSeriesUtils.h"

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

AsyncTaskStatus
runIMUExporter(
    GpioExporterParams inParams,
    std::shared_ptr<GpioExporterOutputParams> inOutputParams,
    AsyncCheckInCB_t inCheckInCB)
{
    bool cancelled = false;
    auto & imus = inParams.m_srcs;

    // validate inputs
    if (imus.empty())
    {
        inCheckInCB(1.f);
        return AsyncTaskStatus::COMPLETE;
    }

    const size_t numSegments = imus.size();
    const SpGpio_t & refIMU = imus.front();
    const std::vector<std::string> channelNames = refIMU->getChannelList();

    Time baseTime;
    switch (inParams.m_writeTimeRelativeTo)
    {
        case WriteTimeRelativeTo::FIRST_DATA_ITEM:
        {
            baseTime = refIMU->getTimingInfo().getStart();
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
        imus[s]->getAllTraces(continuousTraces, logicalTraces);

        for (size_t c = 0; c < numChannels; ++c)
        {
            traces[c].push_back(logicalTraces[c]);
        }
    }

    std::vector<std::vector<std::vector<SpLogicalTrace_t>>> tracesIMU;
    std::vector<std::vector<SpLogicalTrace_t>> sectionTraces1;
    std::vector<std::vector<SpLogicalTrace_t>> sectionTraces2;
    sectionTraces1.push_back(traces[0]);
    sectionTraces1.push_back(traces[1]);
    sectionTraces1.push_back(traces[2]);
    sectionTraces2.push_back(traces[3]);
    sectionTraces2.push_back(traces[4]);
    sectionTraces2.push_back(traces[5]);
    sectionTraces1.push_back(traces[6]);
    sectionTraces1.push_back(traces[7]);
    sectionTraces1.push_back(traces[8]);
    tracesIMU.push_back(sectionTraces1);
    tracesIMU.push_back(sectionTraces2);

    std::vector<std::vector<std::string>> names;
    std::vector<std::string> names1;
    std::vector<std::string> names2;
    names1.push_back(channelNames[0]);
    names1.push_back(channelNames[1]);
    names1.push_back(channelNames[2]);
    names2.push_back(channelNames[3]);
    names2.push_back(channelNames[4]);
    names2.push_back(channelNames[5]);
    names1.push_back(channelNames[6]);
    names1.push_back(channelNames[7]);
    names1.push_back(channelNames[8]);
    names.push_back(names1);
    names.push_back(names2);

    try
    {
        cancelled = writeIMULogicalTraces(strm, tracesIMU, names, baseTime, inCheckInCB);
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
