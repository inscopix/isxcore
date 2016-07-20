#ifndef ISX_DISPATCH_QUEUE_DISPATCHER_H
#define ISX_DISPATCH_QUEUE_DISPATCHER_H

#include "isxDispatchQueue.h"
#include "isxLog.h"

#include <QObject>

// these are needed by Qt so it can queue Task_t objects in its queues between threads
Q_DECLARE_METATYPE(isx::DispatchQueueTask_t);
Q_DECLARE_METATYPE(isx::DispatchQueueContextTask_t);

namespace isx
{

/// class to handle processing dispatched tasks on main or worker thread
class DispatchQueueDispatcher : public QObject
{
    Q_OBJECT
    
public:
    /// Constructor
    ///
    DispatchQueueDispatcher();
    
    /// process a task on this dispatcher's thread 
    /// \param inTask the task to be processed
    ///
    void
    process(DispatchQueueTask_t inTask);

    /// process a task with a context on this dispatcher's thread
    /// \param inContext the context to be passed in to the task
    /// \param inContextTask_t the task to be processed
    void
    processWithContext(void * inContext, DispatchQueueContextTask_t inContextTask_t);
    
signals:
    /// dispatch a task to this dispatcher's thread
    /// \param inTask the task to be processed
    ///
    void
    dispatch(DispatchQueueTask_t inTask);
    
    /// dispatch a task with a context to this dispatcher's thread
    /// \param inContext the context to be passed in to the task
    /// \param inContextTask_t the task to be processed
    void
    dispatchWithContext(void * inContext, DispatchQueueContextTask_t inContextTask_t);
private:
    DispatchQueueDispatcher(const DispatchQueueDispatcher & inOther) = delete;
    const DispatchQueueDispatcher & operator=(const DispatchQueueDispatcher & inOther) = delete;
};
} // namespace isx

#endif // def ISX_DISPATCH_QUEUE_DISPATCHER_H
