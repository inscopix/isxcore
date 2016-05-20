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
    /// Default constructor
    ///
    DispatchQueue();

    /// type of task dispatched into queue for processing
    ///
    typedef std::function<void()> tTask;
    
    /// type of task with context dispatched into queue for processing
    ///
    typedef std::function<void(void *)> tContextTask;

    /// \return the default dispatch queue
    ///
    static DispatchQueue & defaultQueue();

    /// dispatch a task into this queue for processing
    /// \param inWork the task to be processed
    ///
    void dispatch(tTask inWork);
    
    /// dispatch a task with contextinto this queue for processing
    /// \param inContext passed into the task function at processing time
    /// \param inWork the task accepting a context to be processed
    ///
    void dispatch(void * inContext, tContextTask inWork);


private:
    static DispatchQueue m_DefaultQueue;
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