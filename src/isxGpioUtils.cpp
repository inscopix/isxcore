#include "isxGpioUtils.h"

namespace isx
{

void
writePktsToEventBasedFile(
        const std::string & inOutputFilePath,
        const std::vector<EventBasedFileV2::DataPkt> & inPackets,
        const std::vector<std::string> & inChannels,
        const std::vector<SignalType> & inTypes,
        const Time & inStartTime,
        const DurationInSeconds & inSamplePeriod,
        const uint64_t inFirstMicrosecondOffset,
        const uint64_t inLastMicrosecondOffset)
{
    const auto duration = DurationInSeconds::fromMicroseconds(inLastMicrosecondOffset - inFirstMicrosecondOffset + 1);
    const isize_t numTimes = isize_t(std::ceil(duration.toDouble() / inSamplePeriod.toDouble()));
    const TimingInfo timing(inStartTime, inSamplePeriod, numTimes);

    ISX_ASSERT(inChannels.size() == inTypes.size());
    const std::vector<DurationInSeconds> steps(inChannels.size(), inSamplePeriod);

    EventBasedFileV2 outputFile(inOutputFilePath, DataSet::Type::GPIO, inChannels, steps, inTypes);
    for (const auto p : inPackets)
    {
        EventBasedFileV2::DataPkt pkt;
        if (p.offsetMicroSecs >= inFirstMicrosecondOffset)
        {
            pkt.offsetMicroSecs = p.offsetMicroSecs - inFirstMicrosecondOffset;
            pkt.signal = p.signal;
            pkt.value = p.value;
            outputFile.writeDataPkt(pkt);
        }
        else
        {
            ISX_LOG_ERROR("Tried to write packet with negative offset. Skipping.");
        }
    }

    outputFile.setTimingInfo(timing.getStart(), timing.getEnd());
    outputFile.closeFileForWriting();
}

} // namespace isx
