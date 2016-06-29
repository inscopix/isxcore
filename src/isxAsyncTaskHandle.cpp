#include "isxAsyncTaskHandle.h"
#include "isxDispatchQueue.h"

namespace isx
{
AsyncTaskHandle::AsyncTaskHandle() {}

AsyncTaskHandle::AsyncTaskHandle(AsyncTask_t inTask, ProgressCB_t inProgressCB, FinishedCB_t inFinishedCB)
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

    CheckInCB_t ci =[weakThis, this](float inProgress)
    {
        SpAsyncTaskHandle_t sharedThis = weakThis.lock();
        if (!sharedThis)
        {
            return false;
        }
        DispatchQueue::mainQueue()->dispatch([weakThis, this, inProgress](){
            SpAsyncTaskHandle_t sharedThis = weakThis.lock();
            if (!sharedThis)
            {
                return;
            }
            m_progressCB(inProgress);
        });
        return this->m_cancelPending;
    };

    DispatchQueue::poolQueue()->dispatch([weakThis, this, ci](){
        SpAsyncTaskHandle_t sharedThis = weakThis.lock();
        if (!sharedThis)
        {
            return;
        }
        AsyncTaskFinishedStatus status = m_task(ci);
        DispatchQueue::mainQueue()->dispatch([weakThis, this, status](){
            SpAsyncTaskHandle_t sharedThis = weakThis.lock();
            if (!sharedThis)
            {
                return;
            }
            m_finishedCB(status);
        });
    });
}
    
} // namespace isx
