#include "isxDispatchQueue.h"
#include "isxDispatchQueueDispatcher.h"
#include "isxLog.h"

#include <QObject>

namespace isx
{
DispatchQueueDispatcher::DispatchQueueDispatcher()
{
    // these are needed by Qt so it can queue Task_t objects in its queues between threads
    qRegisterMetaType<Task_t>("Task_t");
    qRegisterMetaType<ContextTask_t>("ContextTask_t");

    QObject::connect(this, &DispatchQueueDispatcher::dispatch,
                     this, &DispatchQueueDispatcher::process);
    QObject::connect(this, &DispatchQueueDispatcher::dispatchWithContext,
                     this, &DispatchQueueDispatcher::processWithContext);
}

void 
DispatchQueueDispatcher::process(Task_t inTask)
{
    inTask();
}

void 
DispatchQueueDispatcher::processWithContext(void * inContext, ContextTask_t inContextTask)
{
    inContextTask(inContext);
}

} // namespace isx
