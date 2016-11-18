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
    /// I/O function to read data from disk (video frame, trace, image)
    template <typename T> using IoFunc_t = 
        std::function<std::shared_ptr<T>()>;

    /// Callback used with the output of IoFunc_t as an input
    template <typename T> using CallerFunc_t = 
        std::function<void(const std::shared_ptr<T> & inData)>;
        
    /// issues read frame request asynchronously
    /// \param inGetData function provided by caller
    /// \param inCallback callback function to be called with retrieved data returned by inGetData
    template <typename T>
    void 
    schedule(IoFunc_t<T> inGetData, CallerFunc_t<T> inCallback);

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
