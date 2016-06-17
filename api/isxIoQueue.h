#ifndef ISX_IO_QUEUE_H
#define ISX_IO_QUEUE_H

#include "isxCoreFwd.h"
#include "isxObject.h"
#include "isxDispatchQueueWorker.h"

#include <string>
#include <vector>
#include <memory>

namespace isx {

class IoQueue
{
public:
    void
    Initialize();

    void
    Shutdown();

    bool 
    isInitialized();

    IoQueue *
    Instance();
private:
    IoQueue();
    IoQueue(const IoQueue & other);
    const IoQueue & operator=(const IoQueue & other);
    ~IoQueue();

    isx::SpDispatchQueueWorker_t m_worker;
};

} // namespace isx

#endif // def ISX_IO_QUEUE_H