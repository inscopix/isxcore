#ifndef ISX_GPIO_UTILS_H
#define ISX_GPIO_UTILS_H

#include "isxTimingInfo.h"
#include "isxEventBasedFileV2.h"
#include <string>
#include <vector>

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
        const uint64_t inLastMicrosecondOffset);

} // namespace isx

#endif // ISX_GPIO_UTILS_H
