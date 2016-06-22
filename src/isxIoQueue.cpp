#include "isxIoQueue.h"
#include "isxAssert.h"

namespace isx
{
std::unique_ptr<IoQueue> IoQueue::s_instance;


IoQueue::IoQueue()
{
    m_worker = std::make_shared<DispatchQueueWorker>();
}

IoQueue::~IoQueue()
{
    m_worker.reset();
}

void
IoQueue::initialize()
{
    if (!isInitialized())
    {
        s_instance.reset(new IoQueue());
    }
}

void
IoQueue::destroy()
{
    if (isInitialized())
    {
        s_instance->m_worker->destroy();
        s_instance.reset();
    }
}

bool 
IoQueue::isInitialized()
{
    return !!s_instance;
}

SpDispatchQueueInterface_t
IoQueue::instance()
{
    ISX_ASSERT(isInitialized());
    if (isInitialized())
    {
        return std::dynamic_pointer_cast<DispatchQueueInterface>(s_instance->m_worker);
    }
    return 0;
}
} // namespace isx
