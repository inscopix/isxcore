#ifndef ISX_DELTA_F_OVER_F_H
#define ISX_DELTA_F_OVER_F_H

#include "isxCoreFwd.h"
#include "isxAsyncTaskHandle.h"

namespace isx {
    /// Applies DF/F
    /// \param inSource input movie
    /// \param inDest output movie
    /// \param inCheckInCB callback to invoke periodically
    AsyncTaskHandle::FinishedStatus applyDff(const SpMovie_t & inSource, const SpMovie_t & inDest, AsyncTaskHandle::CheckInCB_t inCheckInCB);
}
#endif // def ISX_DELTA_F_OVER_F_H
