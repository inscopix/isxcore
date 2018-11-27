#include "isxMovie.h"

namespace isx
{

std::vector<uint16_t>
Movie::getFrameHeader(const size_t inFrameNumber)
{
    return std::vector<uint16_t>();
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

} // namespace isx
