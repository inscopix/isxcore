#include "isxDispatchQueue.h"
#include "isxDispatchQueueDispatcher.h"
#include "isxLog.h"

#include <QObject>

// these are needed by Qt so it can queue Task_t objects in its queues between threads
Q_DECLARE_METATYPE(isx::DispatchQueueInterface::Task_t);
Q_DECLARE_METATYPE(isx::DispatchQueueInterface::ContextTask_t);

namespace isx
{
DispatchQueueDispatcher::DispatchQueueDispatcher()
{
    // these are needed by Qt so it can queue Task_t objects in its queues between threads
    int task_id = qRegisterMetaType<DispatchQueueInterface::Task_t>("Task_t");
    int contextTask_id = qRegisterMetaType<DispatchQueueInterface::ContextTask_t>("ContextTask_t");
    ISX_LOG_DEBUG("DispatchQueueDispatcher task_id: ", task_id, ", contextTask_id: ", contextTask_id);
    ISX_LOG_DEBUG("DispatchQueueDispatcher typename(", task_id, "): ", QMetaType::typeName(task_id));
    ISX_LOG_DEBUG("DispatchQueueDispatcher typename(", contextTask_id, "): ", QMetaType::typeName(contextTask_id));

    QObject::connect(this, &DispatchQueueDispatcher::dispatch,
                     this, &DispatchQueueDispatcher::process);
    QObject::connect(this, &DispatchQueueDispatcher::dispatchWithContext,
                     this, &DispatchQueueDispatcher::processWithContext);
}

void 
DispatchQueueDispatcher::process(DispatchQueueInterface::Task_t inTask)
{
    inTask();
}

void 
DispatchQueueDispatcher::processWithContext(void * inContext, DispatchQueueInterface::ContextTask_t inContextTask)
{
    inContextTask(inContext);
}

} // namespace isx
