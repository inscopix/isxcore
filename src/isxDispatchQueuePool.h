#ifndef ISX_DISPATCH_QUEUE_POOL_H
#define ISX_DISPATCH_QUEUE_POOL_H

#include "isxDispatchQueue.h"

namespace isx
{

/// A class implementing DispatchQueueInterface with a Thread pool
/// Use this as the default queue for compute-heavy asynchronous tasks
///
class DispatchQueuePool : public DispatchQueueInterface
{
public:
    /// Constructor
    ///
    DispatchQueuePool();

    /// dispatch a task into this queue for processing
    /// \param inTask the task to be processed
    ///
    virtual
    void
    dispatch(Task_t inTask);
    
    /// dispatch a task with context into this queue for processing
    /// \param inContext passed into the task function at processing time
    /// \param inContextTask the task accepting a context to be processed
    ///
    virtual
    void
    dispatch(void * inContext, ContextTask_t inContextTask);
private:
    DispatchQueuePool(const DispatchQueuePool & inOther) = delete;
    const DispatchQueuePool & operator=(const DispatchQueuePool & inOther) = delete;
};
} // namespace isx
#endif // def ISX_DISPATCH_QUEUE_POOL_H
