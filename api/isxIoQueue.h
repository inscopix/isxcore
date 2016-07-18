#ifndef ISX_IO_QUEUE_H
#define ISX_IO_QUEUE_H

#include "isxCoreFwd.h"
#include "isxObject.h"
#include "isxDispatchQueueWorker.h"
#include "isxMutex.h"

#include <memory>

namespace isx {

/// A class implementing a singleton IoQueue to be used
/// application-wide for file I/O.
///
class IoQueue
{
public:
    /// destructor
    ///
    ~IoQueue();

    /// Singleton initializer
    ///
    static
    void
    initialize();

    /// Singleton destroy
    ///
    static
    void
    destroy();

    /// Check if singleton has been initialized
    /// \return bool indicating the above
    ///
    static
    bool 
    isInitialized();

    /// \return pointer to the IoQueue singleton instance
    ///
    static
    IoQueue *
    instance();

    /// enqueue I/O task to be processed on IoQueue's thread
    /// \param inTask task to be processed
    void
    enqueue(Task_t inTask);

    /// Accessor for single global I/O mutex.
    /// Use this for any I/O through HDF5.
    /// \return Mutex for HDF5 I/O
    static
    Mutex &
    getMutex();
    
private:
    IoQueue();
    IoQueue(const IoQueue & other) = delete;
    const IoQueue & operator=(const IoQueue & other) = delete;

    class Impl;
    std::shared_ptr<Impl> m_pImpl;
    
    static std::unique_ptr<IoQueue> s_instance;
    Mutex m_mutex;
};

} // namespace isx

#endif // def ISX_IO_QUEUE_H