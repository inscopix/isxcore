#include "isxDispatchQueue.h"
#include "isxDispatchQueue_internal.h"
#include "isxLog.h"

#include <QObject>
#include <QThreadPool>
#include <QApplication>

#include <chrono>
#include <thread>

#include <assert.h>

// these are needed by Qt so it can queue Task_t objects in its queues between threads
Q_DECLARE_METATYPE(isx::DispatchQueue::Task_t);
Q_DECLARE_METATYPE(isx::DispatchQueue::ContextTask_t);

namespace isx
{
    
class TaskWrapper : public QRunnable
{
public:
    explicit TaskWrapper(DispatchQueue::Task_t && inTask)
    : m_Task(std::move(inTask))
    {}

    void run()
    {
        m_Task();
    }

private:
    DispatchQueue::Task_t m_Task;
};
    
DispatchQueue::Dispatcher::Dispatcher()
{
    // these are needed by Qt so it can queue Task_t objects in its queues between threads
    qRegisterMetaType<Task_t>("Task_t");
    qRegisterMetaType<ContextTask_t>("ContextTask_t");
    
    QObject::connect(this, &Dispatcher::dispatch,
                     this, &Dispatcher::process);
    QObject::connect(this, &Dispatcher::dispatchWithContext,
                     this, &Dispatcher::processWithContext);
}

void 
DispatchQueue::Dispatcher::process(Task_t inTask)
{
    inTask();
}

void 
DispatchQueue::Dispatcher::processWithContext(void * inContext, ContextTask_t inContexTask_t)
{
    inContexTask_t(inContext);
}
   
SpDispatchQueue_t DispatchQueue::m_Pool;
SpDispatchQueue_t DispatchQueue::m_Main;
bool DispatchQueue::m_IsInitialized;

DispatchQueue::~DispatchQueue()
{
}

void 
DispatchQueue::initializeDefaultQueues()
{
    m_Pool.reset(new DispatchQueue(QUEUE_TYPE_POOL));
    m_Main.reset(new DispatchQueue(QUEUE_TYPE_MAIN));
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

SpDispatchQueue_t
DispatchQueue::create()
{
    return SpDispatchQueue_t(new DispatchQueue(QUEUE_TYPE_SINGLE_THREADED_WORKER));
}

void
DispatchQueue::destroy()
{
    if (m_pWorkerThread)
    {
        m_pWorkerThread->destroy();
    }
}

DispatchQueue::DispatchQueue(QueueType inType)
{
    if (inType == QUEUE_TYPE_POOL)
    {
        // don't create thread object -> dispatch methods will use ThreadPool
    }
    else if (inType == QUEUE_TYPE_MAIN)
    {
        // if possible, verify that we're called on main thread
        if (QApplication::instance())
        {
            assert(QApplication::instance()->thread() == QThread::currentThread());
        }
        m_pDispatcher.reset(new Dispatcher());
    }
    else if (inType == QUEUE_TYPE_SINGLE_THREADED_WORKER)
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

SpDispatchQueue_t
DispatchQueue::poolQueue()
{
    return m_Pool;
}

SpDispatchQueue_t
DispatchQueue::mainQueue()
{
    return m_Main;
}

void
DispatchQueue::dispatch(Task_t inTask)
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
DispatchQueue::dispatch(void * inContext, ContextTask_t inContexTask)
{
    if (m_pDispatcher)
    {
        emit m_pDispatcher->dispatchWithContext(inContext, inContexTask);
    }
    else
    {
        // since we only support the default queue we can
        // hard-code to use global QThreadPool for now
        TaskWrapper * tw = new TaskWrapper([=]()
        {
            inContexTask(inContext);
        });
        QThreadPool::globalInstance()->start(tw);
    }
}

} // namespace isx


