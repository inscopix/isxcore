#include "isxAsyncTaskHandle.h"
#include "isxDispatchQueue.h"

namespace isx
{
AsyncTaskHandle::AsyncTaskHandle() {}

AsyncTaskHandle::AsyncTaskHandle(AsyncTask_t inTask, ProgressCB_t inProgressCB, FinishedCB_t inFinishedCB)
: m_task(inTask)
, m_progressCB(inProgressCB)
, m_finishedCB(inFinishedCB)
{}

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
        if (m_progressCB)
        {
            DispatchQueue::mainQueue()->dispatch([weakThis, this, inProgress](){
                SpAsyncTaskHandle_t sharedThis = weakThis.lock();
                if (!sharedThis)
                {
                    return;
                }
                m_progressCB(inProgress);
            });
        }
        return this->m_cancelPending;
    };

    DispatchQueue::poolQueue()->dispatch([weakThis, this, ci](){
        SpAsyncTaskHandle_t sharedThis = weakThis.lock();
        if (!sharedThis)
        {
            return;
        }
        try {
            m_taskStatus = AsyncTaskStatus::PROCESSING;
            m_taskStatus = m_task(ci);
        } catch (...) {
            m_exception = std::current_exception();
            m_taskStatus = AsyncTaskStatus::ERROR_EXCEPTION;
        }
        if (m_finishedCB)
        {
            DispatchQueue::mainQueue()->dispatch([weakThis, this](){
                SpAsyncTaskHandle_t sharedThis = weakThis.lock();
                if (!sharedThis)
                {
                    return;
                }
                m_finishedCB(m_taskStatus);
            });
        }
    });
}

AsyncTaskStatus
AsyncTaskHandle::getTaskStatus() const
{
    return m_taskStatus;
}

std::exception_ptr
AsyncTaskHandle::getExceptionPtr() const
{
    return m_exception;
}
    
} // namespace isx
