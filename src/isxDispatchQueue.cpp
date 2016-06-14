#include "isxDispatchQueue.h"
#include "isxDispatchQueuePool.h"
#include "isxDispatchQueueMain.h"
#include "isxLog.h"

#include <QObject>
#include <QThreadPool>
#include <QApplication>

#include <chrono>
#include <thread>

// Note aschildan 5/31/2016:
// Dispatch Queues are implemented in three subclasses: 
// DispatchQueuePool, DispatchQueueWorker and DispatchQueueMain.
// There is also a DispatchQueue class that contains only static members that are initialized as part
// of CoreInitialize() and that manages the DispatchQueuePool and DispatchQueueMain singleton instances.
// The DispatchQueueDispatcher class enables dispatching to a Qt event loop via signals / slots.  
// The dispatcher's slots are exeuted on the thread that owns the dispatcher (which is the thread that
// created it), while the signals can be called from any thread.  We are using Qt's Qt::AutoConnection
// connection type which queues signals when called from a different thread and invokes the slot
// function immediately when called on the same thread.

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


