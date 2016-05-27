#ifndef ISX_DISPATCH_QUEUE_POOL_H
#define ISX_DISPATCH_QUEUE_POOL_H

#include "isxDispatchQueue.h"


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
    dispatch(tTask inTask);
    
    /// dispatch a task with context into this queue for processing
    /// \param inContext passed into the task function at processing time
    /// \param inContextTask the task accepting a context to be processed
    ///
    virtual
    void
    dispatch(void * inContext, tContextTask inContextTask);

};

#endif // def ISX_DISPATCH_QUEUE_POOL_H
