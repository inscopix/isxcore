#include "isxEvents.h"
#include "isxMosaicEvents.h"
#include "isxEventsSeries.h"

namespace isx
{

SpEvents_t
readEvents(const std::string & inFileName)
{
    return std::make_shared<MosaicEvents>(inFileName);
}

SpWritableEvents_t
writeEvents(
        const std::string & inFileName,
        const std::vector<std::string> & inChannelNames,
        const std::vector<DurationInSeconds> & inChannelSteps)
{
    return std::make_shared<MosaicEvents>(inFileName, inChannelNames, inChannelSteps);
}

SpEvents_t
readEventsSeries(const std::vector<std::string> & inFileNames)
{
    return std::make_shared<EventsSeries>(inFileNames);
}

} // namespace isx
