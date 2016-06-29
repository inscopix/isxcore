#ifndef ISX_CORE_H
#define ISX_CORE_H

#include <stddef.h>

namespace isx
{
    /// The type of all sizes, lengths and indices
    typedef size_t isize_t;

    /// Status reported back by asynchronous tasks
    enum class AsyncTaskFinishedStatus
    {
        COMPLETE,
        CANCELLED,
        ERROR_EXCEPTION
    };

    void CoreInitialize();
    bool CoreIsInitialized();
    void CoreShutdown();
}


#endif // def ISX_CORE_H
