#ifndef ISX_DISPATCH_QUEUE
#define ISX_DISPATCH_QUEUE

#include "isxCoreFwd.h"

#include <functional>
#include <memory>

namespace isx
{

///
/// An interface class defining a DispatchQueue API.
///

class DispatchQueueInterface
{
public:
    /// Destructor
    ///
    virtual
    ~DispatchQueueInterface();

    /// dispatch a task into this queue for processing
    /// \param inTask the task to be processed
    ///
    virtual
    void
    dispatch(tTask inTask) = 0;
    
    /// dispatch a task with context into this queue for processing
    /// \param inContext passed into the task function at processing time
    /// \param inContextTask the task accepting a context to be processed
    ///
    virtual
    void
    dispatch(void * inContext, tContextTask inContextTask) = 0;
};


/// Container for Pool DispatchQueue & Main DispatchQueue singletons
///
class DispatchQueue
{
public:
    /// type of task dispatched into queue for processing
    ///
    typedef std::function<void()> tTask;
    
    /// type of task with context dispatched into queue for processing
    ///
    typedef std::function<void(void *)> tContextTask;

    /// initialize the default queues (main and pool).
    /// Note: Must be called on main threaad.
    ///
    static
    void 
    initializeDefaultQueues();

    /// accessor for static m_IsInitialized boolean
    /// true only in between initialize and destroy calls
    ///
    static
    bool 
    isInitialized();

    /// destroy the default queues (main and pool).
    ///
    static
    void 
    destroyDefaultQueues();

    /// \return the main thread dispatch queue
    ///
    static
    tDispatchQueue_SP
    mainQueue();

    /// \return the thread pool dispatch queue
    ///
    static
    tDispatchQueue_SP
    poolQueue();

private:
    /// Default Constructor
    ///
    DispatchQueue() = delete;

    /// Copy constructor
    /// private (don't make copies of DispatchQueue objects)
    ///
    DispatchQueue(const DispatchQueue &) = delete;

    /// Assignment operator
    /// private (don't assign DispatchQueue objects)
    ///
    DispatchQueue & operator=(const DispatchQueue &) = delete;

    static tDispatchQueue_SP m_Pool;
    static tDispatchQueue_SP m_Main;
    static bool m_IsInitialized;
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