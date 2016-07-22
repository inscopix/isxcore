#ifndef ISX_CORE_H
#define ISX_CORE_H

#include <stddef.h>
#include <vector>

namespace isx
{

    /// The type of all sizes, lengths and indices
    typedef size_t isize_t;

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

    void CoreInitialize();
    bool CoreIsInitialized();
    void CoreShutdown();

    int CoreVersionMajor();
    int CoreVersionMinor();
    int CoreVersionPatch();

    /// \return     The version numbers in a vector.
    ///
    std::vector<int> CoreVersionVector();

} // namespace isx

#endif // def ISX_CORE_H
