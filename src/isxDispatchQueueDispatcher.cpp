#include "isxDispatchQueue.h"
#include "isxDispatchQueueDispatcher.h"
#include "isxLog.h"

#include <QObject>

namespace isx
{
DispatchQueueDispatcher::DispatchQueueDispatcher()
{
    // these are needed by Qt so it can queue Task_t objects in its queues between threads
    qRegisterMetaType<DispatchQueueTask_t>("DispatchQueueTask_t");
    qRegisterMetaType<DispatchQueueContextTask_t>("DispatchQueueContextTask_t");

    QObject::connect(this, &DispatchQueueDispatcher::dispatch,
                     this, &DispatchQueueDispatcher::process);
    QObject::connect(this, &DispatchQueueDispatcher::dispatchWithContext,
                     this, &DispatchQueueDispatcher::processWithContext);
}

void 
DispatchQueueDispatcher::process(DispatchQueueTask_t inTask)
{
    inTask();
}

void 
DispatchQueueDispatcher::processWithContext(void * inContext, DispatchQueueContextTask_t inContextTask)
{
    inContextTask(inContext);
}

} // namespace isx
