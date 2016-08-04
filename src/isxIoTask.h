#ifndef ISX_IO_TASK_H
#define ISX_IO_TASK_H

#include "isxCore.h"
#include "isxAsyncTaskHandle.h"
#include "isxDispatchQueue.h"
#include <functional>
#include <memory>

namespace isx
{
/// A class providing an interface for dealing with running tasks asynchronously.
/// Provided cancellation and progressr reporting.
///
class IoTask 
: public AsyncTaskHandle
, public std::enable_shared_from_this<IoTask>
{
public:
    /// default constructor
    IoTask();

    /// Constructor
    /// \param inTask task to run asynchronously
    /// \param inFinishedCB callback function to call when task finished
    IoTask(Task_t inTask, AsyncFinishedCB_t inFinishedCB);

    virtual
    ~IoTask() override;

    /// cancel this task
    /// cancellation is complete when m_finishedCB is called with FinishedStatus CANCELLED
    virtual
    void 
    cancel() override;

    /// schedule this taks for processing
    virtual
    void
    schedule() override;

    ///\return status of finished task. Use this if no finished callback was specified.
    virtual
    AsyncTaskStatus
    getTaskStatus() const override;

    ///\return current exception, if one has occurred while processing this task
    virtual
    std::exception_ptr
    getExceptionPtr() const override;

    ///\ return true if this task has been cancelled
    bool
    isCancelPending() const;

    ///\return this IoTask's embedded work task
    Task_t &
    getTask();

    ///\return this IoTask's finished CB
    AsyncFinishedCB_t &
    getFinishedCB();

    /// set this IoTask's status
    /// \param inStatus new status for this task
    void
    setTaskStatus(AsyncTaskStatus inStatus);

    /// set this IoTask's exception_ptr
    /// \param inExceptionPtr new exception_ptr
    void
    setExceptionPtr(std::exception_ptr inExceptionPtr);

private:
    bool                m_cancelPending = false;
    Task_t              m_task;
    AsyncFinishedCB_t   m_finishedCB;

    AsyncTaskStatus     m_taskStatus = AsyncTaskStatus::PENDING;
    std::exception_ptr  m_exception;
};
    
typedef std::shared_ptr<IoTask> SpIoTask_t;
    
} // namespace isx


#endif // def ISX_IO_TASK_H
