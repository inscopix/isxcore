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
    /// factory method to create a new dispatch queue with
    /// its own single worker thread
    ///
    static tDispatchQueue_SP
    create();

    /// Destructor
    ///
    ~DispatchQueue();

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

    /// type of task dispatched into queue for processing
    ///
    typedef std::function<void()> tTask;
    
    /// type of task with context dispatched into queue for processing
    ///
    typedef std::function<void(void *)> tContextTask;

    /// \return the main thread dispatch queue
    ///
    static tDispatchQueue_SP
    mainQueue();

    /// \return the thread pool dispatch queue
    ///
    static tDispatchQueue_SP
    poolQueue();

    /// dispatch a task into this queue for processing
    /// \param inTask the task to be processed
    ///
    void
    dispatch(tTask inTask);
    
    /// dispatch a task with context into this queue for processing
    /// \param inContext passed into the task function at processing time
    /// \param inContextTask the task accepting a context to be processed
    ///
    void
    dispatch(void * inContext, tContextTask inContextTask);

private:
    enum eType
    {
        kSINGLE_THREADED_WORKER,
        kPOOL,
        kMAIN
    };

    /// Default constructor
    ///
    explicit DispatchQueue(eType inType);

    /// Copy constructor
    /// private (don't make copies of DispatchQueue objects)
    ///
    DispatchQueue(const DispatchQueue &);

    /// Assignment operator
    /// private (don't assign DispatchQueue objects)
    ///
    DispatchQueue & operator=(const DispatchQueue &);

    static tDispatchQueue_SP m_Pool;
    static tDispatchQueue_SP m_Main;
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