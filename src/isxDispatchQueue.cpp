#include "isxDispatchQueue.h"
#include "isxDispatchQueuePool.h"
#include "isxDispatchQueueMain.h"
#include "isxLog.h"

#include <QObject>
#include <QThreadPool>
#include <QApplication>

#include <chrono>
#include <thread>

#include <assert.h>

namespace isx
{
    
SpDispatchQueueInterface_t DispatchQueue::s_pool;
SpDispatchQueueInterface_t DispatchQueue::s_main;
bool DispatchQueue::s_isInitialized;

void
DispatchQueue::initializeDefaultQueues()
{
    s_pool.reset(new DispatchQueuePool());
    s_main.reset(new DispatchQueueMain());
    s_isInitialized = true;
}

bool 
DispatchQueue::isInitialized()
{
    return s_isInitialized;
}

void 
DispatchQueue::destroyDefaultQueues()
{
    s_pool.reset();
    s_main.reset();
    s_isInitialized = false;
}

SpDispatchQueueInterface_t
DispatchQueue::poolQueue()
{
    return s_pool;
}

SpDispatchQueueInterface_t
DispatchQueue::mainQueue()
{
    return s_main;
}

} // namespace isx


