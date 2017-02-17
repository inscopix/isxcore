#include "isxIoTaskTracker.h"

namespace isx
{

template class IoTaskTracker<VideoFrame>;
template class IoTaskTracker<FTrace_t>;
template class IoTaskTracker<Image>;

} // namespace isx
