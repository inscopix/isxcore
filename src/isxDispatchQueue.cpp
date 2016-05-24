#include "isxDispatchQueue.h"
#include "isxDispatchQueue_internal.h"
#include <QObject>
#include <QThreadPool>
#include <QApplication>

#include <assert.h>

Q_DECLARE_METATYPE(isx::DispatchQueue::tTask);
Q_DECLARE_METATYPE(isx::DispatchQueue::tContextTask);

namespace isx
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
    
DispatchQueue::MainThreadObject::MainThreadObject()
{
    qRegisterMetaType<tTask>("tTask");
    qRegisterMetaType<tContextTask>("tContextTask");
    QObject::connect(this, &MainThreadObject::dispatchToMain,
                     this, &MainThreadObject::processOnMain);
    QObject::connect(this, &MainThreadObject::dispatchToMainWithContext,
                     this, &MainThreadObject::processOnMainWithContext);
}

void 
DispatchQueue::MainThreadObject::processOnMain(tTask inTask)
{
    if (QApplication::instance())
    {
        assert(QApplication::instance()->thread() == QThread::currentThread());
    }
    inTask();
}

void 
DispatchQueue::MainThreadObject::processOnMainWithContext(void * inContext, tContextTask inContextTask)
{
    if (QApplication::instance())
    {
        assert(QApplication::instance()->thread() == QThread::currentThread());
    }
    inContextTask(inContext);
}
    
DispatchQueue DispatchQueue::m_DefaultQueue;
DispatchQueue DispatchQueue::m_MainQueue(true);
    
DispatchQueue::DispatchQueue(bool inIsMain)
{
    if (inIsMain)
    {
//        if (QApplication::instance())
        {
//            if (QApplication::instance()->thread() == QThread::currentThread())
            {
                m_pMainThreadObject.reset(new MainThreadObject());
            }
        }
        assert(m_pMainThreadObject);
    }
}

DispatchQueue &
DispatchQueue::defaultQueue()
{
    return m_DefaultQueue;
}

DispatchQueue &
DispatchQueue::mainQueue()
{
    return m_MainQueue;
}

void
DispatchQueue::dispatch(tTask inTask)
{
    if (m_pMainThreadObject)
    {
        emit m_pMainThreadObject->dispatchToMain(inTask);
    }
    else
    {
        // since we only support the default queue we can
        // hard-code to use global QThreadPool for now
        TaskWrapper * tw = new TaskWrapper(std::move(inTask));
        QThreadPool::globalInstance()->start(tw);
    }
}

void
DispatchQueue::dispatch(void * inContext, tContextTask inTask)
{
    if (m_pMainThreadObject)
    {
        emit m_pMainThreadObject->dispatchToMainWithContext(inContext, inTask);
    }
    else
    {
        // since we only support the default queue we can
        // hard-code to use global QThreadPool for now
        TaskWrapper * tw = new TaskWrapper([=]()
        {
            inTask(inContext);
        });
        QThreadPool::globalInstance()->start(tw);
    }
}

} // namespace isx


