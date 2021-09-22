#include "isxMovie.h"

namespace isx
{

std::vector<uint16_t>
Movie::getFrameHeader(const size_t inFrameNumber)
{
    return std::vector<uint16_t>();
}

std::unordered_map<std::string, uint64_t>
Movie::getFrameHeaderMetadata(const size_t inFrameNumber)
{
    return std::unordered_map<std::string, uint64_t>();
}

std::vector<uint16_t>
Movie::getFrameFooter(const size_t inFrameNumber)
{
    return std::vector<uint16_t>();
}

std::string
Movie::getExtraProperties() const
{
    return "null";
}

SpacingInfo
Movie::getOriginalSpacingInfo() const
{
    return SpacingInfo::getDefault();
}

bool
Movie::hasFrameTimestamps() const
{
    return false;
}

uint64_t
Movie::getFrameTimestamp(const isize_t inIndex)
{
    (void)inIndex;
    return 0;
}

void
Movie::closeFileStream()
{
}

} // namespace isx
