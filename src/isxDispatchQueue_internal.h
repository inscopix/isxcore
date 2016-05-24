#ifndef ISX_DISPATCH_QUEUE_INTERNAL
#define ISX_DISPATCH_QUEUE_INTERNAL

#include "isxDispatchQueue.h"
#include <QObject>

namespace isx
{

/// class to handle processing dispatched tasks on main thread
class DispatchQueue::MainThreadObject : public QObject
{
    Q_OBJECT
    
public:
    /// Constructor
    ///
    MainThreadObject();
    
    /// process a task on the main thread
    /// \param inTask the task to be processed
    ///
    void
    processOnMain(tTask inTask);

    /// process a task with a context on the main thread
    /// \param inContext the context to be passed in to the task
    /// \param inContextTask the task to be processed
    void
    processOnMainWithContext(void * inContext, tContextTask inContextTask);
    
signals:
    /// dispatch a task to the main thread
    /// \param inTask the task to be processed
    ///
    void
    dispatchToMain(tTask inTask);
    
    /// dispatch a task with a context to the main thread
    /// \param inContext the context to be passed in to the task
    /// \param inContextTask the task to be processed
    void
    dispatchToMainWithContext(void * inContext, tContextTask inContextTask);
    
};

} // namespace isx

#endif // def ISX_DISPATCH_QUEUE_INTERNAL