#include "isxAsyncTaskHandle.h"
#include "isxDispatchQueue.h"

namespace isx
{
AsyncTaskHandle::AsyncTaskHandle() {}

AsyncTaskHandle::AsyncTaskHandle(Task_t inTask, ProgressCB_t inProgressCB, FinishedCB_t inFinishedCB)
: m_task(inTask)
, m_progressCB(inProgressCB)
, m_finishedCB(inFinishedCB)
{

}

void 
AsyncTaskHandle::cancel() 
{
    m_cancelPending = true;
}

void 
AsyncTaskHandle::process() 
{
    WpAsyncTaskHandle_t weakThis = shared_from_this();
    DispatchQueue::poolQueue()->dispatch([weakThis, this](){
        SpAsyncTaskHandle_t sharedThis = weakThis.lock();
        if (!sharedThis)
        {
            return;
        }
        m_task();
        DispatchQueue::mainQueue()->dispatch([weakThis, this](){
            SpAsyncTaskHandle_t sharedThis = weakThis.lock();
            if (!sharedThis)
            {
                return;
            }
            m_finishedCB(AsyncTaskFinishedStatus::COMPLETE);
        });
    });
}
    
} // namespace isx
