#include "isxConditionVariable.h"

#include <QMutex>
#include <QWaitCondition>

namespace isx
{

class ConditionVariable::Impl
{
public:
    Impl(){}
    ~Impl(){}
    
    void
    wait(NativeMutex_t inLock)
    {
        QMutex * m = static_cast<QMutex *>(inLock);
        m_waitCondition.wait(m, ULONG_MAX);
    }

    bool
    waitForMs(NativeMutex_t inLock, uint32_t inWaitForMs)
    {
        QMutex * m = static_cast<QMutex *>(inLock);
        return m_waitCondition.wait(m, inWaitForMs);
    }

    void
    notifyOne()
    {
        m_waitCondition.wakeOne();
    }

    void
    notifyAll()
    {
        m_waitCondition.wakeAll();
    }

private:
    QWaitCondition m_waitCondition;
};

ConditionVariable::ConditionVariable()
: m_pImpl(new Impl())
{}

ConditionVariable::~ConditionVariable() {}

void
ConditionVariable::wait(NativeMutex_t inLock)
{
    m_pImpl->wait(inLock);
}

bool
ConditionVariable::waitForMs(NativeMutex_t inLock, uint32_t inWaitForMs)
{
    return m_pImpl->waitForMs(inLock, inWaitForMs);
}

void
ConditionVariable::notifyOne()
{
    m_pImpl->notifyOne();
}

void
ConditionVariable::notifyAll()
{
    m_pImpl->notifyAll();
}

} // namespace isx
