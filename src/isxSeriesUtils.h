#ifndef ISX_SERIES_UTILS_H
#define ISX_SERIES_UTILS_H

#include "isxTimingInfo.h"
#include "isxCoreFwd.h"

#include <vector>

namespace isx
{

/// \param  inGpios         The GPIO members sorted in time (oldest is first).
/// \throw  ExceptionSeries If the series members are not consistent.
void
checkGpioSeriesMembers(const std::vector<SpGpio_t> & inGpios);

/// \return The global timing info from many consistent timing infos.
///         Note that this is not valid for MovieSeries, which is currently
///         handled a little differently.
TimingInfo makeGlobalTimingInfo(const TimingInfos_t & inTis);

/// \return The timing info for series without gaps from many consistent
///         timing infos.
TimingInfo makeGaplessTimingInfo(const TimingInfos_t & inTis);

} // namespace isx

#endif // ISX_SERIES_UTILS_H
