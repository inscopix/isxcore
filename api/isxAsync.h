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

/// helper function template to create an async task
/// \param inFunc function to call as part of task
/// \param inParams parameters object to pass into inFunc
template <typename TF, typename TP>
static
AsyncFunc_t
MakeAsyncFunc(TF inFunc, TP inParams)
{
    return [inFunc, inParams](AsyncCheckInCB_t inCheckInCB)
        {
            return inFunc(inParams, inCheckInCB);
        };
}

SpAsyncTaskHandle_t 
CreateAsyncTask(AsyncFunc_t inTask, AsyncProgressCB_t inProgressCB, AsyncFinishedCB_t inFinishedCB);

    
} // namespace isx


#endif // def ISX_ASYNC_H
