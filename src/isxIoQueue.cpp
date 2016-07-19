#include "isxIoQueue.h"
#include "isxDispatchQueueWorker.h"
#include "isxAssert.h"
#include "isxMutex.h"
#include "isxConditionVariable.h"

#include <memory>
#include <queue>

namespace isx
{
std::unique_ptr<IoQueue> IoQueue::s_instance;

class IoQueue::Impl : public std::enable_shared_from_this<IoQueue::Impl>
{
    typedef std::weak_ptr<Impl>     WpImpl_t;
    typedef std::shared_ptr<Impl>   SpImpl_t;
public:
    Impl()
    {
        m_worker.reset(new DispatchQueueWorker());
    }
    
    void
    init()
    {
        WpImpl_t weakThis = shared_from_this();
        m_worker->dispatch([weakThis, this](){
            SpImpl_t sharedThis = weakThis.lock();
            if (sharedThis)
            {
                m_taskQueueCV.notifyOne();
                m_taskQueueMutex.lock("worker impl");
                while (1)
                {
                    while (!m_taskQueue.empty())
                    {
                        Task t = m_taskQueue.front();
                        m_taskQueue.pop();
                        m_taskQueueMutex.unlock();
                        AsyncTaskFinishedStatus status = AsyncTaskFinishedStatus::COMPLETE;
                        try
                        {
                            t.m_task();
                        }
                        catch(...)
                        {
                            status = AsyncTaskFinishedStatus::ERROR_EXCEPTION;
                        }
                        t.m_finishedCB(status);
                        m_taskQueueMutex.lock("worker impl");
                    }
                    m_taskQueueCV.wait(m_taskQueueMutex);
                    // m_taskQueueMutex is taken
                    if (m_destroy)
                    {
                        break;
                    }
                }
                m_taskQueueMutex.unlock();
            }
        });
        
        m_taskQueueMutex.lock("wait for worker");
        bool didNotTimeout = m_taskQueueCV.waitForMs(m_taskQueueMutex, 250);
        m_taskQueueMutex.unlock();
        ISX_ASSERT(didNotTimeout);
    }

    void
    destroy()
    {
        {
            ScopedMutex locker(m_taskQueueMutex, "destroy");
            m_destroy = true;
        }
        m_taskQueueCV.notifyOne();
        m_worker->destroy();
    }

    void
    enqueue(Task inTask)
    {
        {
            ScopedMutex locker(m_taskQueueMutex, "enqueue");
            m_taskQueue.push(inTask);
        }
        m_taskQueueCV.notifyOne();
    }

private:
    UpDispatchQueueWorker_t  m_worker;
    std::queue<Task>         m_taskQueue;
    Mutex                    m_taskQueueMutex;
    ConditionVariable        m_taskQueueCV;
    bool                     m_destroy = false;
};

IoQueue::IoQueue()
{
    m_pImpl.reset(new Impl());
    m_pImpl->init();
}

IoQueue::~IoQueue()
{
    m_pImpl.reset();
}

void
IoQueue::initialize()
{
    if (!isInitialized())
    {
        s_instance.reset(new IoQueue());
    }
}

void
IoQueue::destroy()
{
    if (isInitialized())
    {
        s_instance->m_pImpl->destroy();
        s_instance.reset();
    }
}

bool 
IoQueue::isInitialized()
{
    return s_instance != nullptr;
}

IoQueue *
IoQueue::instance()
{
    ISX_ASSERT(isInitialized());
    if (isInitialized())
    {
        return s_instance.get();
    }
    return 0;
}

void
IoQueue::enqueue(Task inTask)
{
    if (isInitialized())
    {
        m_pImpl->enqueue(inTask);
    }
}

} // namespace isx
