#include "isxDispatchQueue.h"
#include "isxDispatchQueueMain.h"
#include "isxDispatchQueueDispatcher.h"
#include <QCoreApplication>
#include <QThreadPool>

#include "isxAssert.h"

namespace isx
{

DispatchQueueMain::DispatchQueueMain()
: m_dispatcher(new DispatchQueueDispatcher())
{
    // if possible, verify that we're called on main thread
    if (QCoreApplication::instance())
    {
        ISX_ASSERT(QCoreApplication::instance()->thread() == QThread::currentThread());
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
