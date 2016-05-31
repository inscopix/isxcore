#ifndef ISX_DISPATCH_QUEUE_DISPATCHER_H
#define ISX_DISPATCH_QUEUE_DISPATCHER_H

#include "isxDispatchQueue.h"
#include "isxLog.h"

#include <QObject>


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
    process(DispatchQueueInterface::Task_t inTask);

    /// process a task with a context on this dispatcher's thread
    /// \param inContext the context to be passed in to the task
    /// \param inContexTask_t the task to be processed
    void
    processWithContext(void * inContext, DispatchQueueInterface::ContextTask_t inContexTask_t);
    
signals:
    /// dispatch a task to this dispatcher's thread
    /// \param inTask the task to be processed
    ///
    void
    dispatch(DispatchQueueInterface::Task_t inTask);
    
    /// dispatch a task with a context to this dispatcher's thread
    /// \param inContext the context to be passed in to the task
    /// \param inContexTask_t the task to be processed
    void
    dispatchWithContext(void * inContext, DispatchQueueInterface::ContextTask_t inContexTask_t);
private:
    DispatchQueueDispatcher(const DispatchQueueDispatcher & inOther) = delete;
    const DispatchQueueDispatcher & operator=(const DispatchQueueDispatcher & inOther) = delete;
};
} // namespace isx

#endif // def ISX_DISPATCH_QUEUE_DISPATCHER_H
