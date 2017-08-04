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
writeEvents(const std::string & inFileName)
{
    return std::make_shared<MosaicEvents>(inFileName, true);
}

SpEvents_t
readEventsSeries(const std::vector<std::string> & inFileNames)
{
    return std::make_shared<EventsSeries>(inFileNames);
}

} // namespace isx
