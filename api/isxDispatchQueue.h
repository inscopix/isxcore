#ifndef ISX_DISPATCH_QUEUE
#define ISX_DISPATCH_QUEUE

#include "isxCoreFwd.h"

#include <functional>
#include <memory>

namespace isx
{

///
/// A class implementing a DispatchQueue API.
///

class DispatchQueue
{
public:
    /// type of task dispatched into queue for processing
    ///
    typedef std::function<void()> Task_t;
    
    /// type of task with context dispatched into queue for processing
    ///
    typedef std::function<void(void *)> ContextTask_t;

    /// initialize the default queues (main and pool).
    /// Note: Must be called on main threaad.
    ///
    static void 
    initializeDefaultQueues();

    /// accessor for static m_IsInitialized boolean
    /// true only in between initialize and destroy calls
    ///
    static bool 
    isInitialized();

    /// destroy the default queues (main and pool).
    ///
    static void 
    destroyDefaultQueues();

    /// \return the main thread dispatch queue
    ///
    static SpDispatchQueue_t
    mainQueue();

    /// \return the thread pool dispatch queue
    ///
    static SpDispatchQueue_t
    poolQueue();

    /// factory method to create a new dispatch queue with
    /// its own single worker thread
    ///
    static SpDispatchQueue_t
    create();

    /// destroy a dispatch queue instance with a single worker thread
    /// that was created with the create method
    /// call before destructor gets called
    ///
    void destroy();

    /// Destructor
    ///
    ~DispatchQueue();

    /// dispatch a task into this queue for processing
    /// \param inTask the task to be processed
    ///
    void
    dispatch(Task_t inTask);
    
    /// dispatch a task with context into this queue for processing
    /// \param inContext passed into the task function at processing time
    /// \param inContexTask the task accepting a context to be processed
    ///
    void
    dispatch(void * inContext, ContextTask_t inContexTask);

private:
    enum QueueType
    {
        QUEUE_TYPE_SINGLE_THREADED_WORKER,
        QUEUE_TYPE_POOL,
        QUEUE_TYPE_MAIN,
    };

    /// Default Constructor
    ///
    DispatchQueue() = delete;

    /// Constructor
    ///
    explicit DispatchQueue(QueueType inType);

    /// Copy constructor
    /// private (don't make copies of DispatchQueue objects)
    ///
    DispatchQueue(const DispatchQueue &) = delete;

    /// Assignment operator
    /// private (don't assign DispatchQueue objects)
    ///
    DispatchQueue & operator=(const DispatchQueue &) = delete;

    static SpDispatchQueue_t m_Pool;
    static SpDispatchQueue_t m_Main;
    static bool m_IsInitialized;

    class Dispatcher;
    std::shared_ptr<Dispatcher> m_pDispatcher;
    class WorkerThread;
    std::unique_ptr<WorkerThread> m_pWorkerThread;
};



} // namespace isx




// place for random planning ideas
#if 0
template<typename T>
class future
{
public:
    void cancel();
    float progress();
    T get();
private:
    T r;
};

template<typename T>
class ITask
{
public:
    virtual ~ITask();
    virtual bool checkIn(float inProgress);
};

dispatch_async_f(std::function<void()> func);
future f = dispatch_async_f(std::function<void()> func);
#endif

#endif // ISX_DISPATCH_QUEUE