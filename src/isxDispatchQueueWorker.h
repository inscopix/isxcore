#ifndef ISX_DISPATCH_QUEUE_WORKER_H
#define ISX_DISPATCH_QUEUE_WORKER_H

#include "isxDispatchQueue.h"
#include <memory>

namespace isx
{

class DispatchQueueDispatcher;


/// A class implementing DispatchQueueInterface for a single worker thread
/// that it maintains.
/// Use this for example for doing I/O.
///
class DispatchQueueWorker : public DispatchQueueInterface
{
public:
    /// Constructor
    ///
    DispatchQueueWorker();

    /// Destructor
    ///
    ~DispatchQueueWorker() override;

    /// dispatch a task into this queue for processing
    /// \param inTask the task to be processed
    ///
    void
    dispatch(DispatchQueueTask_t inTask) override;
    
    /// dispatch a task with context into this queue for processing
    /// \param inContext passed into the task function at processing time
    /// \param inContextTask the task accepting a context to be processed
    ///
    void
    dispatch(void * inContext, DispatchQueueContextTask_t inContextTask) override;

    /// destroys this worker, tries to destroy thread which may fail
    /// call destroy before calling desctructor
    ///
    void destroy();

private:
    static const int32_t m_numThreadedRetries = 100;
    static const int32_t m_numThreadedWaitMs = 4;
    DispatchQueueWorker(const DispatchQueueWorker & inOther) = delete;
    const DispatchQueueWorker & operator=(const DispatchQueueWorker & inOther) = delete;

    class WorkerThread;
    std::unique_ptr<WorkerThread> m_worker;
    std::shared_ptr<DispatchQueueDispatcher> m_dispatcher;
};
} // namespace isx


#endif // def ISX_DISPATCH_QUEUE_WORKER_H
