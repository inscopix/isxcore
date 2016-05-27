#ifndef ISX_DISPATCH_QUEUE_INTERNAL
#define ISX_DISPATCH_QUEUE_INTERNAL

#include "isxDispatchQueue.h"
#include "isxLog.h"

#include <QObject>
#include <QThread>

#include <chrono>
#include <thread>

#include <assert.h>

namespace isx
{

/// class to handle processing dispatched tasks on main thread
class DispatchQueue::Dispatcher : public QObject
{
    Q_OBJECT
    
public:
    /// Constructor
    ///
    Dispatcher();
    
    /// process a task on this dispatcher's thread 
    /// \param inTask the task to be processed
    ///
    void
    process(tTask inTask);

    /// process a task with a context on this dispatcher's thread
    /// \param inContext the context to be passed in to the task
    /// \param inContextTask the task to be processed
    void
    processWithContext(void * inContext, tContextTask inContextTask);
    
signals:
    /// dispatch a task to this dispatcher's thread
    /// \param inTask the task to be processed
    ///
    void
    dispatch(tTask inTask);
    
    /// dispatch a task with a context to this dispatcher's thread
    /// \param inContext the context to be passed in to the task
    /// \param inContextTask the task to be processed
    void
    dispatchWithContext(void * inContext, tContextTask inContextTask);
};

/// class for a worker thread
///
class DispatchQueue::WorkerThread : public QThread
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

    /// accessor for dispatcher object
    /// \ return shared_ptr to dispatcher object
    ///
    std::shared_ptr<Dispatcher> dispatcher()
    {
        return m_pDispatcher;
    }

    /// run method
    ///
    void run() Q_DECL_OVERRIDE
    {
        m_pDispatcher.reset(new Dispatcher());
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
    std::shared_ptr<Dispatcher> m_pDispatcher;
};
} // namespace isx

#endif // def ISX_DISPATCH_QUEUE_INTERNAL