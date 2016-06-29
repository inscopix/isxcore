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
    AsyncTaskHandle(Task_t inTask, ProgressCB_t inProgressCB, FinishedCB_t inFinishedCB);

    /// cancel this task
    /// cancellation is complete when m_finishedCB is called with FinishedStatus CANCELLED
    void 
    cancel();

    /// start processing this task
    void
    process();

private:
    bool            m_cancelPending = false;
    Task_t          m_task;
    ProgressCB_t    m_progressCB;
    FinishedCB_t    m_finishedCB;
};
    
} // namespace isx


#endif // def ISX_ASYNC_TASK_HANDLE_H

#if 0
in main-thread / model:

....

{
    ....

    Data1 * input = setupInput();
    Data2 * output = setupOutput();
    Task_t t(algo1, input, output);

    SpAsyncTaskHandle_t h = process(t, progress_callback, finished_callback);

    // do we need pause / resume?

    h->cancel();

    ....
}
....

SpAsyncTaskHandle_t process(Task_t t, progress_callback, finished_callback)
{
    SpAsyncTaskHandle_t h(progress_callback, finished_callback);
    t(h);
    return h;
}

....

void algo_1(Data1 * input, Data2 * output, SpAsyncTaskHandle_t h)
{
    for(xyz = ...)
    {

        ....

        if (h->checkIn(progress))
        {
            cancelPending = true;
            break;
        }
    }
    
    ....

    h->finished(cancelPending ? CANCELLED : COMPLETE);
}

....

void asyncTaskFinishedCallback(Status inStatus)
{
    if (inStatus == COMPLETE)
    {

    }
    else if (inStatus == CANCELLED)
    {

    }
    else if (inStatus == ERROR_1)
    {
    
    }
}


class AsyncTaskHandle
{
public:
    // "owner" API
    void cancel() { m_cancelPending = true; }
    // optional:
    float getProgress();

    // task API
    bool checkIn(float inProgress) {m_progressCallback(inProgress); return m_cancelPending;} 
    void finished(Status inStatus) {m_finishedCallback(inStatus)}

private:
    bool m_cancelPending = false;
    std::func m_progressCallback;
    std::func m_finishedCallback;
}

#endif

