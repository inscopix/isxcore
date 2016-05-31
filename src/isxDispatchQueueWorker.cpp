#include "isxDispatchQueue.h"
#include "isxDispatchQueueWorker.h"
#include "isxDispatchQueueDispatcher.h"
#include <QObject>
#include <QThread>

#include <chrono>
#include <thread>
#include <assert.h>

// these are needed by Qt so it can queue Task_t objects in its queues between threads
Q_DECLARE_METATYPE(isx::DispatchQueueInterface::Task_t);
Q_DECLARE_METATYPE(isx::DispatchQueueInterface::ContextTask_t);

namespace isx
{

class DispatchQueueWorker::WorkerThread : public QThread
{
    Q_OBJECT

public:
#if 0    
    /// constructor
    ///
    WorkerThread()
    {
    }
#endif
    /// Destructor
    ///
    ~WorkerThread(){}

    /// accessor for dispatcher object
    /// \ return shared_ptr to dispatcher object
    ///
    std::shared_ptr<DispatchQueueDispatcher> dispatcher()
    {
        return m_dispatcher;
    }

    /// run method
    ///
    void run() Q_DECL_OVERRIDE
    {
        m_dispatcher.reset(new DispatchQueueDispatcher());
        exec();
    }

    /// destroys this worker
    ///
    void destroy()
    {
        exit();
        for (int i = 0; i < 100; ++i)
        {
            if (isFinished())
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        assert(isFinished());
        if (!isFinished())
        {
            ISX_LOG_ERROR("Worker thread failed to finish");
            //m_pWorkerThread->terminate();
        }
    }
private:
    std::shared_ptr<DispatchQueueDispatcher> m_dispatcher;
};

DispatchQueueWorker::DispatchQueueWorker()
{
    // these are needed by Qt so it can queue Task_t objects in its queues between threads
    int task_id = qRegisterMetaType<DispatchQueueInterface::Task_t>("Task_t");
    int contextTask_id = qRegisterMetaType<DispatchQueueInterface::ContextTask_t>("ContextTask_t");
    ISX_LOG_DEBUG("DispatchQueueWorker task_id: ", task_id, ", contextTask_id: ", contextTask_id);
    ISX_LOG_DEBUG("DispatchQueueWorker typename(", task_id, "): ", QMetaType::typeName(task_id));
    ISX_LOG_DEBUG("DispatchQueueWorker typename(", contextTask_id, "): ", QMetaType::typeName(contextTask_id));
    
    m_worker.reset(new WorkerThread());
    m_worker->start();
    for (int i = 0; i < 100; ++i)
    {
        m_dispatcher = m_worker->dispatcher();
        if (m_dispatcher)
        {
            break;
        }
        std::chrono::milliseconds d(2);
        std::this_thread::sleep_for(d);
    }
    assert(m_dispatcher);
    if (!m_dispatcher)
    {
        ISX_LOG_ERROR("Error: Worker thread did not provide dispatcher.");
    }
}

DispatchQueueWorker::~DispatchQueueWorker(){}

void 
DispatchQueueWorker::destroy()
{
    m_worker->destroy();
}

void
DispatchQueueWorker::dispatch(Task_t inTask)
{
    emit m_dispatcher->dispatch(inTask);
}
    
void
DispatchQueueWorker::dispatch(void * inContext, ContextTask_t inContextTask)
{
    emit m_dispatcher->dispatchWithContext(inContext, inContextTask);
}

} // namespace isx
#include "isxDispatchQueueWorker.moc"
