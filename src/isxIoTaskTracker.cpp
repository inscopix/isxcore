#include "isxIoTaskTracker.h"

namespace isx
{

template class IoTaskTracker<VideoFrame>;
template class IoTaskTracker<FTrace_t>;
template class IoTaskTracker<Image>;
template class IoTaskTracker<DTrace_t>;
template class IoTaskTracker<LogicalTrace>;

} // namespace isx
