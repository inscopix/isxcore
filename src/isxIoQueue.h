#ifndef ISX_IO_QUEUE_H
#define ISX_IO_QUEUE_H

#include "isxCoreFwd.h"
#include "isxCore.h"
#include "isxDispatchQueue.h"

#include <memory>

namespace isx {

/// A class implementing a singleton IoQueue to be used
/// application-wide for file I/O.
///
class IoQueue
{
public:
    /// A class for a task to be processed by the IoQueue
    ///
    class Task{
    public:
        /// type of function that implements the asynchronous task
        typedef std::function<void()> Task_t;
        /// type of finished callback function
        typedef std::function<void(AsyncTaskStatus inStatus)> FinishedCB_t;
        /// constructor
        Task(Task_t inTask, FinishedCB_t inFinishedCB) : m_task(inTask), m_finishedCB(inFinishedCB){}

        Task_t       m_task;        ///< this instance's task to process on IoQueue
        FinishedCB_t m_finishedCB;  ///< this instance's finished callback to call when task is done processing
    };

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
    enqueue(Task inTask);

private:
    IoQueue();
    IoQueue(const IoQueue & other) = delete;
    const IoQueue & operator=(const IoQueue & other) = delete;

    class Impl;
    std::shared_ptr<Impl> m_pImpl;
    
    static std::unique_ptr<IoQueue> s_instance;
};

} // namespace isx

#endif // def ISX_IO_QUEUE_H