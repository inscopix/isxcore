#ifndef ISX_GPIO_UTILS_H
#define ISX_GPIO_UTILS_H

#include "isxTimingInfo.h"
#include "isxEventBasedFileV2.h"
#include <string>
#include <vector>

namespace isx
{

/// This is currently used when writing nVoke 1 and nVista 3
/// GPIO data, so they they do things fairly consistently.
void
writePktsToEventBasedFile(
        const std::string & inOutputFilePath,
        const std::vector<EventBasedFileV2::DataPkt> & inPackets,
        const std::vector<std::string> & inChannels,
        const std::vector<SignalType> & inTypes,
        const Time & inStartTime,
        const DurationInSeconds & inSamplePeriod,
        const uint64_t inFirstMicrosecondOffset,
        const uint64_t inLastMicrosecondOffset);

} // namespace isx

#endif // ISX_GPIO_UTILS_H
