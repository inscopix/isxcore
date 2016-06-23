#include "isxCore.h"
#include "isxDispatchQueue.h"
#include "isxIoQueue.h"

namespace isx
{
    void CoreInitialize()
    {
        DispatchQueue::initializeDefaultQueues();
        IoQueue::initialize();
    }
    bool CoreIsInitialized()
    {
        return DispatchQueue::isInitialized()
            && IoQueue::isInitialized();
    }
    void CoreShutdown()
    {
        IoQueue::destroy();
        DispatchQueue::destroyDefaultQueues();
    }
}
