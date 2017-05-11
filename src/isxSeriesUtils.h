#ifndef ISX_SERIES_UTILS_H
#define ISX_SERIES_UTILS_H

#include "isxTimingInfo.h"

namespace isx
{

/// \return The global timing info from many consistent timing infos.
///         Note that this is not valid for MovieSeries, which is currently
///         handled a little differently.
TimingInfo makeGlobalTimingInfo(const TimingInfos_t & inTis);

/// \return The timing info for series without gaps from many consistent
///         timing infos.
TimingInfo makeGaplessTimingInfo(const TimingInfos_t & inTis);

} // namespace isx

#endif // ISX_SERIES_UTILS_H
