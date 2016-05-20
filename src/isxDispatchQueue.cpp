#include "isxDispatchQueue.h"
#include "QThreadPool"

namespace isx
{

DispatchQueue DispatchQueue::m_DefaultQueue;

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

DispatchQueue::DispatchQueue()
{

}

DispatchQueue &
DispatchQueue::defaultQueue()
{
    return m_DefaultQueue;
}

void DispatchQueue::dispatch(tTask inWork)
{
    // since we only support the default queue we can
    // hard-code to use global QThreadPool for now
    TaskWrapper * tw = new TaskWrapper(std::move(inWork));
    QThreadPool::globalInstance()->start(tw);
}

void DispatchQueue::dispatch(void * inContext, tContextTask inWork)
{
    // since we only support the default queue we can
    // hard-code to use global QThreadPool for now
    TaskWrapper * tw = new TaskWrapper([=]()
    {
        inWork(inContext);
    });
    QThreadPool::globalInstance()->start(tw);
}

} // namespace isx

