#include "isxDispatchQueue.h"
#include "isxDispatchQueue_internal.h"
#include "isxLog.h"

#include <QObject>
#include <QThreadPool>
#include <QApplication>

#include <chrono>
#include <thread>

#include <assert.h>

// these are needed by Qt so it can queue tTask objects in its queues between threads
Q_DECLARE_METATYPE(isx::DispatchQueue::tTask);
Q_DECLARE_METATYPE(isx::DispatchQueue::tContextTask);

namespace isx
{
    
class TaskWrapper : public QRunnable
{
public:
    explicit TaskWrapper(DispatchQueue::tTask && inTask)
    : m_Task(std::move(inTask))
    {}

    void run()
    {
        m_Task();
    }

private:
    DispatchQueue::tTask m_Task;
};
    
DispatchQueue::Dispatcher::Dispatcher()
{
    // these are needed by Qt so it can queue tTask objects in its queues between threads
    qRegisterMetaType<tTask>("tTask");
    qRegisterMetaType<tContextTask>("tContextTask");
    
    QObject::connect(this, &Dispatcher::dispatch,
                     this, &Dispatcher::process);
    QObject::connect(this, &Dispatcher::dispatchWithContext,
                     this, &Dispatcher::processWithContext);
}

void 
DispatchQueue::Dispatcher::process(tTask inTask)
{
    inTask();
}

void 
DispatchQueue::Dispatcher::processWithContext(void * inContext, tContextTask inContextTask)
{
    inContextTask(inContext);
}
   
tDispatchQueue_SP DispatchQueue::m_Pool;
tDispatchQueue_SP DispatchQueue::m_Main;
bool DispatchQueue::m_IsInitialized;

DispatchQueue::~DispatchQueue()
{
}

void 
DispatchQueue::initializeDefaultQueues()
{
    m_Pool.reset(new DispatchQueue(kPOOL));
    m_Main.reset(new DispatchQueue(kMAIN));
    m_IsInitialized = true;
}

bool 
DispatchQueue::isInitialized()
{
    return m_IsInitialized;
}

void 
DispatchQueue::destroyDefaultQueues()
{
    m_Pool.reset();
    m_Main.reset();
    m_IsInitialized = false;
}

tDispatchQueue_SP
DispatchQueue::create()
{
    return tDispatchQueue_SP(new DispatchQueue(kSINGLE_THREADED_WORKER));
}

DispatchQueue::DispatchQueue(eType inType)
{
    if (inType == kPOOL)
    {
        // don't create thread object -> dispatch methods will use ThreadPool
    }
    else if (inType == kMAIN)
    {
        // if possible, verify that we're called on main thread
        if (QApplication::instance())
        {
            assert(QApplication::instance()->thread() == QThread::currentThread());
        }
        m_pDispatcher.reset(new Dispatcher());
    }
    else if (inType == kSINGLE_THREADED_WORKER)
    {
        m_pWorkerThread.reset(new WorkerThread());
        m_pWorkerThread->start();
        for (int i = 0; i < 100; ++i)
        {
            m_pDispatcher = m_pWorkerThread->dispatcher();
            if (m_pDispatcher)
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        assert(m_pDispatcher);
        if (!m_pDispatcher)
        {
            ISX_LOG_ERROR("Error: Worker thread did not provide dispatcher.");
        }
    }
}

tDispatchQueue_SP
DispatchQueue::poolQueue()
{
    return m_Pool;
}

tDispatchQueue_SP
DispatchQueue::mainQueue()
{
    return m_Main;
}

void
DispatchQueue::dispatch(tTask inTask)
{
    if (m_pDispatcher)
    {
        emit m_pDispatcher->dispatch(inTask);
    }
    else
    {
        // since we only support the default queue we can
        // hard-code to use global QThreadPool for now
        TaskWrapper * tw = new TaskWrapper(std::move(inTask));
        QThreadPool::globalInstance()->start(tw);
    }
}

void
DispatchQueue::dispatch(void * inContext, tContextTask inTask)
{
    if (m_pDispatcher)
    {
        emit m_pDispatcher->dispatchWithContext(inContext, inTask);
    }
    else
    {
        // since we only support the default queue we can
        // hard-code to use global QThreadPool for now
        TaskWrapper * tw = new TaskWrapper([=]()
        {
            inTask(inContext);
        });
        QThreadPool::globalInstance()->start(tw);
    }
}

} // namespace isx


