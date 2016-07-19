#include "isxCore.h"
#include "isxDispatchQueue.h"
#include "isxIoQueue.h"
#include "isxVersion.h"

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

    int CoreVersionMajor()
    {
        return APP_VERSION_MAJOR;
    }
    int CoreVersionMinor()
    {
        return APP_VERSION_MINOR;
    }

    int CoreVersionPatch()
    {
        return APP_VERSION_PATCH;
    }
}
