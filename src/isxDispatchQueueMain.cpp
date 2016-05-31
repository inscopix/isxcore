#include "isxDispatchQueue.h"
#include "isxDispatchQueueMain.h"
#include "isxDispatchQueueDispatcher.h"
#include <QApplication>
#include <QThreadPool>
#include <assert.h>

// these are needed by Qt so it can queue Task_t objects in its queues between threads
Q_DECLARE_METATYPE(isx::DispatchQueueInterface::Task_t);
Q_DECLARE_METATYPE(isx::DispatchQueueInterface::ContextTask_t);

namespace isx
{

DispatchQueueMain::DispatchQueueMain()
: m_dispatcher(new DispatchQueueDispatcher())
{
    // if possible, verify that we're called on main thread
    if (QApplication::instance())
    {
        assert(QApplication::instance()->thread() == QThread::currentThread());
    }
}

void
DispatchQueueMain::dispatch(Task_t inTask)
{
    emit m_dispatcher->dispatch(inTask);
}
    
void
DispatchQueueMain::dispatch(void * inContext, ContextTask_t inContextTask)
{
    emit m_dispatcher->dispatchWithContext(inContext, inContextTask);
}

} // namespace isx
