#include "isxIoQueue.h"
#include "isxAsync.h"
#include "isxIoTask.h"
#include "isxDispatchQueue.h"

namespace isx
{
IoTask::IoTask() {}

IoTask::IoTask(Task_t inTask, AsyncFinishedCB_t inFinishedCB)
: m_task(inTask)
, m_finishedCB(inFinishedCB)
{}

IoTask::~IoTask() {}

void 
IoTask::cancel() 
{
    m_cancelPending = true;
}

void 
IoTask::schedule() 
{
    IoQueue::instance()->enqueue(shared_from_this());
}

AsyncTaskStatus
IoTask::getTaskStatus() const
{
    return m_taskStatus;
}

std::exception_ptr
IoTask::getExceptionPtr() const
{
    return m_exception;
}

bool
IoTask::isCancelPending() const
{
    return m_cancelPending;
}

Task_t &
IoTask::getTask()
{
    return m_task;
}

AsyncFinishedCB_t &
IoTask::getFinishedCB()
{
    return m_finishedCB;
}

void
IoTask::setTaskStatus(AsyncTaskStatus inStatus)
{
    m_taskStatus = inStatus;
}

void
IoTask::setExceptionPtr(std::exception_ptr inExceptionPtr)
{
    m_exception = inExceptionPtr;
}
    
} // namespace isx
