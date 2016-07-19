#include "isxDispatchQueue.h"
#include "isxDispatchQueueWorker.h"
#include "isxDispatchQueueDispatcher.h"
#include <QObject>
#include <QThread>

#include <chrono>
#include <thread>

#include "isxAssert.h"

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
        for (int32_t i = 0; i < m_numThreadedRetries; ++i)
        {
            if (isFinished())
            {
                break;
            }
            std::chrono::milliseconds d(m_numThreadedWaitMs);
            std::this_thread::sleep_for(d);
        }
        ISX_ASSERT(isFinished(), "Worker thread failed to finish.");
        //if (!isFinished())
        //{
        //    m_pWorkerThread->terminate();
        //}
    }
private:
    std::shared_ptr<DispatchQueueDispatcher> m_dispatcher;
};

DispatchQueueWorker::DispatchQueueWorker()
{
    m_worker.reset(new WorkerThread());
    m_worker->start();
    for (int32_t i = 0; i < m_numThreadedRetries; ++i)
    {
        m_dispatcher = m_worker->dispatcher();
        if (m_dispatcher)
        {
            break;
        }
        std::chrono::milliseconds d(m_numThreadedWaitMs);
        std::this_thread::sleep_for(d);
    }
    ISX_ASSERT(m_dispatcher, "Worker thread did not provide dispatcher.");
}

DispatchQueueWorker::~DispatchQueueWorker(){}

void 
DispatchQueueWorker::destroy()
{
    m_dispatcher.reset();
    m_worker->destroy();
}

void
DispatchQueueWorker::dispatch(DispatchQueueTask_t inTask)
{
    ISX_ASSERT(m_dispatcher, "Tried to dispatch task after worker was destroyed.");
    if (m_dispatcher)
    {
        emit m_dispatcher->dispatch(inTask);
    }
}
    
void
DispatchQueueWorker::dispatch(void * inContext, DispatchQueueContextTask_t inContextTask)
{
    ISX_ASSERT(m_dispatcher, "Tried to dispatch task after worker was destroyed.");
    if (m_dispatcher)
    {
        emit m_dispatcher->dispatchWithContext(inContext, inContextTask);
    }
}

} // namespace isx
#include "isxDispatchQueueWorker.moc"
