#ifndef ISX_DISPATCH_QUEUE_MAIN_H
#define ISX_DISPATCH_QUEUE_MAIN_H

#include "isxDispatchQueue.h"

namespace isx
{

class DispatchQueueDispatcher;

/// A class implementing DispatchQueueInterface for the main thread
/// We will probably not use this as we plan to use Qt signals and slots
/// to communicate back to the main thread
///
class DispatchQueueMain : public DispatchQueueInterface
{
public:
    /// Constructor
    ///
    DispatchQueueMain();

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
private:
    DispatchQueueMain(const DispatchQueueMain & inOther) = delete;
    const DispatchQueueMain & operator=(const DispatchQueueMain & inOther) = delete;

    std::shared_ptr<DispatchQueueDispatcher> m_dispatcher;
};
} // namespace isx


#endif // def ISX_DISPATCH_QUEUE_MAIN_H
