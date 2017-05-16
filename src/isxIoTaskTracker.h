#ifndef ISX_IO_TASK_TRACKER_H
#define ISX_IO_TASK_TRACKER_H

#include "isxIoTask.h"

#include "isxCore.h"
#include "isxMovie.h"
#include "isxVideoFrame.h"
#include "isxLogicalTrace.h"
#include "isxTrace.h"
#include "isxImage.h"
#include "isxMutex.h"
#include "isxAsyncTaskResult.h"

#include <memory>
#include <map>
#include <functional>

namespace isx
{

/// Utility class used by implementors of the Movie interface to track and possibly cancel IoTasks
template <typename T>
class IoTaskTracker : public std::enable_shared_from_this<IoTaskTracker<T>>
{
public:    
    /// I/O function to read data from disk (video frame, trace, image)
    template <typename D> using IoFunc_t =
        std::function<std::shared_ptr<D>()>;
    
    /// Callback used with the output of IoFunc_t as an input
    using CallerFunc_t =
        std::function<void(AsyncTaskResult<std::shared_ptr<T>> inAsyncResult)>;

    /// issues read frame request asynchronously
    /// \param inGetData function provided by caller
    /// \param inCallback callback function to be called with retrieved data returned by inGetData
    void
    schedule(IoFunc_t<T> inGetData, CallerFunc_t inCallback)
    {
        uint64_t readRequestId = 0;
        {
            ScopedMutex locker(m_pendingRequestsMutex, "IoTaskTracker::schedule readRequestId");
            readRequestId = m_requestCount++;
        }
        
        std::weak_ptr<IoTaskTracker> weakThis = this->shared_from_this();
        Task_t asyncTask =
        [weakThis, this, inGetData, readRequestId]()
        {
            auto sharedThis = weakThis.lock();
            if (!sharedThis)
            {
                return;
            }
            auto r = inGetData();

            ScopedMutex locker(m_pendingRequestsMutex, "IoTaskTracker::schedule asyncResults update");
            m_asyncResults[readRequestId].setValue(r);
        };
        
        AsyncFinishedCB_t finishedCB =
        [weakThis, this, readRequestId, inCallback](AsyncTaskStatus inStatus)
        {
            auto sharedThis = weakThis.lock();
            if (!sharedThis)
            {
                return;
            }
            
            // unregisterPendingTask moves any captured exceptions from the AsyncTaskHandle to the AsyncTaskResult
            auto rt = unregisterPendingTask(readRequestId);
            ISX_ASSERT(inStatus == rt.first->getTaskStatus());
            
            inCallback(rt.second);
        };

        auto readIoTask = std::make_shared<IoTask>(asyncTask, finishedCB);
        
        {
            ScopedMutex locker(m_pendingRequestsMutex, "IoTaskTracker::schedule insert into maps");
            m_pendingRequests[readRequestId] = readIoTask;
            m_asyncResults[readRequestId] = AsyncTaskResult<SpT_t>{};
        }
        readIoTask->schedule();
    }

    /// cancels all pending tasks
    void 
    cancelPendingTasks()
    {
        ScopedMutex locker(m_pendingRequestsMutex, "cancelPendingTasks");
        for (auto & pr: m_pendingRequests)
        {
            pr.second->cancel();
        }
    }

private:
    using SpT_t = std::shared_ptr<T>;
    
    std::pair<SpAsyncTaskHandle_t, AsyncTaskResult<SpT_t>>
    unregisterPendingTask(uint64_t inRequestId)
    {
        ScopedMutex locker(m_pendingRequestsMutex, "unregisterPendingTask");
        if (m_pendingRequests[inRequestId]->getTaskStatus() == AsyncTaskStatus::ERROR_EXCEPTION)
        {
            m_asyncResults[inRequestId].setException(std::move(m_pendingRequests[inRequestId]->getExceptionPtr()));
        }
        
        auto ret = std::make_pair(m_pendingRequests[inRequestId], m_asyncResults[inRequestId]);
        m_pendingRequests.erase(inRequestId);
        m_asyncResults.erase(inRequestId);
        return ret;
    }

    uint64_t                                    m_requestCount = 0;
    isx::Mutex                                  m_pendingRequestsMutex;
    std::map<uint64_t, SpAsyncTaskHandle_t>     m_pendingRequests;
    std::map<uint64_t, AsyncTaskResult<SpT_t>>  m_asyncResults;

};

extern template class IoTaskTracker<VideoFrame>;
extern template class IoTaskTracker<FTrace_t>;
extern template class IoTaskTracker<Image>;
extern template class IoTaskTracker<LogicalTrace>;
    
} // namespace isx
#endif // def ISX_IO_TASK_TRACKER_H
