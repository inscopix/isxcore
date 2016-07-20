#ifndef ISX_ASYNC_TASK_HANDLE_H
#define ISX_ASYNC_TASK_HANDLE_H

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
    
    /// type of callback function that an asynchronous task has to call periodically
    typedef std::function<bool(float)> CheckInCB_t;
    /// type of function that implements the asynchronous task
    typedef std::function<AsyncTaskFinishedStatus(CheckInCB_t)> AsyncTask_t;
    /// type of progress callback function
    typedef std::function<void(float)> ProgressCB_t;
    /// type of finished callback function
    typedef std::function<void(AsyncTaskFinishedStatus inStatus)> FinishedCB_t;

    /// default constructor
    AsyncTaskHandle();

    /// Constructor
    /// \param inTask task to run asynchronously
    /// \param inProgressCB callback function to call with current progress (0.0: not started, 1.0: complete)
    /// \param inFinishedCB callback function to call when task finished
    AsyncTaskHandle(AsyncTask_t inTask, ProgressCB_t inProgressCB, FinishedCB_t inFinishedCB);

    /// helper function template to create an async task
    /// \param inFunc function to call as part of task
    /// \param inSrc source data to pass into inFunc
    /// \param outDst destination where inFunc should write its output
    template <typename TF, typename TI, typename TO>
    static
    AsyncTask_t 
    MakeAsyncTask(TF inFunc, TI inSrc, TO outDst)
    {
        return [inFunc, inSrc, outDst](isx::AsyncTaskHandle::CheckInCB_t inCheckInCB)
            {
                return inFunc(inSrc, outDst, inCheckInCB);
            };
    }

    /// cancel this task
    /// cancellation is complete when m_finishedCB is called with FinishedStatus CANCELLED
    void 
    cancel();

    /// start processing this task
    void
    process();

    ///\return status of finished task. Use this if no finished callback was specified.
    AsyncTaskFinishedStatus
    getTaskStatus() const;

private:
    bool            m_cancelPending = false;
    AsyncTask_t     m_task;
    ProgressCB_t    m_progressCB = nullptr;
    FinishedCB_t    m_finishedCB = nullptr;

    AsyncTaskFinishedStatus m_taskStatus = AsyncTaskFinishedStatus::PENDING;
};
    
} // namespace isx


#endif // def ISX_ASYNC_TASK_HANDLE_H
