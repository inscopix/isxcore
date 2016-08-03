#ifndef ISX_ASYNC_TASK_H
#define ISX_ASYNC_TASK_H

#include "isxCore.h"
#include "isxAsyncTaskHandle.h"
#include <functional>
#include <memory>

namespace isx
{
/// A class providing an interface for dealing with running tasks asynchronously.
/// Provided cancellation and progressr reporting.
///
class AsyncTask 
: public AsyncTaskHandle
, public std::enable_shared_from_this<isx::AsyncTask>
{
public:
    /// default constructor
    AsyncTask();

    /// Constructor
    /// \param inTask task to run asynchronously
    /// \param inProgressCB callback function to call with current progress (0.0: not started, 1.0: complete)
    /// \param inFinishedCB callback function to call when task finished
    AsyncTask(AsyncFunc_t inTask, AsyncProgressCB_t inProgressCB, AsyncFinishedCB_t inFinishedCB);

    virtual
    ~AsyncTask() override;

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

private:
    bool            m_cancelPending = false;
    AsyncFunc_t     m_task;
    AsyncProgressCB_t    m_progressCB;
    AsyncFinishedCB_t    m_finishedCB;

    AsyncTaskStatus m_taskStatus = AsyncTaskStatus::PENDING;
    std::exception_ptr  m_exception;
};
    
} // namespace isx


#endif // def ISX_ASYNC_TASK_H
