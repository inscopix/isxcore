#include "isxCore.h"
#include "isxDispatchQueue.h"

namespace isx
{
    void CoreInitialize()
    {
        DispatchQueue::initializeDefaultQueues();
    }
    bool CoreIsInitialized()
    {
        return DispatchQueue::isInitialized();
    }
    void CoreShutdown()
    {
        DispatchQueue::destroyDefaultQueues();
    }
}
