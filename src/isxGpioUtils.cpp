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
        const uint64_t inFirstMicrosecondOffset,
        const uint64_t inLastMicrosecondOffset)
{
    const isize_t numTimes = inLastMicrosecondOffset - inFirstMicrosecondOffset + 1;
    const auto step = DurationInSeconds::fromMicroseconds(1);
    const TimingInfo timing(inStartTime, step, numTimes);

    ISX_ASSERT(inChannels.size() == inTypes.size());
    const std::vector<DurationInSeconds> steps(inChannels.size(), step);

    EventBasedFileV2 outputFile(inOutputFilePath, DataSet::Type::GPIO, inChannels, steps, inTypes);
    for (const auto p : inPackets)
    {
        EventBasedFileV2::DataPkt pkt;
        pkt.offsetMicroSecs = p.offsetMicroSecs - inFirstMicrosecondOffset;
        pkt.signal = p.signal;
        pkt.value = p.value;
        outputFile.writeDataPkt(pkt);
    }

    outputFile.setTimingInfo(timing.getStart(), timing.getEnd());
    outputFile.closeFileForWriting();
}

} // namespace isx
