#include "isxMovie.h"

namespace isx
{

SpVideoFrame_t
Movie::getFrameWithHeaderFooter(const size_t inFrameNumber)
{
    return getFrame(inFrameNumber);
}

std::string
Movie::getExtraProperties() const
{
    return "";
}

} // namespace isx
