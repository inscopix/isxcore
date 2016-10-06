#ifndef ISX_IO_TASK_TRACKER_H
#define ISX_IO_TASK_TRACKER_H


#include "isxCore.h"
#include "isxMovie.h"
#include "isxMutex.h"

#include <memory>
#include <map>
#include <functional>

namespace isx
{

/// Utility class used by implementors of the Movie interface to track and possibly cancel IoTasks
class IoTaskTracker : public std::enable_shared_from_this<IoTaskTracker>
{
public:
    /// alias for getFrame callback to be passed into this frame reader
    using getFrameCB_t = std::function<SpVideoFrame_t()>;

    /// issues read frame request asynchronously
    /// \param inGetFrame getFrame function provided by caller
    /// \param inCallback callback function to be called with retrieved frame returned by inGetFrame
    // NOTE aschildan 9/28/2016: this could be made a function template on the two parameters
    void 
    schedule(getFrameCB_t inGetFrame, MovieGetFrameCB_t inCallback);

    /// cancels all pending tasks
    void 
    cancelPendingTasks();

private:
    SpAsyncTaskHandle_t
    unregisterPendingTask(uint64_t inRequestId);

    uint64_t                                m_requestCount = 0;
    isx::Mutex                              m_pendingRequestsMutex;
    std::map<uint64_t, SpAsyncTaskHandle_t> m_pendingRequests;

};
} // namespace isx
#endif // def ISX_IO_TASK_TRACKER_H
