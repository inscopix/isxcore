#include "isxIoTaskTracker.h"
#include "isxIoTask.h"

#include <memory>
#include <map>
#include <functional>

namespace isx
{

void
IoTaskTracker::schedule(getFrameCB_t inGetFrame, MovieGetFrameCB_t inCallback)
{
    uint64_t readRequestId = 0;
    {
        ScopedMutex locker(m_pendingRequestsMutex, "IoTaskTracker::schedule");
        readRequestId = m_requestCount++;
    }

    std::weak_ptr<IoTaskTracker> weakThis = shared_from_this();
    Task_t asyncTask = 
        [inGetFrame, inCallback]()
        {
            inCallback(inGetFrame());
        };

    AsyncFinishedCB_t finishedCB =
        [weakThis, this, readRequestId, inCallback](AsyncTaskStatus inStatus)
        {
            auto sharedThis = weakThis.lock();
            if (!sharedThis)
            {
                return;
            }

            auto rt = unregisterPendingTask(readRequestId);

            checkAsyncTaskStatus(rt, inStatus, "IoTaskTracker::schedule");
            if (inStatus == AsyncTaskStatus::ERROR_EXCEPTION
              ||inStatus == AsyncTaskStatus::CANCELLED)
            {
                inCallback(SpVideoFrame_t());
            }
        };

    auto readIoTask = std::make_shared<IoTask>(asyncTask, finishedCB);

    {
        ScopedMutex locker(m_pendingRequestsMutex, "IoTaskTracker::schedule");
        m_pendingRequests[readRequestId] = readIoTask;
    }
    readIoTask->schedule();
}

void 
IoTaskTracker::cancelPendingTasks()
{
    ScopedMutex locker(m_pendingRequestsMutex, "cancelPendingTasks");
    for (auto & pr: m_pendingRequests)
    {
        pr.second->cancel();
    }
    m_pendingRequests.clear();
}

SpAsyncTaskHandle_t
IoTaskTracker::unregisterPendingTask(uint64_t inRequestId)
{
    ScopedMutex locker(m_pendingRequestsMutex, "unregisterPendingTask");
    auto ret = m_pendingRequests[inRequestId];
    m_pendingRequests.erase(inRequestId);
    return ret;
}

} // namespace isx
