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

    std::string errorMessage;
    for (isize_t i = 1; i < gpios.size(); ++i)
    {
        if (!checkNewMemberOfSeries({gpios[i - 1]}, gpios[i], errorMessage))
        {
            ISX_THROW(ExceptionSeries, errorMessage);
        }
    }

    const SpGpio_t & refGpio = gpios.front();
    const isize_t numChannels = refGpio->numberOfChannels();
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

    const int32_t maxDecimalsForDouble = std::numeric_limits<double>::digits10 + 1;
    const int32_t maxDecimalsForFloat = std::numeric_limits<float>::digits10 + 1;

    std::ofstream strm(inParams.m_fileName, std::ios::trunc);
    if (!strm.good())
    {
        ISX_THROW(ExceptionFileIO, "Error writing to output file.");
    }

    // shared setup for progress reporting
    float progress = 0.f;
    isize_t numLinesTotal = 0;
    isize_t numLinesWritten = 0;

    // the first column of analog and logical traces is a time stamp
    strm << "Time (s)";

    if (refIsAnalog)
    {
        for (auto & name : channelNames)
        {
            strm << ", " << name;       
        }
        strm << "\n";

        for (auto & gpio : gpios)
        {
            numLinesTotal += gpio->getTimingInfo().getNumTimes();
        }

        for (const auto & gpio : gpios)
        {
            std::vector<SpFTrace_t> traces(numChannels);
            isize_t n(0);
            for (auto & name : channelNames)
            {
                traces[n++] = gpio->getAnalogData(name);
            }

            const TimingInfo & ti = gpio->getTimingInfo();
            const isize_t numSamples = ti.getNumTimes();
            
            
            for (isize_t s = 0; s < numSamples; ++s)
            {
                const Time time = ti.convertIndexToStartTime(s);
                strm << std::setprecision(maxDecimalsForDouble)
                     << (time - baseTime).toDouble();

                for (isize_t i(0); i < numChannels; ++i)
                {
                    const SpFTrace_t trace = traces.at(i);
                    strm << ", "
                         << std::setprecision(maxDecimalsForFloat)
                         << trace->getValue(s);
                }

                strm << "\n";

                ++numLinesWritten;
                progress = float(numLinesWritten) / float(numLinesTotal);
                cancelled = inCheckInCB(progress);
                if (cancelled)
                {
                    break;
                }
            }  
        }
    }
    else
    {
        strm << std::setprecision(maxDecimalsForDouble);

        // we write all time/value pairs of each channel sequentially, so
        // we also need a channel name column (which will be sorted)
        strm << ", Channel Name, Value\n";

        // to calculate the number of lines, we need to read all the logical
        // traces, so we store them to avoid a repeated read
        std::vector<std::vector<SpLogicalTrace_t>> traces(numChannels);
        for (isize_t c = 0; c < numChannels; ++c)
        {
            for (const auto & gpio : gpios)
            {
                traces[c].push_back(gpio->getLogicalData(channelNames.at(c)));
                numLinesTotal += traces[c].back()->getValues().size();
            }
        }

        for (isize_t c = 0; c < numChannels; ++c)
        {
            const std::string & channelName = channelNames.at(c);
            for (size_t g = 0; g < gpios.size(); ++g)
            {
                const SpLogicalTrace_t & trace = traces.at(c).at(g);
                for (const auto & tv : trace->getValues())
                {
                    strm << (tv.first - baseTime).toDouble() << ", "
                         << channelName << ", "
                         << tv.second << "\n";

                    ++numLinesWritten;
                    progress = float(numLinesWritten) / float(numLinesTotal);
                    cancelled = inCheckInCB(progress);
                    if (cancelled)
                    {
                        break;
                    }
                }
            }
        }
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
