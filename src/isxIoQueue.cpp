#include "isxIoThread.h"




IoQueue::IoQueue()
{

}

void
IoQueue::Initialize()
{

}

void
IoQueue::Shutdown()
{

}

bool 
IoQueue::isInitialized()
{

}

IoQueue *
IoQueue::Instance()
{
    return 
}

private:
    IoQueue();
    IoQueue(const IoQueue & other);
    const IoQueue & operator=(const IoQueue & other);
    ~IoQueue();

    static isx::SpDispatchQueueWorker_t m_worker;
