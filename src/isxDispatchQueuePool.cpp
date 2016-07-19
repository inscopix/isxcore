#include "isxDispatchQueue.h"
#include "isxDispatchQueuePool.h"

#include <QThreadPool>

namespace isx
{
namespace 
{
class TaskWrapper : public QRunnable
{
public:
    explicit TaskWrapper(DispatchQueueTask_t && inTask)
    : m_Task(std::move(inTask))
    {}

    void run()
    {
        m_Task();
    }

private:
    DispatchQueueTask_t m_Task;
};

} // namespace

DispatchQueuePool::DispatchQueuePool()
{}
    
void
DispatchQueuePool::dispatch(DispatchQueueTask_t inTask)
{
    // note: QRunnable::autoDelete is true by default,
    //       this means QThreadPool will delete the QRunnable
    //       object automatically
    TaskWrapper * tw = new TaskWrapper(std::move(inTask));
    QThreadPool::globalInstance()->start(tw);
}
    
void
DispatchQueuePool::dispatch(void * inContext, DispatchQueueContextTask_t inContextTask)
{
    // note: QRunnable::autoDelete is true by default,
    //       this means QThreadPool will delete the QRunnable
    //       object automatically
    TaskWrapper * tw = new TaskWrapper([=]()
    {
        inContextTask(inContext);
    });
    QThreadPool::globalInstance()->start(tw);
}

} // namespace isx
