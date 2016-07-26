#include "isxCallOnMainThread.h"
#include "isxDispatchQueue.h"


namespace isx
{
    void
    callOnMainThread(std::function<void()> inTask)
    {
        DispatchQueue::mainQueue()->dispatch(inTask);
    }
} // namespace isx
