#include "isxDispatchQueuePool.h"

#include <QThreadPool>

namespace isx
{
namespace 
{
class TaskWrapper : public QRunnable
{
public:
    explicit TaskWrapper(DispatchQueue::tTask && inTask)
    : m_Task(std::move(inTask))
    {}

    void run()
    {
        m_Task();
    }

private:
    DispatchQueue::tTask m_Task;
};

} // namespace

DispatchQueuePool::DispatchQueuePool()
{}
    
void
DispatchQueuePool::dispatch(tTask inTask)
{
    // note: QRunnable::autoDelete is true by default,
    //       this means QThreadPool will delete the QRunnable
    //       object automatically
    TaskWrapper * tw = new TaskWrapper(std::move(inTask));
    QThreadPool::globalInstance()->start(tw);
}
    
void
DispatchQueuePool::dispatch(void * inContext, tContextTask inContextTask)
{
    // note: QRunnable::autoDelete is true by default,
    //       this means QThreadPool will delete the QRunnable
    //       object automatically
    TaskWrapper * tw = new TaskWrapper([=]()
    {
        inTask(inContext);
    });
    QThreadPool::globalInstance()->start(tw);
}

} // namespace isx

#endif // def ISX_DISPATCH_QUEUE_POOL_H
