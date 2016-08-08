#include "isxAsync.h"
#include "isxAsyncTask.h"
#include "isxDispatchQueue.h"

namespace isx
{
AsyncTask::AsyncTask() {}

AsyncTask::AsyncTask(AsyncFunc_t inTask, AsyncProgressCB_t inProgressCB, AsyncFinishedCB_t inFinishedCB)
: m_task(inTask)
, m_progressCB(inProgressCB)
, m_finishedCB(inFinishedCB)
{}

AsyncTask::~AsyncTask() {}

void 
AsyncTask::cancel() 
{
    m_cancelPending = true;
}

void 
AsyncTask::schedule() 
{
    WpAsyncTaskHandle_t weakThis = shared_from_this();

    AsyncCheckInCB_t ci =[weakThis, this](float inProgress)
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
AsyncTask::getTaskStatus() const
{
    return m_taskStatus;
}

std::exception_ptr
AsyncTask::getExceptionPtr() const
{
    return m_exception;
}

SpAsyncTaskHandle_t 
CreateAsyncTask(AsyncFunc_t inTask, AsyncProgressCB_t inProgressCB, AsyncFinishedCB_t inFinishedCB)
{
    return std::make_shared<AsyncTask>(inTask, inProgressCB, inFinishedCB);
}
    
} // namespace isx
