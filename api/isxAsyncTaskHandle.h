#ifndef ISX_ASYNC_TASK_HANDLE_H
#define ISX_ASYNC_TASK_HANDLE_H

#include "isxCore.h"
#include "isxAsync.h"

#include <functional>
#include <memory>

namespace isx
{
/// A class providing an interface for dealing with running tasks asynchronously.
/// Provided cancellation and progressr reporting.
///
class AsyncTaskHandle
{
public:

    /// Destructor
    ///
    virtual
    ~AsyncTaskHandle() = 0;

    /// cancel this task
    /// cancellation is complete when the finished callback is called with FinishedStatus CANCELLED
    virtual
    void 
    cancel() = 0;

    /// schedule this task for processing
    virtual
    void
    schedule() = 0;

    ///\return status of finished task. Use this if no finished callback was specified.
    virtual
    AsyncTaskStatus
    getTaskStatus() const = 0;

    ///\return current exception, if one has occurred while processing this task
    virtual
    std::exception_ptr
    getExceptionPtr() const = 0;
};

inline AsyncTaskHandle::~AsyncTaskHandle(){}
    
} // namespace isx


#endif // def ISX_ASYNC_TASK_HANDLE_H
