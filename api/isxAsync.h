#ifndef ISX_ASYNC_H
#define ISX_ASYNC_H

#include "isxCore.h"
#include "isxCoreFwd.h"
#include <functional>

namespace isx
{

/// \cond doxygen chokes on enum class inside of namespace
/// status of an asynchronous task
enum class AsyncTaskStatus    
{
    PENDING,                ///< task is pending / not done processing
    PROCESSING,             ///< task is processing
    COMPLETE,               ///< task completed successfully
    CANCELLED,              ///< task was cancelled
    ERROR_EXCEPTION,        ///< an exception occurred while processing the task
    UNKNOWN_ERROR           ///< an error occurred while processing the task
};
/// \endcond doxygen chokes on enum class inside of namespace

/// type of finished callback function
typedef std::function<void(AsyncTaskStatus inStatus)> AsyncFinishedCB_t;
/// type of callback function that an asynchronous task has to call periodically
typedef std::function<bool(float)> AsyncCheckInCB_t;
/// type of function that implements the asynchronous task
typedef std::function<AsyncTaskStatus(AsyncCheckInCB_t)> AsyncFunc_t;
/// type of progress callback function
typedef std::function<void(float)> AsyncProgressCB_t;

SpAsyncTaskHandle_t 
CreateAsyncTask(AsyncFunc_t inTask, AsyncProgressCB_t inProgressCB, AsyncFinishedCB_t inFinishedCB);

/// Check the status of an asynchronous task.
///
/// This simply logs errors/info and does not throw exceptions.
///
/// \param  inTask      The task used to rethrow an exception if needed.
/// \param  inStatus    The status to check.
/// \param  inTaskName  The name of the task.
void
checkAsyncTaskStatus(
        const SpAsyncTaskHandle_t & inTask,
        AsyncTaskStatus inStatus,
        const std::string & inTaskName);

} // namespace isx

#endif // def ISX_ASYNC_H
