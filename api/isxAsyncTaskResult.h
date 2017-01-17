#ifndef ISX_ASYNC_TASK_RESULT_H
#define ISX_ASYNC_TASK_RESULT_H

#include "isxCore.h"
#include "isxAsync.h"

#include <functional>
#include <memory>

namespace isx
{
/// A class the result (either the requested result or an exception) of an
/// asynchrnously executed task.  Currently this is only used for IoTasks
/// but it could (& should?) also be used in AsyncTasks.
///
template <typename T>
class AsyncTaskResult
{
public:
    /// Setter for result value of async operation.
    /// \param inValue new for this AsyncTaskResult instance.
    void
    setValue(const T& inValue) { m_value = inValue; }

    /// Setter for exception captured during async operation.
    /// \param inException new Exception for this AsyncTaskResult instance.
    void
    setException(std::exception_ptr inException) { m_exception = inException; }

    /// Accessor for result stored in this AsyncTaskResult instance.
    /// \throw  Exception   If the async task resulted in an exception 
    ///                     that was captured into this AsyncTaskResult instance. 
    T
    get() const { conditionallyRethrow(); return m_value; }

    /// Accessor for exception stored in this AsyncTaskResult instance.
    std::exception_ptr
    getException() const { return m_exception; }

    /// Rethrow captured exception if there is one, otherwise this is a no-op.
    /// \throw  Exception   If the async task resulted in an exception 
    ///                     that was captured into this AsyncTaskResult instance. 
    void
    conditionallyRethrow() const
    {
        if (m_exception) 
        { 
            std::rethrow_exception(m_exception);
        }
    }

private:
    T                   m_value;
    std::exception_ptr  m_exception;
};
    
} // namespace isx


#endif // def ISX_ASYNC_TASK_RESULT_H
