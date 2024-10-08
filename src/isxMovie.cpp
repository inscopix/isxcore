#include "isxMovie.h"

namespace isx
{

std::vector<uint16_t>
Movie::getFrameHeader(const size_t inFrameNumber)
{
    return std::vector<uint16_t>();
}

std::string
Movie::getFrameMetadata(const size_t inFrameNumber)
{
    return "null";
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
