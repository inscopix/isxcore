#ifndef ISX_DISPATCH_QUEUE_H
#define ISX_DISPATCH_QUEUE_H

#include "isxCoreFwd.h"

#include <functional>
#include <memory>

namespace isx
{
    
// NOTE: aschildan 5/31/2016: Ideally these would be in DispatchQueueInterface but
// I get the following when I try to register them with the QMetaType system
// via qRegisterMetaType:
// ""
// QObject::connect: Cannot queue arguments of type 'DispatchQueueInterface::ContextTask_t'
// (Make sure 'DispatchQueueInterface::ContextTask_t' is registered using qRegisterMetaType().)
// ""
    
/// type of task dispatched into queue for processing
///
typedef std::function<void()> Task_t;

/// type of task with context dispatched into queue for processing
///
typedef std::function<void(void *)> ContextTask_t;

///
/// An interface class defining a DispatchQueue API.
///
class DispatchQueueInterface
{
public:
    /// Destructor
    ///
    virtual
    ~DispatchQueueInterface() = default;

    /// dispatch a task into this queue for processing
    /// \param inTask the task to be processed
    ///
    virtual
    void
    dispatch(Task_t inTask) = 0;
    
    /// dispatch a task with context into this queue for processing
    /// \param inContext passed into the task function at processing time
    /// \param inContextTask the task accepting a context to be processed
    ///
    virtual
    void
    dispatch(void * inContext, ContextTask_t inContextTask) = 0;
};


/// Container for Pool DispatchQueue & Main DispatchQueue singletons
///
class DispatchQueue
{
public:
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
    SpDispatchQueueInterface_t
    mainQueue();

    /// \return the thread pool dispatch queue
    ///
    static
    SpDispatchQueueInterface_t
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

    static SpDispatchQueueInterface_t s_pool;
    static SpDispatchQueueInterface_t s_main;
    static bool s_isInitialized;
};

} // namespace isx

#endif // ISX_DISPATCH_QUEUE_H
