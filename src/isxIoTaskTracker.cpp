#include "isxIoTaskTracker.h"
#include "isxIoTask.h"

#include "isxVideoFrame.h"
#include "isxImage.h"
#include "isxTrace.h"

#include <memory>
#include <map>
#include <functional>

namespace isx
{

template <typename T>
void
IoTaskTracker::schedule(IoFunc_t<T> inGetData, CallerFunc_t<T> inCallback)
{
    uint64_t readRequestId = 0;
    {
        ScopedMutex locker(m_pendingRequestsMutex, "IoTaskTracker::schedule");
        readRequestId = m_requestCount++;
    }

    std::weak_ptr<IoTaskTracker> weakThis = shared_from_this();
    Task_t asyncTask = 
        [inGetData, inCallback]()
        {
            inCallback(inGetData());
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
                inCallback(std::shared_ptr<T>());
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

template void IoTaskTracker::schedule(IoFunc_t<Trace<float>>, CallerFunc_t<Trace<float>>);
template void IoTaskTracker::schedule(IoFunc_t<Image>, CallerFunc_t<Image>);
template void IoTaskTracker::schedule(IoFunc_t<VideoFrame>, CallerFunc_t<VideoFrame>);


} // namespace isx
