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
    
    // this dispatches a single task that runs until m_destroy gets set from the
    // outside (via the destroy() method)
    // the tasks waits on m_taskQueueCV, when that gets notified (in enqueue() or destroy())
    // it processes m_taskQueue until it is empty.
    void
    init()
    {
        WpImpl_t weakThis = shared_from_this();
        m_worker->dispatch([weakThis, this](){
            SpImpl_t sharedThis = weakThis.lock();
            if (sharedThis)
            {
                m_taskQueueMutex.lock("worker impl");
                while (1)
                {
                    while (!m_taskQueue.empty())    // under lock, so enqueue can't push onto queue
                    {
                        SpIoTask_t t = m_taskQueue.front();
                        m_taskQueue.pop();
                        m_taskQueueMutex.unlock();
                        if (m_destroy || t->isCancelPending())
                        {
                            t->setTaskStatus(AsyncTaskStatus::CANCELLED);
                            t->getFinishedCB()(t->getTaskStatus());
                        }
                        else
                        {
                            t->setTaskStatus(AsyncTaskStatus::PROCESSING);
                            try
                            {
                                t->getTask()();             // execute without holding lock, enqueue can push onto queue
                                t->setTaskStatus(AsyncTaskStatus::COMPLETE);
                            }
                            catch(...)
                            {
                                t->setExceptionPtr(std::current_exception());
                                t->setTaskStatus(AsyncTaskStatus::ERROR_EXCEPTION);
                            }
                            t->getFinishedCB()(t->getTaskStatus());
                        }
                        m_taskQueueMutex.lock("worker impl");
                    }
                    if (m_destroy)
                    {
                        break;
                    }
                    m_taskQueueCV.wait(m_taskQueueMutex);
                    // m_taskQueueMutex is taken
                }
                m_taskQueueMutex.unlock();
                m_taskQueueCV.notifyOne();
            }
        });
    }

    void
    destroy()
    {
        {
            ScopedMutex locker(m_taskQueueMutex, "destroy");
            m_destroy = true;
        }
        m_taskQueueCV.notifyOne();
        
        // now wait for thread to respond to destroy
        m_taskQueueMutex.lock("destroy wait");
        m_taskQueueCV.waitForMs(m_taskQueueMutex, 250);
        m_taskQueueMutex.unlock();

        m_worker->destroy();
    }

    void
    enqueue(SpIoTask_t inTask)
    {
        {
            ScopedMutex locker(m_taskQueueMutex, "enqueue");
            m_taskQueue.push(inTask);
        }
        m_taskQueueCV.notifyOne();
    }

private:
    UpDispatchQueueWorker_t  m_worker;
    std::queue<SpIoTask_t>   m_taskQueue;
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
    if (isInitialized())
    {
        return s_instance.get();
    }
    return 0;
}

void
IoQueue::enqueue(SpIoTask_t inTask)
{
    if (isInitialized())
    {
        m_pImpl->enqueue(inTask);
    }
}

} // namespace isx
