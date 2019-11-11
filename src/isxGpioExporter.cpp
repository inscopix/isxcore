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
    uint8_t section1Size = IMUFile::s_maxAccArrSize + IMUFile::s_maxOriArrSize;
    for (uint8_t i = 0; i < section1Size; ++i)
    {
        sectionTraces1.push_back(traces[i]);
    }
    for (uint8_t i = section1Size; i < section1Size + IMUFile::s_maxMagArrSize; ++i)
    {
        sectionTraces2.push_back(traces[i]);
    }
    tracesIMU.push_back(sectionTraces1);
    tracesIMU.push_back(sectionTraces2);

    std::vector<std::vector<std::string>> names;
    std::vector<std::string> names1;
    std::vector<std::string> names2;
    for (uint8_t i = 0; i < section1Size; ++i)
    {
        names1.push_back(channelNames[i]);
    }
    for (uint8_t i = section1Size; i < section1Size + IMUFile::s_maxMagArrSize; ++i)
    {
        names2.push_back(channelNames[i]);
    }
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
