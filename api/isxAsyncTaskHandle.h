#ifndef ISX_ASYNC_TASK_HANDLE_H
#define ISX_ASYNC_TASK_HANDLE_H

#include "isxDispatchQueue.h"
#include "isxCore.h"

#include <functional>
#include <memory>

namespace isx
{
/// A class providing an interface for dealing with running tasks asynchronously.
/// Provided cancellation and progressr reporting.
///
class AsyncTaskHandle : public std::enable_shared_from_this<isx::AsyncTaskHandle>
{
public:
    /// return status of an asynchronous task
    enum class FinishedStatus
    {
        COMPLETE,               ///< task completed successfully
        CANCELLED,              ///< task was cancelled
        UNKNOWN_ERROR,          ///< an error occurred while processing the task
        ERROR_EXCEPTION         ///< an exception occurred while processing the task
    };
    
    /// type of callback function that an asynchronous task has to call periodically
    typedef std::function<bool(float)> CheckInCB_t;
    /// type of function that implements the asynchronous task
    typedef std::function<FinishedStatus(CheckInCB_t)> AsyncTask_t;
    /// type of progress callback function
    typedef std::function<void(float)> ProgressCB_t;
    /// type of finished callback function
    typedef std::function<void(FinishedStatus inStatus)> FinishedCB_t;

    /// default constructor
    AsyncTaskHandle();

    /// Constructor
    /// \param inTask task to run asynchronously
    /// \param inProgressCB callback function to call with current progress (0.0: not started, 1.0: complete)
    /// \param inFinishedCB callback function to call when task finished
    AsyncTaskHandle(AsyncTask_t inTask, ProgressCB_t inProgressCB, FinishedCB_t inFinishedCB);

    /// cancel this task
    /// cancellation is complete when m_finishedCB is called with FinishedStatus CANCELLED
    void 
    cancel();

    /// start processing this task
    void
    process();

private:
    bool            m_cancelPending = false;
    AsyncTask_t     m_task;
    ProgressCB_t    m_progressCB;
    FinishedCB_t    m_finishedCB;
};
    
} // namespace isx


#endif // def ISX_ASYNC_TASK_HANDLE_H
