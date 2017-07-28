#include "isxEvents.h"
#include "isxMosaicEvents.h"


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
    /// TODO: implement when we have support for Event series
    ISX_ASSERT(false);
    return nullptr;
}

} // namespace isx
