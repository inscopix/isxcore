#include "isxGpioExporter.h"
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
GpioExporterParams::getOpName()
{
    return "Export GPIO";
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
    const bool refIsAnalog = refGpio->isAnalog();

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

    if (refIsAnalog)
    {
        std::vector<std::vector<SpFTrace_t>> traces(numSegments);
        for (size_t s = 0; s < numSegments; ++s)
        {
            for (const auto & name : channelNames)
            {
                traces[s].push_back(gpios[s]->getAnalogData(name));
            }
        }

        cancelled = writeTraces(strm, traces, channelNames, {}, baseTime, inCheckInCB);
    }
    else
    {
        const size_t numChannels = channelNames.size();
        std::vector<std::vector<SpLogicalTrace_t>> traces(numChannels);
        for (size_t c = 0; c < numChannels; ++c)
        {
            for (size_t s = 0; s < numSegments; ++s)
            {
                traces[c].push_back(gpios[s]->getLogicalData(channelNames[c]));
            }
        }

        cancelled = writeLogicalTraces(strm, traces, channelNames, "Channel Name", baseTime, inCheckInCB);
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
