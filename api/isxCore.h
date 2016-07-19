#ifndef ISX_CORE_H
#define ISX_CORE_H

#include <stddef.h>


namespace isx
{

    /// The type of all sizes, lengths and indices
    typedef size_t isize_t;

    void CoreInitialize();
    bool CoreIsInitialized();
    void CoreShutdown();

    /// \cond doxygen chokes on enum class inside of namespace
    /// return status of an asynchronous task
    enum class AsyncTaskFinishedStatus    
    {
        COMPLETE,               ///< task completed successfully
        CANCELLED,              ///< task was cancelled
        UNKNOWN_ERROR,          ///< an error occurred while processing the task
        ERROR_EXCEPTION         ///< an exception occurred while processing the task
    };
    /// \endcond doxygen chokes on enum class inside of namespace

} // namespace isx

#endif // def ISX_CORE_H
