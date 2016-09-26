#include "isxAsyncFrameReader.h"
#include "isxIoTask.h"

#include <memory>
#include <map>
#include <functional>

namespace isx
{

void
AsyncFrameReader::getFrameAsync(getFrameCB_t inGetFrame, MovieGetFrameCB_t inCallback)
{
    uint64_t readRequestId = 0;
    {
        ScopedMutex locker(m_pendingReadsMutex, "getFrameAsync");
        readRequestId = m_readRequestCount++;
    }

    std::weak_ptr<AsyncFrameReader> weakThis = shared_from_this();
    auto readIoTask = std::make_shared<IoTask>(
        [inGetFrame, inCallback]()
        {
            inCallback(inGetFrame());
        },
        [weakThis, this, readRequestId, inCallback](AsyncTaskStatus inStatus)
        {
            auto sharedThis = weakThis.lock();
            if (!sharedThis)
            {
                return;
            }

            auto rt = unregisterReadRequest(readRequestId);

            checkAsyncTaskStatus(rt, inStatus, "MosaicMovie::getFrameAsync");
            if (inStatus == AsyncTaskStatus::ERROR_EXCEPTION)
            {
                inCallback(SpVideoFrame_t());
            }
        }
    );

    {
        ScopedMutex locker(m_pendingReadsMutex, "getFrameAsync");
        m_pendingReads[readRequestId] = readIoTask;
    }
    readIoTask->schedule();
}

void 
AsyncFrameReader::cancelPendingReads()
{
    ScopedMutex locker(m_pendingReadsMutex, "cancelPendingReads");
    for (auto & pr: m_pendingReads)
    {
        pr.second->cancel();
    }
    m_pendingReads.clear();
}

SpAsyncTaskHandle_t
AsyncFrameReader::unregisterReadRequest(uint64_t inReadRequestId)
{
    ScopedMutex locker(m_pendingReadsMutex, "unregisterPendingRead");
    auto ret = m_pendingReads[inReadRequestId];
    m_pendingReads.erase(inReadRequestId);
    return ret;
}

} // namespace isx
