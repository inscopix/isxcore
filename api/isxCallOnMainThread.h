#ifndef ISX_CALL_ON_MAIN_THREAD_H
#define ISX_CALL_ON_MAIN_THREAD_H

#include <functional>

namespace isx
{
    /// dispatches a task (lambda / closure) to the applications main thread
    /// any exceptions thrown will have to be handled in the task
    /// \param inTask task to be executed on the main thread
    void
    callOnMainThread(std::function<void()> inTask);
} // namespace isx

#endif // def ISX_CALL_ON_MAIN_THREAD_H