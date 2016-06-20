#ifndef ISX_IO_QUEUE_H
#define ISX_IO_QUEUE_H

#include "isxCoreFwd.h"
#include "isxObject.h"
#include "isxDispatchQueueWorker.h"

#include <string>
#include <vector>
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

    /// \return pointer to the contained worker's dispatch queue
    ///
    static
    DispatchQueueInterface *
    instance();

private:
    IoQueue();
    IoQueue(const IoQueue & other) = delete;
    const IoQueue & operator=(const IoQueue & other) = delete;

    SpDispatchQueueWorker_t m_worker;
    static std::unique_ptr<IoQueue> s_instance;
};

} // namespace isx

#endif // def ISX_IO_QUEUE_H